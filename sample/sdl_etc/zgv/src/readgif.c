/* zgv 5.4 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * readgif.c - GIF reader.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "zgv.h"
#include "readgif.h"


/* XXX should make this allocate on the fly rather than inflicting
 * arbitrary values on people... :-)
 */
#define MAX_GIF_IMAGES		256

/* this only enforced for multi-image GIFs. Leaves room for GIFs to
 * be loaded as 24-bit at some point in the future, though they're
 * always 8-bit as things are.
 */
#define MAX_GIFMULTI_BYTES_PER_IMAGE	((1<<22)-1)


static byte *image;
static hffunc howfar;
static FILE *global_gif_infile;   /* only used for error cleanup */

typedef struct
  {
  int left,top;
  int width,height;
  int numcols;
  int misc,delay,transparent_col,gcb_control;
  unsigned char *cmap,*image;
  } gif_image;

/* the GIF is read into these */
static unsigned char *global_cmap;
static int swidth,sheight;	  /* screen width/height (for multi-image) */
static int imagecount;
static gif_image *images[MAX_GIF_IMAGES];

/* these are global to save going through multiple hoops :-) */
int gif_delaycount;
int *gif_delay=NULL;		/* array of delays after each image */

static int imagex,imagey,stopoutput;
static int dc_cc,dc_eoi;                  /* the CC and EOI codes */
static int passnum,passyloc,passstep;     /* for interlaced GIFs */
static int interlaced,width,height,bpp,numcols,gnumcols,lnumcols;
static int global_colour_map,local_colour_map;
static int transparent_col,delay,gcb_control;


/* now this is for the string table.
 * the st_ptr array stores which pos to back reference to,
 *  each string is [...]+ end char, [...] is traced back through
 *  the 'pointer' (index really), then back through the next, etc.
 *  a 'null pointer' is = to UNUSED.
 * the st_chr array gives the end char for each.
 *  an unoccupied slot is = to UNUSED.
 */
#define UNUSED 32767
#define MAXSTR 4096
static int st_ptr[MAXSTR],st_chr[MAXSTR],st_last;
static int st_ptr1st[MAXSTR];

/* this is for the byte -> bits mangler:
 *  dc_bitbox holds the bits, dc_bitsleft is number of bits left in dc_bitbox,
 *  blocksize is how many bytes of an image sub-block we have left.
 */
static int dc_bitbox,dc_bitsleft,blocksize;

struct
  {
  char sig[6];           /* should be GIF87a or GIF89a */
  byte wide_lo,wide_hi;  /* NUXI Problem Avoidance System (tm) */
  byte high_lo,high_hi;  /* these are 'screen size', BTW */
  byte misc;             /* misc, and bpp */
  byte back;             /* background index */
  byte zero;             /* if this ain't zero, problem */
  } gifhed;
  
/* BTW, the NUXI thing above is because most of this code is reused from
 * a GIF viewer I did for Tektronix 4200 series terminals. If you want a copy,
 * mail me, but I figure not many people use them.
 * (at least, not from Linux PCs :-))
 */
  
struct
  {
  byte left_lo,left_hi;  /* usually zero - ignore */
  byte top_lo,top_hi;
  byte wide_lo,wide_hi;  /* this is 'image size', often the same as screen */
  byte high_lo,high_hi;
  byte misc;
  } imagehed;



void outputchr(int code)
{
if(!stopoutput)
  {
  *(image+(interlaced?passyloc:imagey)*width+imagex)=code;
  imagex++;
  if(imagex>=width)
    {
    imagex=0;
    imagey++;
    if(interlaced)
      {
      passyloc+=passstep;
      while(passyloc>=height && passnum<4)
        {
        passnum++;
        passyloc=(1<<(4-passnum));
        passstep=(1<<(5-passnum));
        }
      }
    if(howfar)
      howfar(imagey,height);
    if(imagey==height) stopoutput=1;
    }
  }
}


/* This used to be recursive, but a broken GIF could cause problems
 * that way. This version should be unhangable. :-)
 */
void outputstring(int code)
{
static int buf[MAXSTR];
int *ptr=buf;

while(st_ptr[code]!=UNUSED && ptr<buf+MAXSTR)
  {
  *ptr++=st_chr[code];
  code=st_ptr[code];
  }

outputchr(st_chr[code]);
while(ptr>buf)
  outputchr(*--ptr);
}


