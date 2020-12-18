/* Zgv v5.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2000 Russell Marks. See README for license details.
 *
 * readpnm.c - PBM/PGM/PPM loader.
 *             PBM/PGM are loaded as 8-bit, PPM as 24-bit.
 *             if 8-bit display, PPM is dithered.
 *             xv 3:3:2 thumbnails also.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "zgv.h"
#include "rcfile.h"
#include "rc_config.h"
#include "readpnm.h"


/* redefine CLOSE_AND_RET to use closefunc (either fclose or pclose) */
#undef CLOSE_AND_RET
#define CLOSE_AND_RET(x)	do { closefunc(in); return(x); } while(0)


/* for aborted_file_pnm_cleanup() */
static unsigned char *work_bmap,*work_pal;
static FILE *work_in;
static int (*work_closefunc)(FILE *);

/* for dither routine */
static int *evenerr=NULL,*odderr=NULL;
static unsigned char *dbuf=NULL;


/* prototypes */
void ditch_line(FILE *in);
int read_next_number(FILE *in);
void aborted_file_pnm_cleanup(void);
int ditherinit(int w);
void ditherfinish(void);
void ditherline(unsigned char *theline, int linenum, int width);
int read_xv332(char *filename, unsigned char **bmap, int *width, int *height);
void make_332_palette(unsigned char *palptr);



/* this works in a slightly confusing way so that the main PNM
 * reading code can be used for reading from a filter run with popen(),
 * e.g. tifftopnm.
 */


