/* Zgv v3.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * readxpm.c - XPM loader.
 *
 * Since (as far as I can tell) the XPM library seems to be
 * difficult or impossible to use on a machine without X installed
 * (which is a possibility with zgv), I reluctantly decided not to
 * use it. So this is a fairly noddy from-scratch XPM reader.
 *
 * XXX one problem with this - >256-col reading is suboptimal for cpp==2.
 * Could be made quicker by using same sort of speedup used for <=256-col
 * files when cpp==2, i.e. by using a 65536-entry direct-lookup array
 * containing ints (well, longs) with rgb values. That would be 256k,
 * but that wouldn't be too bad to malloc.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "zgv.h"
#include "readpnm.h"	/* for dithering routines */
#include "rcfile.h"
#include "rc_config.h"
#include "readxpm.h"
#include <unistd.h>

/* max chars/pixel we allow;
 * most files use one, some use two, I've seen a couple which used
 * three (though not for any good reason!), so this sounds ok I think.
 */
#define MAX_CPP		16


/* for aborted_file_xpm_cleanup() */
static unsigned char *work_bmap,*work_pal;
static FILE *work_in;

/* for read_next_string, but outside it in case e.g. a read error
 * happens halfway through, which would (in theory, if not practice)
 * screw up further XPM reads.
 */
static int in_comment=0;

/* for colchars_bsearch_cmp, but outside it as it's called from
 * bsearch().
 */
static int bsearch_cmp_cpp;


/* RGB DB array (dynamically allocated) */

#define MAX_COLNAME_LEN	64	/* not ideal but simplifies matters */

struct rgb_db_tag
  {
  int rgb;
  unsigned char name[MAX_COLNAME_LEN+1];
  };

static struct rgb_db_tag *rgb_db_array=NULL;

/* rgb_db_size must be at least 2*sizeof... (see init_rgb_db() for why) */
static int rgb_db_size=64*sizeof(struct rgb_db_tag);
static int rgb_db_incr=32*sizeof(struct rgb_db_tag);
static int rgb_db_num_entries=0;

static struct colchars_tag	/* this is used to store colour char codes */
  {
  int idx;
  unsigned char r,g,b;
  unsigned char name[MAX_CPP+1];
  } *colchars=NULL;


/* prototypes */
int colchars_cmp(const void *p1,const void *p2);
int colchars_bsearch_cmp(const void *p1,const void *p2);
unsigned char *read_next_string(FILE *in);
unsigned char *next_token(unsigned char *ptr);
void get_token(unsigned char *token,int tokenmax,unsigned char *buf);
int lookup_named_colour(unsigned char *colname);
void init_rgb_db(void);