int findfirstchr(int code)
{
if(st_ptr[code]!=UNUSED)       /* not first? then use brand new st_ptr1st! */
  code=st_ptr1st[code];                /* now with no artificial colouring */
return(st_chr[code]);
}


int readgifhed(FILE *in)
{
fread(&gifhed,sizeof(gifhed),1,in);
if(strncmp(gifhed.sig,"GIF",3))
  return(_PICERR_BADMAGIC);
global_colour_map=(gifhed.misc&128)?1:0;
bpp=(gifhed.misc&7)+1;
gnumcols=numcols=(1<<bpp);
return(_PIC_OK);
}


void readcolmap(FILE *in,byte *palette,int numcols)
{
int f;

for(f=0;f<numcols;f++)
  {
  palette[    f]=(byte)fgetc(in);
  palette[256+f]=(byte)fgetc(in);
  palette[512+f]=(byte)fgetc(in);  
  }
}


int readimagehed(FILE *in)
{
int c,f;

gcb_control=0;
delay=0;
transparent_col=0;

c=fgetc(in);

while(c=='!')      /* oh damn it, they've put an ext. block in, ditch it */
  {
  c=fgetc(in);       /* function code */
  
  switch(c)
    {
    case 0xf9:	/* graphics control block, specifies transparency etc. */
      if((c=fgetc(in))==4)
        {
        gcb_control=fgetc(in);
        delay=fgetc(in);
        delay+=fgetc(in)*256;
        transparent_col=fgetc(in);
        c=fgetc(in);
        
        break;		/* avoid fall-through required by `else' below */
        }
      else
        {
        /* not 4 bytes, must be corrupt - try ditching the block. */
        ungetc(c,in);
        }
      /* FALLS THROUGH */
    
    default:	/* other block types are ignored */
      c=fgetc(in);
      while(c && c!=EOF)
        {    /* well then, c = number of bytes, so ignore that many */
        for(f=0;f<c;f++) fgetc(in);
        c=fgetc(in);
        }
    }
  
  c=fgetc(in);     /* test for image again */
  }

if(c==';')
  return(_PICERR_NOMORE);	/* no more images */

if(c!=',')
  return(_PICERR_NOIMAGE);

fread(&imagehed,sizeof(imagehed),1,in);

local_colour_map=(imagehed.misc&128)?1:0;

if((imagehed.misc&64)!=0)
  {
  interlaced=1;
  passnum=1;
  passyloc=0;
  passstep=8;
  }
else
  interlaced=0;

width=(imagehed.wide_lo+256*imagehed.wide_hi);
height=(imagehed.high_lo+256*imagehed.high_hi);

lnumcols=0;
if(local_colour_map) lnumcols=(1<<((imagehed.misc&7)+1));

return(_PIC_OK);
}


void inittable(int orgcsize)
{
int f;
int numcols=(1<<(orgcsize-1));

for(f=0;f<MAXSTR;f++)
  {
  st_chr[f]=UNUSED;
  st_ptr[f]=UNUSED;
  }
for(f=0;f<numcols+2;f++)
  {
  st_ptr[f]=UNUSED;     /* these are root values... no back pointer */
  st_chr[f]=f;          /* for numcols and numcols+1, doesn't matter */
  }
st_last=numcols+1;      /* last occupied slot */
dc_cc=numcols;
dc_eoi=numcols+1;
if(numcols==2)
  {
  st_chr[2]=st_chr[3]=UNUSED;
  dc_cc=4;
  dc_eoi=5;
  st_chr[dc_cc]=dc_cc; st_chr[dc_eoi]=dc_eoi;
  st_last=5;
  }
}


/* add a string specified by oldstring + chr to string table */
int addstring(int oldcode,int chr)
{
st_last++;
if((st_last&4096))
  {
  st_last=4095;
  return(1);		/* not too clear it should die or not... */
  }
while(st_chr[st_last]!=UNUSED)
  {
  st_last++;
  if((st_last&4096))
    {
    st_last=4095;
    return(1);
    }
  }
st_chr[st_last]=chr;
if(st_last==oldcode)
  return(0);		/* corrupt GIF - can cause hangs without this */
st_ptr[st_last]=oldcode;
if(st_ptr[oldcode]==UNUSED)          /* if we're pointing to a root... */
  st_ptr1st[st_last]=oldcode;        /* then that holds the first char */
else                                 /* otherwise... */
  st_ptr1st[st_last]=st_ptr1st[oldcode]; /* use their pointer to first */

return(1);
}