/* output_type returns how many bytes per pixel needed for display */
int read_pnm_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **pal,int *output_type,PICINFO *pp)
{
FILE *in;

*bmap=NULL;
*pal=NULL;

if((in=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

return(read_pnm_main(in,howfarfunc,bmap,pal,output_type,pp,fclose));
}


int read_pnm_main(FILE *in,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **pal,int *output_type,PICINFO *pp,
                  int (*closefunc)(FILE *))
{
int c,sub,raw,w,h,maxval,bytepp,x,y,i,n,red,grn,blu,max_is_big;
unsigned char *ptr;

*bmap=NULL;
*pal=NULL;

c=fgetc(in);
if(c!='P')
  CLOSE_AND_RET(_PICERR_BADMAGIC);

sub=fgetc(in)-48;
if(sub<1 || sub>6)
  CLOSE_AND_RET(_PICERR_BADMAGIC);

fgetc(in);

raw=0;
if(sub>3)
  {
  raw=1;
  sub-=3;
  }

bytepp=(sub==3)?3:1;
if((*pal=calloc(768,1))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);
else
  {
  ptr=*pal;
  switch(sub)
    {
    case 3:
      /* only actually used if using 8-bit display, but defined always */
      make_332_palette(ptr);
      break;
    case 2:
      for(x=0;x<256;x++,ptr++)
        *ptr=ptr[256]=ptr[512]=x;
      break;
    case 1:
      ptr[0]=ptr[256]=ptr[512]=0;
      ptr[1]=ptr[257]=ptr[513]=255;
    }
  }

w=read_next_number(in);
h=read_next_number(in);

if(w==0 || h==0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if(sub==1)
  maxval=1;
else
  if((maxval=read_next_number(in))==0)
    CLOSE_AND_RET(_PICERR_CORRUPT);

max_is_big=(maxval>255);

if(sub==3 && (*output_type==1 || cfg.jpeg24bit==0))	/* dither PPM? */
  {
  bytepp=1;
  if(ditherinit(w)==0)
    CLOSE_AND_RET(_PICERR_NOMEM);
  }

if(sub==2 && cfg.betterpgm && cfg.jpeg24bit && (*output_type)==3)
  bytepp=3;	/* grind it to 24-bit */

/* we allocate two blank lines at the end. reason is the dithering
 * of PPM files to 8-bit works a line at a time, and we need
 * 3 times as much for each line, which works out only meaning
 * 3x as much for the last line. If you see what I mean. (!?)
 */
if(WH_BAD(w,h) || (*bmap=malloc(w*(h+2)*bytepp))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);


ptr=*bmap;

/* save stuff in case of abort */
work_in=in; work_bmap=ptr; work_pal=*pal;
work_closefunc=closefunc;

/* finally!, we get to read in the image */
/* no errors are detected once we start reading. */
for(y=0;y<h;y++)
  {
  for(x=0;x<w;x++)
    {
    if(raw)
      {
      switch(sub)
        {
        case 1: /* PBM */
          c=fgetc(in);
          for(i=0,n=128;i<8 && x+i<w;i++,n>>=1)
            *ptr++=(c&n)?0:1;
          x+=7;
          break;
        case 2: /* PGM */
          if(max_is_big)
            c=(fgetc(in)<<8),*ptr++=((c|fgetc(in))*255)/maxval;	/* 16-bit */
          else
            *ptr++=(fgetc(in)*255)/maxval;			/* 8-bit */
          break;
        case 3: /* PPM */
          if(max_is_big)
            {
            c=(fgetc(in)<<8),red=((c|fgetc(in))*255)/maxval;
            c=(fgetc(in)<<8),grn=((c|fgetc(in))*255)/maxval;
            c=(fgetc(in)<<8),blu=((c|fgetc(in))*255)/maxval;
            }
          else
            {
            red=(fgetc(in)*255)/maxval;
            grn=(fgetc(in)*255)/maxval;
            blu=(fgetc(in)*255)/maxval;
            }
          
          *ptr++=blu; *ptr++=grn; *ptr++=red;
        }
      }
    else
      {
      switch(sub)
        {
        case 1:
          do
            if((c=fgetc(in))=='#') ditch_line(in);
          while(c!='0' && c!='1' && c!=EOF);
          *ptr++=(c=='1')?0:1;
          break;
        case 2:
          *ptr++=(read_next_number(in)*255)/maxval;
          break;
        case 3:
          red=(read_next_number(in)*255)/maxval;
          grn=(read_next_number(in)*255)/maxval;
          blu=(read_next_number(in)*255)/maxval;
          *ptr++=blu; *ptr++=grn; *ptr++=red;
        }
      }		/* end of 'if(raw)' */

    /* if PGM and bytepp=3, but have cfg.betterpgm set */
    if(sub==2 && bytepp==3)
      {
      c=ptr[-1];
      *ptr++=c; *ptr++=c;
      }
    }

  /* if PPM and 8-bit display, dither now */
  if(sub==3 && bytepp==1)
    {
    ptr-=w*3;
    ditherline(ptr,y,w);
    ptr+=w;
    }
    
  if(howfarfunc!=NULL) howfarfunc(y,h);
  }

pp->width=w;
pp->height=h;
pp->numcols=(sub==1)?2:256;

*output_type=bytepp;

if(sub==3 && bytepp==1)
  ditherfinish();
  
closefunc(in);
return(_PIC_OK);  
}


void ditch_line(FILE *in)
{
int c;

while((c=fgetc(in))!='\n' && c!=EOF);
}


/* for text-style PNM files, i.e. P[123].
 * we take an extremely generous outlook - anything other than a decimal
 * digit is considered whitespace.
 * and as per p[bgp]m(5), comments can start anywhere.
 */
int read_next_number(FILE *in)
{
int c,num,in_num,gotnum;

num=0;
in_num=gotnum=0;

do
  {
  if(feof(in)) return(0);
  if((c=fgetc(in))=='#')
    ditch_line(in);
  else
    if(isdigit(c))
      num=10*num+c-49+(in_num=1);
    else
      gotnum=in_num;
  }
while(!gotnum);  

return(num);
}


void aborted_file_pnm_cleanup()
{
free(work_bmap);
free(work_pal);
work_closefunc(work_in);
}


int ditherinit(int w)
{
if(WH_BAD(w+10,sizeof(int))) return(0);

ditherfinish();		/* make sure any previous mem is unallocated */
if((evenerr=calloc(3*(w+10),sizeof(int)))==NULL ||
   (odderr =calloc(3*(w+10),sizeof(int)))==NULL ||
   (dbuf   =malloc(w))==NULL)
  return(0);
else
  return(1);
}


void ditherfinish()
{
if(evenerr!=NULL) { free(evenerr); evenerr=NULL; }
if(odderr!=NULL)  { free(odderr);  odderr=NULL; }
if(dbuf!=NULL)    { free(dbuf);    dbuf=NULL; }
}


void ditherline(unsigned char *theline,int linenum,int width)
{
int x,y,lx;
int c0,c1,c2,times2;
int terr0,terr1,terr2,actual0,actual1,actual2;
int start,addon,r,g,b;
int *thiserr;
int *nexterr;

y=linenum;
if((y&1)==0)
  {start=0; addon=1;
  thiserr=evenerr+3; nexterr=odderr+width*3;}
else
  {start=width-1; addon=-1;
  thiserr=odderr+3; nexterr=evenerr+width*3;}
nexterr[0]=nexterr[1]=nexterr[2]=0;
x=start;
for(lx=0;lx<width;lx++)
  {
  b=theline[x*3];
  g=theline[x*3+1];
  r=theline[x*3+2];

  terr0=r+((thiserr[0]+8)>>4);
  terr1=g+((thiserr[1]+8)>>4);
  terr2=b+((thiserr[2]+8)>>4);

  /* is this going to screw up on white? */  
  actual0=(terr0>>5)*255/7;
  actual1=(terr1>>5)*255/7;
  actual2=(terr2>>6)*255/3;
  
  if(actual0<0) actual0=0; if(actual0>255) actual0=255;
  if(actual1<0) actual1=0; if(actual1>255) actual1=255;
  if(actual2<0) actual2=0; if(actual2>255) actual2=255;
  
  c0=terr0-actual0;
  c1=terr1-actual1;
  c2=terr2-actual2;

  times2=(c0<<1);
  nexterr[-3] =c0; c0+=times2;
  nexterr[ 3]+=c0; c0+=times2;
  nexterr[ 0]+=c0; c0+=times2;
  thiserr[ 3]+=c0;

  times2=(c1<<1);
  nexterr[-2] =c1; c1+=times2;
  nexterr[ 4]+=c1; c1+=times2;
  nexterr[ 1]+=c1; c1+=times2;
  thiserr[ 4]+=c1;

  times2=(c2<<1);
  nexterr[-1] =c2; c2+=times2;
  nexterr[ 5]+=c2; c2+=times2;
  nexterr[ 2]+=c2; c2+=times2;
  thiserr[ 5]+=c2;

  dbuf[x]=(actual0>>5)*32+(actual1>>5)*4+(actual2>>6);

  thiserr+=3;
  nexterr-=3;
  x+=addon;
  }

memcpy(theline,dbuf,width);
}


/* xv 3:3:2 thumbnail files - these are similar to pgm raw files,
 * but the context in which they are used is very different; as such
 * we have a separate routine for loading them.
 * they seem to have a max. size of 80x60.
 */
int read_xv332(char *filename,unsigned char **bmap,int *width,int *height)
{
FILE *in;
char buf[128];
int w,h,maxval;
int count;

*bmap=NULL; *width=0; *height=0;

if((in=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

fgets(buf,sizeof(buf),in);
if(strcmp(buf,"P7 332\n")!=0)
  return(_PICERR_BADMAGIC);

/* we're not worried about any comments */
w=read_next_number(in);
h=read_next_number(in);

*width=w; *height=h;

if(w==0 || h==0 || w>80 || h>60)
  return(_PICERR_CORRUPT);

/* for some reason, they have a maxval...!?
 * we complain if it's not 255.
 */
if((maxval=read_next_number(in))!=255)
  return(_PICERR_CORRUPT);

if(WH_BAD(w,h) || (*bmap=malloc(w*h))==NULL)
  return(_PICERR_NOMEM);

count=fread(*bmap,1,w*h,in);
if(count!=w*h)
  return(_PICERR_CORRUPT);

fclose(in);
return(_PIC_OK);  
}


void make_332_palette(unsigned char *palptr)
{
int r,g,b;

for(r=0;r<8;r++)
  for(g=0;g<8;g++)	/* colours are 3:3:2 */
    for(b=0;b<4;b++)
      {
      *palptr=r*255/7; palptr[256]=g*255/7; palptr[512]=b*255/3;
      palptr++;
      }
}