/* output_type returns how many bytes per pixel needed for display */
int read_xpm_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **pal,int *output_type,PICINFO *pp)
{
static unsigned char smallcol2idx[65536];	/* lookup tbl if cpp<3 */
static unsigned char buf[128]; /* only used for magic/tokens so can be short */
static unsigned char colname[256];	/* temp. used to store colour name */
FILE *in;
unsigned char *inpline;
int w,h,f,x,y,bytepp,ncols,cpp;	/* cpp=chars/pixel */
unsigned char *cptr;
unsigned char *ptr,*palptr;
int incr;

*bmap=NULL;
*pal=NULL;
in_comment=0;
bytepp=1;

if((in=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

if(fgets(buf,sizeof(buf),in)==NULL || strncmp(buf,"/* XPM",6)!=0)
  CLOSE_AND_RET(_PICERR_BADMAGIC);


/* get width/height etc. */
if((inpline=read_next_string(in))==NULL)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if(sscanf(inpline,"%d %d %d %d",&w,&h,&ncols,&cpp)!=4)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if(w==0 || h==0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if(cpp>MAX_CPP)
  CLOSE_AND_RET(_PICERR_TOOMANYCOLS);

if((*pal=palptr=calloc(768,1))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

/* we zero out the palette to avoid possible problems with vkludge in
 * vgadisp.c if we don't (i.e. random junk colours being used) as there
 * may well be fewer than 256 colours.
 */
memset(palptr,0,768);

if(ncols>256)
  {
  bytepp=3;
  if(*output_type==1 || cfg.jpeg24bit==0)	/* dither? */
    {
    int r,g,b;
    
    bytepp=1;
    if(ditherinit(w)==0)
      CLOSE_AND_RET(_PICERR_NOMEM);
    
    ptr=palptr;
    for(r=0;r<8;r++)
      for(g=0;g<8;g++)	/* colours are 3:3:2 */
        for(b=0;b<4;b++)
          {
          *ptr=r*255/7; ptr[256]=g*255/7; ptr[512]=b*255/3;
          ptr++;
          }
    }
  }


/* make an effort to always show them on a grey background
 * by making colour zero grey if possible.
 * (XXX will need to be changed if the other one is)
 */
incr=0;
if(ncols<256)
  {
  incr=1;
  palptr[0]=palptr[256]=palptr[512]=128;
  }


/* read rgb.txt if we haven't already */
if(rgb_db_array==NULL)
  init_rgb_db();


/* colchars will usually be NULL at this point, but an error could
 * well have left it hanging around - nuke it if so.
 */
if(colchars!=NULL) free(colchars);

/* alloc colchars array */
if(ncols>(1<<24) || (colchars=malloc(ncols*sizeof(struct colchars_tag)))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);


/* read in colours. This is REALLY FUN. :-(
 */

for(f=0;f<ncols;f++)
  {
  int r,g,b;
  
  if((inpline=read_next_string(in))==NULL)
    CLOSE_AND_RET(_PICERR_CORRUPT);
  
  memset(colchars[f].name,0,MAX_CPP+1);
  memcpy(colchars[f].name,inpline,cpp);
  colchars[f].idx=f+incr;
  
  /* look for "c" token */
  cptr=inpline+cpp; *buf=0;
  while(*cptr && strcmp(buf,"c")!=0)
    {
    cptr=next_token(cptr);
    get_token(buf,sizeof(buf),cptr);
    }

  if(strcmp(buf,"c")!=0)
    {
    /* no colour visual definition found. 
     * this is pretty rare - in fact, I've not seen *any* cases of it.
     * for now (XXX), gives a fatal error.
     */
    CLOSE_AND_RET(_PICERR_UNSUPPORTED);
    }
  
  cptr=next_token(cptr);
  
  /* so now cptr points to a colour name/number. Unfortunately,
   * people seem to take amazing liberties with colour names.
   * They lowercase them (not allowed by the format spec), they
   * use separate words for colours like "light sky blue" (not
   * allowed by the format spec)... happy happy joy joy, not.
   *
   * The least awful solution, I think, is to add to the next token
   * (i.e. the one that *should* be the colour) any consecutively
   * following ones which are more than 2 chars long. Then in any
   * string comparisons, we use strcasecmp rather than strcmp.
   * As far as I can see, this scheme should work in all cases.
   */
  get_token(colname,sizeof(colname),cptr);
  cptr=next_token(cptr);
  get_token(buf,sizeof(buf),cptr);
  while(strlen(buf)>2)
    {
    /* avoid overruns */
    if(strlen(colname)+strlen(buf)+2>sizeof(colname))
      break;
    
    strcat(colname," ");
    strcat(colname,buf);
    cptr=next_token(cptr);
    get_token(buf,sizeof(buf),cptr);
    }
  
  /* now work out what we've got. if it starts with a `#' it's RGB,
   * and if it's "None" it's the transparent `colour', otherwise it's a
   * colour name from rgb.txt.
   *
   * However, we also tolerate the old-style "#Transparent" for "None".
   */
  if(*colname=='#' && isxdigit(colname[1]))
    {
    /* RGB - length of rest must be 3, 6, 9, or 12. */
    int rgb;
    
    switch(strlen(colname+1))
      {
      case 3:
        rgb=strtol(colname+1,NULL,16);
        r=(rgb>>8)*16+(rgb>>8);
        g=((rgb>>4)&15)*16+((rgb>>4)&15);
        b=(rgb&15)*16+(rgb&15);
        break;
      
      case 6:
        rgb=strtol(colname+1,NULL,16);
        r=(rgb>>16);
        g=((rgb>>8)&255);
        b=(rgb&255);
        break;
      
      case 9:
        /* 0123456789
         * #rr.gg.bb.
         */
        colname[3]=colname[6]=colname[9]=0;
        r=strtol(colname+1,NULL,16);
        g=strtol(colname+4,NULL,16);
        b=strtol(colname+7,NULL,16);
        break;
      
      case 12:
        /* 0123456789012
         * #rr..gg..bb..
         */
        colname[3]=colname[7]=colname[11]=0;
        r=strtol(colname+1,NULL,16);
        g=strtol(colname+5,NULL,16);
        b=strtol(colname+9,NULL,16);
        break;
      
      default:
        CLOSE_AND_RET(_PICERR_BADXCOL);
      }
    }
  else		/* not RGB number... */
    if(strcasecmp(colname,"None")==0 || strcasecmp(colname,"#Transparent")==0)
      {
      /* transparent `colour' - we use grey.
       * This is inconsistent with PNG's current background colour,
       * true, but it's likely to be what you'd want.
       * (XXX may want to make configurable though)
       */
      r=g=b=128;
      }
    else
      {
      /* lookup the colour name. */
      int rgb=lookup_named_colour(colname);
      
      if(rgb==-1)	/* colour not found */
        CLOSE_AND_RET(_PICERR_BADXCOL);
      
      r=(rgb>>16);
      g=((rgb>>8)&255);
      b=(rgb&255);
      }
  
  /* write colour to colchars[] in case we're producing 24-bit output */
  colchars[f].r=r;
  colchars[f].g=g;
  colchars[f].b=b;
  
  if(ncols<=256)
    {
    /* also write palette entries */
    palptr[f+incr    ]=r;
    palptr[f+incr+256]=g;
    palptr[f+incr+512]=b;
    }
  }


switch(cpp)
  {
  /* for cpp<=2, make code -> index lookup table. the disadvantage of
   * this is that we don't spot invalid colour codes, but it's so
   * much quicker this is probably worth it. The memsets do at least
   * ensure we give a consistent result (invalid codes give the
   * background colour (i.e. colour index 0)).
   */
  case 1:
    memset(smallcol2idx,0,256);
    for(f=0;f<ncols;f++)
      smallcol2idx[colchars[f].name[0]]=f+incr;
    break;
    
  case 2:
    /* even for 2 cpp, use faster method if <=256 colours... */
    if(ncols<=256)
      {
      memset(smallcol2idx,0,65536);
      for(f=0;f<ncols;f++)
        smallcol2idx[(colchars[f].name[0])|(colchars[f].name[1]<<8)]=f+incr;
      break;
      }
    /* else FALLS THROUGH */
  
  default:	/* i.e. if cpp>=3 (or cpp==2 and ncols>256) */
    /* sort colchars[] so binary search will work. */
    qsort(colchars,ncols,sizeof(struct colchars_tag),colchars_cmp);
  }


/* phew. finally dealt with the colours, let's try (gasp) reading the
 * bloody *picture*! :-)
 */

/* extra lines are in case we're dithering. */
if(WH_BAD(w,h) || (*bmap=malloc(w*(h+2)*bytepp))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

ptr=*bmap;

/* save stuff in case of abort */
work_in=in; work_bmap=ptr; work_pal=*pal;

bsearch_cmp_cpp=cpp;	/* needed if we get cols with bsearch() */

for(y=0;y<h;y++)
  {
  cptr=read_next_string(in);
  
  switch(cpp)
    {
    case 1:
      /* loop over line directly indexing in smallcol2idx */
      for(x=0;x<w;x++)
        *ptr++=smallcol2idx[*cptr++];
      break;
    
    case 2:
      if(ncols<=256)
        {
        /* loop over line directly indexing in smallcol2idx again, but a
         * little more work needed to convert code to an int...
         */
        for(x=0;x<w;x++,cptr+=2)
          *ptr++=smallcol2idx[(*cptr)|(cptr[1]<<8)];
        break;
        }
      /* else FALLS THROUGH */
    
    default:	/* cpp>=3 */
      /* loop over line doing binary search to get colour index. */
      for(x=0;x<w;x++,cptr+=cpp)
        {
        static struct colchars_tag *match;
        
        match=bsearch(cptr,colchars,ncols,
        		sizeof(struct colchars_tag),colchars_bsearch_cmp);
        
        if(match==NULL)
          CLOSE_AND_RET(_PICERR_CORRUPT);	/* colour code not found */
        
        if(ncols<=256)
          *ptr++=match->idx;
        else
          {
          *ptr++=match->b;
          *ptr++=match->g;
          *ptr++=match->r;
          }
        }
      break;
    }
  
  /* do dithering if needed */
  if(ncols>256 && bytepp==1)
    {
    ptr-=w*3;
    ditherline(ptr,y,w);
    ptr+=w;
    }
  
  if(howfarfunc!=NULL) howfarfunc(y,h);
  }


pp->width=w;
pp->height=h;
pp->numcols=256;

*output_type=bytepp;

if(ncols>256 && bytepp==1)
  ditherfinish();

free(colchars);
colchars=NULL;
  
fclose(in);
return(_PIC_OK);  
}


/* I don't normally bother with const, but using it here saves
 * a complaint from gcc -Wall.
 */
int colchars_cmp(const void *p1,const void *p2)
{
struct colchars_tag *cc1=(struct colchars_tag *)p1;
struct colchars_tag *cc2=(struct colchars_tag *)p2;

return(strcmp(cc1->name,cc2->name));
}


/* need a different cmp for bsearch(). */
int colchars_bsearch_cmp(const void *p1,const void *p2)
{
unsigned char *cc1=(unsigned char *)p1;
struct colchars_tag *cc2=(struct colchars_tag *)p2;

return(memcmp(cc1,cc2->name,bsearch_cmp_cpp));
}


/* reads a string from the XPM.
 * returns string, or NULL if EOF found first.
 */
unsigned char *read_next_string(FILE *in)
{
static unsigned char buf[16384];
static int in_string=0;
int c,c2,buflen,gotstring;

in_string=gotstring=0;
buflen=0;

do
  {
  if((c=fgetc(in))==EOF)
    return(NULL);
  
  if(in_comment)
    {
    if(c=='*')		/* check for end of comment */
      {
      if((c2=fgetc(in))=='/')
        in_comment=0;
      else
        ungetc(c2,in);	/* since this could (in theory) be a `*' */
      }
    }
  else	/* not in comment */
    {
    if(!in_string)
      {
      if(c=='/')	/* check for start of comment */
        {
        if((c2=fgetc(in))=='*')
          in_comment=1;
        else
          ungetc(c2,in);  /* equally, this could be `"' to start string */
        }
      else
        if(c=='"')	/* check for start of string */
          in_string=1;
      }
    else	/* already in string */
      {
      if(c=='"')	/* check for end of string */
        {
        in_string=0;
        gotstring=1;
        }
      else
        {
        buf[buflen++]=c;
        /* this is probably unlikely, but just in case... */
        if(buflen>=sizeof(buf)-1) gotstring=1;
        }
      }
    }
  }
while(!gotstring);  

buf[buflen]=0;
return(buf);
}


void aborted_file_xpm_cleanup()
{
free(work_bmap);
free(work_pal);
fclose(work_in);
}


unsigned char *next_token(unsigned char *ptr)
{
while(*ptr!=0 && *ptr!=' ' && *ptr!='\t')
  ptr++;
while(*ptr==' ' || *ptr=='\t')
  ptr++;
return(ptr);
}


void get_token(unsigned char *token,int tokenmax,unsigned char *buf)
{
int n=0;

while(*buf!=0 && *buf!=' ' && *buf!='\t' && n<tokenmax-1)
  *token++=*buf++,n++;
*token=0;
}


/* return 24-bit colour value if found, else -1. */
int lookup_named_colour(unsigned char *colname)
{
int f;

for(f=0;f<rgb_db_num_entries;f++)
  if(strcasecmp(colname,rgb_db_array[f].name)==0)
    return(rgb_db_array[f].rgb);

return(-1);
}


/* parse X rgb db (rgb.txt) into rgb_db_array. */
void init_rgb_db()
{
static char buf[256];
FILE *db;
char *ptr;
int r,g,b;

if((rgb_db_array=malloc(rgb_db_size))==NULL)
  return;	/* no big deal, it was NULL before anyway :-) */

if((db=fopen("rgb.txt","r"))==NULL)
  {
  /* knock up entries for black and white at least!
   * we know rgb_db_array is at least two entries big already.
   */
  
  rgb_db_array[rgb_db_num_entries].rgb=0;
  strcpy(rgb_db_array[rgb_db_num_entries].name,"black");
  rgb_db_num_entries++;
  rgb_db_array[rgb_db_num_entries].rgb=0xffffff;
  strcpy(rgb_db_array[rgb_db_num_entries].name,"white");
  rgb_db_num_entries++;
  
  return;
  }


/* ok, we've opened the file, read it in. */

while(fgets(buf,sizeof(buf),db)!=NULL)
  {
  if(*buf=='#' || *buf=='!') continue;
  
  /* chop LF */
  if(*buf && buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;
  
  if(*buf==0) continue;
  
  /* make array bigger if needed */
  if(rgb_db_num_entries*sizeof(struct rgb_db_tag)>=rgb_db_size)
    {
    rgb_db_size+=rgb_db_incr;
    if((rgb_db_array=realloc(rgb_db_array,rgb_db_size))==NULL)
      {
      /* give up if out of memory */
      fclose(db);
      return;
      }
    }
  
  ptr=buf;
  r=strtol(ptr,&ptr,10); if(r<0) r=0; if(r>255) r=255;
  g=strtol(ptr,&ptr,10); if(g<0) g=0; if(g>255) g=255;
  b=strtol(ptr,&ptr,10); if(b<0) b=0; if(b>255) b=255;
  rgb_db_array[rgb_db_num_entries].rgb=((r<<16)|(g<<8)|b);
  ptr=next_token(ptr);
  if(strlen(ptr)>MAX_COLNAME_LEN)
    ptr[MAX_COLNAME_LEN]=0;
  strcpy(rgb_db_array[rgb_db_num_entries].name,ptr);
  rgb_db_num_entries++;
  }

fclose(db);
}