/* read a code of bitlength numbits from in file */
int readcode(int *newcode,int numbits,FILE *in)
{
int bitsfilled,got;

bitsfilled=got=0;
(*newcode)=0;

while(bitsfilled<numbits)
  {
  if(dc_bitsleft==0)        /* have we run out of bits? */
    {
    if(blocksize<=0)        /* end of block? */
      blocksize=fgetc(in);  /* start new block, blocksize = num of bytes */
    blocksize--;
    dc_bitbox=fgetc(in);    /* read eight more bits */
    if(feof(in))
      return(0);
    dc_bitsleft=8;
    }
  if(dc_bitsleft<(numbits-bitsfilled))
    got=dc_bitsleft;
  else
    got=numbits-bitsfilled;
  (*newcode)|=((dc_bitbox&((1<<got)-1))<<bitsfilled);
  dc_bitbox>>=got;
  dc_bitsleft-=got;
  bitsfilled+=got;
  }

if((*newcode)<0 || (*newcode)>MAXSTR-1) return(0);
return(1);
}


int decompress(FILE *in)
{
int csize,orgcsize;
int newcode,oldcode,k;
int first=1;

csize=fgetc(in)+1;
orgcsize=csize;
inittable(orgcsize);

oldcode=newcode=0;

while(newcode!=dc_eoi)
  {
  if(!readcode(&newcode,csize,in)) return(_PICERR_CORRUPT);
  if(newcode!=dc_eoi)
    {
    if(newcode==dc_cc)
      {
      /* don't redo it if it's the first code */
      if(!first) inittable(orgcsize);
      csize=orgcsize;
      if(!readcode(&newcode,csize,in)) return(_PICERR_CORRUPT);
      oldcode=newcode;
      outputstring(newcode);
      }
    else
      {
      if(st_chr[newcode]!=UNUSED)
        {
        outputstring(newcode);
        k=findfirstchr(newcode);
        }
      else
        {
        k=findfirstchr(oldcode);
        outputstring(oldcode);
        outputchr(k);
        }
      if(st_last!=4095)
        {
        if(!addstring(oldcode,k))
          return(_PICERR_CORRUPT);
        if(st_last==(1<<csize)-1)
          {
          csize++;
          if(csize==13) csize=12;
          }
        }
      oldcode=newcode;
      }
    }
  
  first=0;
  }

return(_PIC_OK);
}



/* read a GIF file as zero or more separate images;
 * used by read_gif_file for the actual GIF-reading.
 */
int read_gif_multi(char *giffn)
{
FILE *in;
int poserr;
int i,c;
gif_image *im;
int got_valid_image=0;	/* used to test we have at least one image */

for(i=0;i<MAX_GIF_IMAGES;i++)
  images[i]=NULL;

global_cmap=NULL;
imagecount=0;

if((in=global_gif_infile=fopen(giffn,"rb"))==NULL)
  return(_PICERR_NOFILE);

if((poserr=  readgifhed(in)  )!=_PIC_OK)
  {
  fclose(in);
  return(poserr);
  }

/* get screen width/height */
swidth=gifhed.wide_lo+256*gifhed.wide_hi;
sheight=gifhed.high_lo+256*gifhed.high_hi;

if(global_colour_map)
  {
  if((global_cmap=(byte *)malloc(768))==NULL)
    {
    fclose(in);
    return(_PICERR_NOMEM);
    }
  
  readcolmap(in,global_cmap,numcols);
  }

while((poserr=readimagehed(in))==_PIC_OK)
  {
  dc_bitbox=dc_bitsleft=blocksize=0;
  imagex=imagey=stopoutput=0;
  
  if(howfar)
    howfar(-1,0);		/* reset progress indicator */
  
  /* exit successfully if too many images (XXX debatable approach) */
  if(imagecount>=MAX_GIF_IMAGES-1)
    break;
  
  if((im=(gif_image *)malloc(sizeof(gif_image)))==NULL)
    {
    fclose(in);
    return(_PICERR_NOMEM);
    }
  
  images[imagecount++]=im;
  
  im->image=NULL;
  im->cmap=NULL;
  im->width=width;
  im->height=height;
  im->left=imagehed.left_lo+256*imagehed.left_hi;
  im->top=imagehed.top_lo+256*imagehed.top_hi;
  im->misc=imagehed.misc;
  im->numcols=lnumcols;		/* XXX is this always right? */
  im->gcb_control=gcb_control;
  im->delay=delay;
  im->transparent_col=transparent_col;
  
  if(local_colour_map)
    {
    if((im->cmap=malloc(768))==NULL)
      {
      fclose(in);
      return(_PICERR_NOMEM);
      }
    
    readcolmap(in,im->cmap,lnumcols);
    }
  
  if(WH_BAD(width,height) || (im->image=(byte *)malloc(width*height))==NULL)
    {
    fclose(in);
    return(_PICERR_NOMEM);
    }
  
  image=im->image;
  
  memset(image,0,width*height);		/* XXX is this needed? */
  
  if((poserr=decompress(in))!=_PIC_OK)
    {
    fclose(in);
    return(poserr);
    }
  
  /* if we get here, we must have at least one valid image */
  got_valid_image=1;

  /* ditch image block terminator */
  fgetc(in);
  
  /* some broken GIF encoders seem to output extra block terminators,
   * so tolerate that... :-/
   */
  while((c=fgetc(in))==0)
    ;
  
  ungetc(c,in);
  }

fclose(in);

/* if the `error' is no more images, it's ok really */
if(poserr==_PICERR_NOMORE)
  poserr=_PIC_OK;

/* if there was no image but we already have one or more, that's also ok */
if(poserr==_PICERR_NOIMAGE && got_valid_image)
  poserr=_PIC_OK;

/* conversely, if we didn't get that (which can happen for certain odd
 * GIF files), but *don't* have a valid image, that's *not* ok :-)
 */
if(!got_valid_image)
  poserr=_PICERR_NOIMAGE;

return(poserr);
}


void images_cleanup(void)
{
int i;

for(i=0;i<MAX_GIF_IMAGES;i++)
  if(images[i])
    {
    if(images[i]->cmap)
      free(images[i]->cmap);
    if(images[i]->image)
      free(images[i]->image);
    free(images[i]);
    images[i]=NULL;
    }
}


/* howfarfunc, for percent complete displays etc., can be NULL.
 * a return value other than _PIC_OK means something went wrong;
 * see zgv.h for more details and error codes.
 */
int read_gif_file(char *giffn,hffunc howfarfunc,byte **theimageptr, 
			byte **palptr,int *pixelsize,PICINFO *ginfo)
{
int ret,i;

howfar=howfarfunc;           

*pixelsize=1;

if((ret=read_gif_multi(giffn))!=_PIC_OK)
  {
  images_cleanup();
  return(ret);
  }

if(imagecount==1)
  {
  /* copy stuff from images[0] to the usual image etc. */
  
  ginfo->width=images[0]->width;
  ginfo->height=images[0]->height;
  ginfo->bpp=(images[0]->misc&0x07)+1;
  ginfo->numcols=lnumcols?lnumcols:gnumcols;
  }
else
  {
  ginfo->width=swidth;
  ginfo->height=sheight*imagecount;
  
  /* this is complicated, so we lie :-) */
  ginfo->bpp=8;
  ginfo->numcols=256;
  }

/* allocate main image and palette */

/* width/height already checked in read_gif_multi(), here we
 * check the number of images and their size won't cause a problem.
 */
if(imagecount>1 && (imagecount>MAX_GIF_IMAGES ||
                    swidth*sheight>MAX_GIFMULTI_BYTES_PER_IMAGE))
  {
  images_cleanup();
  return(_PICERR_NOMEM);
  }

if((*theimageptr=(byte *)malloc(ginfo->width*ginfo->height))==NULL)
  {
  images_cleanup();
  return(_PICERR_NOMEM);
  }

if((*palptr=(byte *)malloc(768))==NULL)
  {
  images_cleanup();
  /* theimage freed in vgadisp.c */
  return(_PICERR_NOMEM);
  }

if(imagecount==1)
  {
  memcpy(*theimageptr,images[0]->image,ginfo->width*ginfo->height);
  
  if(images[0]->cmap)
    memcpy(*palptr,images[0]->cmap,768);
  else
    {
    if(global_cmap)
      memcpy(*palptr,global_cmap,768);
    else
      {
      images_cleanup();
      /* theimage and palette freed in vgadisp.c */
      return(_PICERR_NOCOLOURMAP);
      }
    }
  }
else	/* if more than one image... */
  {
  unsigned char *ptr;

  /* put global colourmap in first, and overwrite with local if present
   * (or give error if neither exist).
   */
  memset(*palptr,0,768);	/* avoid vkludge problems for <=128-col */
  
  if(global_cmap)
    for(i=0;i<gnumcols;i++)
      {
      (*palptr)[i    ]=global_cmap[i    ];
      (*palptr)[i+256]=global_cmap[i+256];
      (*palptr)[i+512]=global_cmap[i+512];
      }
  
  if(images[0]->cmap)
    for(i=0;i<images[0]->numcols;i++)
      {
      (*palptr)[i    ]=images[0]->cmap[i    ];
      (*palptr)[i+256]=images[0]->cmap[i+256];
      (*palptr)[i+512]=images[0]->cmap[i+512];
      }
  
  if(!images[0]->cmap && !global_cmap)
    {
    images_cleanup();
    /* theimage and palette freed in vgadisp.c */
    return(_PICERR_NOCOLOURMAP);
    }
  
  ptr=*theimageptr;
  memset(ptr,0,swidth*sheight);
  
  for(i=0;i<imagecount;i++)
    {
    int x,y,left,w;
    unsigned char *ptr1,*ptr2,*oldptr1;

    /* basic width/height vs. "screen" checks, left/top handled elsewhere */
    if(images[i]->width>swidth) images[i]->width=swidth;
    if(images[i]->height>sheight) images[i]->height=sheight;
    
    /* for images after the first, we need to set the initial contents
     * (as far as GIF is concerned, the `screen' contents) as directed
     * by the graphics control block.
     */
    if(i>0)
      {
      switch((images[i-1]->gcb_control&0x1c)>>2)	/* "disposal method" */
        {
        case 0:  /* unspecified */
        case 1:  /* "do not dispose", i.e. copy old one as basis for new */
          memcpy(ptr,ptr-swidth*sheight,swidth*sheight);
          break;
        
        case 2:  /* restore to background colour */
          memcpy(ptr,ptr-swidth*sheight,swidth*sheight);
          left=images[i-1]->left;
          w=images[i-1]->width;
          if(left>=swidth) break;
          if(left+w>swidth) w=swidth-left;
          
          for(y=images[i-1]->top;
              y<sheight && y<images[i-1]->top+images[i-1]->height;y++)
            memset(ptr+y*swidth+left,0,w);
          break;
        
        case 3:  /* restore to previous contents */
          /* XXX this says "gifhed.back", above says "0" - which is right? */
          if(i==1)
            memset(ptr,gifhed.back,swidth*sheight);
          else 
            memcpy(ptr,ptr-swidth*sheight*2,swidth*sheight);
          break;
        
        /* the spec says of the other cases, "4-7 -    To be defined."
         * So they're silently ignored.
         */
        }
      }

    /* an image with left or top offscreen is broken, but relying
     * unknowingly on the image not appearing at all. So skip it.
     */
    if(images[i]->left<swidth && images[i]->top<sheight)
      {
      ptr1=ptr+images[i]->left+images[i]->top*swidth;
      
      for(y=0;y<images[i]->height && images[i]->top+y<sheight;y++)
        {
        oldptr1=ptr1;
        ptr2=images[i]->image+y*images[i]->width;
        
        for(x=0;x<images[i]->width && images[i]->left+x<swidth;x++)
          if(!(images[i]->gcb_control&1) || /* if no transparent col defined */
               images[i]->transparent_col!=*ptr2)
            *ptr1++=*ptr2++;
          else
            ptr1++,ptr2++;

        ptr1=oldptr1+swidth;
        }
      }
    
    ptr+=swidth*sheight;
    }
  }	/* end of multi-image case */


/* export stuff for animated GIF */

gif_delaycount=imagecount;

/* remove any old array */
if(gif_delay) free(gif_delay);

if((gif_delay=malloc(sizeof(int)*imagecount))==NULL)
  {
  /* just don't allow animation :-) */
  gif_delaycount=0;
  }
else
  for(i=0;i<imagecount;i++)
    {
    /* XXX should gcb_control's "user input flag" override delay? */
    gif_delay[i]=images[i]->delay;
    }

images_cleanup();

return(_PIC_OK);
}


void aborted_file_gif_cleanup(void)
{
/* image and palette haven't been allocated when this is called,
 * so it's just these...
 */
fclose(global_gif_infile);
images_cleanup();
}
