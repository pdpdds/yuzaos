/* zgv 5.5 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * readtga.c - loads TGA files (based on readpnm.c).
 *
 * only supports file types 1, 2, 9 and 10, since that's all I've
 * got documentation for. If you want support for the others,
 * write it yourself. ;-)
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "zgv.h"
#include "readtga.h"
#include "readpnm.h"	/* for dithering routines */
#include "rcfile.h"
#include "rc_config.h"


/* for aborted_file_tga_cleanup() */
static unsigned char *work_bmap,*work_pal;
static FILE *work_in;

int tga_need_flip;


/* prototypes */
/* (none needed) */


/* convert 16-bit at ptr to 24-bit. */
static void fix16bit(unsigned char *ptr,int width)
{
static unsigned char scaleup[32];
static int first=1;
unsigned char *src,*dst;

if(first)
  {
  int f;
  
  first=0;
  for(f=0;f<32;f++)
    scaleup[f]=(f*255)/31;
  }

/* does it right-to-left to avoid needing temporary buffer */
dst=ptr+width*3;
for(src=ptr+width*2;src>ptr;src-=2)
  {
  *--dst=scaleup[(src[-1]>>2)&31];
  *--dst=scaleup[src[-2]>>5 | (src[-1]&3)<<3];
  *--dst=scaleup[src[-2]&31];
  }
}


int read_tga_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **pal,int *output_type,PICINFO *pp)
{
FILE *in;
struct tgahed hed;
int c,w,h,bytepp,x,y,i,cmapstart,cmaplen;
int rle_left=0,rle_pkt=0;
unsigned char rle_byte[3],*rle_ptr=NULL;
unsigned char *ptr;
int filebytepp=1;

*bmap=NULL;
*pal=NULL;

if((in=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

if(fread(&hed,sizeof(hed),1,in)!=1)
  CLOSE_AND_RET(_PICERR_BADMAGIC);

cmapstart=(hed.cmapstart_hi<<8)+hed.cmapstart_lo;
cmaplen  =(hed.cmaplen_hi<<8)  +hed.cmaplen_lo;

switch(hed.type)
  {
  case 1:
  case 9:
    bytepp=1;
    break;
  case 2:
  case 10:
    bytepp=3;
    filebytepp=hed.bpp/8;
    if(filebytepp<1 || filebytepp>3)
      filebytepp=3;
    break;
  default:
    CLOSE_AND_RET(_PICERR_BADMAGIC);
  }

/* skip any id field */
for(i=0;i<hed.idfieldlen;i++) fgetc(in);

if((*pal=calloc(768,1))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);
else
  {
  int r,g,b;
  ptr=*pal;

  /* only actually used if using 8-bit display, but defined always */
  for(r=0;r<8;r++)
    for(g=0;g<8;g++)	/* colours are 3:3:2 */
      for(b=0;b<4;b++)
        {
        *ptr=r*255/7; ptr[256]=g*255/7; ptr[512]=b*255/3;
        ptr++;
        }
  }

/* read colour map if there is one - note we actually skip it entirely
 * if some hoser put a colourmap on a 24-bit file (which is possible
 * with TGA apparently; duh, I'll have the 16 meg palette please Bob).
 */
if(hed.hascmap)
  {
  int bytecmapd;
  unsigned char tmp[4];
  
  if(cmapstart+cmaplen>256)
    CLOSE_AND_RET(_PICERR_CORRUPT);		/* just in case... */
  if(hed.cmapdepth!=16 && hed.cmapdepth!=24 && hed.cmapdepth!=32)
    CLOSE_AND_RET(_PICERR_CORRUPT);

  bytecmapd=hed.cmapdepth/8;
  if(hed.type==2 || hed.type==10)	/* supported RGB types */
    {
    /* skip colourmap */
    for(i=0;i<cmaplen;i++) fread(tmp,bytecmapd,1,in);
    }
  else
    {
    /* read colourmap */
    ptr=*pal;
    for(i=0;i<cmaplen;i++)
      {
      fread(tmp,bytecmapd,1,in);
      switch(hed.cmapdepth)
        {
        case 16:
          c=((tmp[1]<<8)|tmp[0]);
          ptr[    cmapstart+i]=(( c     &31)*255)/31;
          ptr[256+cmapstart+i]=(((c>> 5)&31)*255)/31;
          ptr[512+cmapstart+i]=(((c>>10)&31)*255)/31;
        case 24: case 32:
          ptr[    cmapstart+i]=tmp[2];
          ptr[256+cmapstart+i]=tmp[1];
          ptr[512+cmapstart+i]=tmp[0];
        }
      }
    }
  }

w=(hed.width_hi<<8) +hed.width_lo;
h=(hed.height_hi<<8)+hed.height_lo;

if(w==0 || h==0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if(bytepp==3 && (*output_type==1 || cfg.jpeg24bit==0))	/* dither? */
  {
  bytepp=1;
  if(ditherinit(w)==0)
    CLOSE_AND_RET(_PICERR_NOMEM);
  }

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

tga_need_flip=(hed.desc&32)?0:1;

/* read in the image
 * no errors are detected once we start reading.
 * now, of course, TGA files are stored backwards. rather than
 * try to cope with this, I *completely* ignore it until the whole
 * image is read in, then I flip the image. :-)
 */
switch(hed.type)
  {
  case 1: case 2:
    /* uncompressed */
    for(y=0;y<h;y++)
      {
      fread(ptr,filebytepp,w,in);
      
      if(filebytepp==2)
        fix16bit(ptr,w);
      
      /* dither if required */
      if(hed.type==2 && bytepp==1)
        ditherline(ptr,y,w);

      ptr+=bytepp*w;
      if(howfarfunc!=NULL) howfarfunc(y,h);
      }
    break;
  
  case 9: case 10:
    /* RLE compressed */
    rle_left=0;
    for(y=0;y<h;y++)
      {
      static unsigned char buf[128*3];
      
      for(x=0;x<w;x++)
        {
        if(rle_left==0)
          {
          c=fgetc(in);
          rle_pkt=(c&128)?1:0;
          rle_left=(c&127)+1;
          if(rle_pkt)
            fread(rle_ptr=rle_byte,filebytepp,1,in);
          else
            fread(rle_ptr=buf     ,filebytepp,rle_left,in);
          }
        
        memcpy(ptr,rle_ptr,filebytepp);
        if(!rle_pkt) rle_ptr+=filebytepp;
        rle_left--;
        ptr+=filebytepp;
        }
      
      if(filebytepp==2)
        fix16bit(ptr-filebytepp*w,w);
      
      /* dither if required */
      if(hed.type==10 && bytepp==1)
        {
        ptr-=3*w;
        ditherline(ptr,y,w);
        ptr+=w;
        }
      
      if(howfarfunc!=NULL) howfarfunc(y,h);
      }
    break;
  }

pp->width=w;
pp->height=h;
pp->numcols=256;

*output_type=bytepp;

if((hed.type==2 || hed.type==10) && bytepp==1)
  ditherfinish();
  
fclose(in);

/* the image is actually flipped in vgadisp.c */

return(_PIC_OK);  
}


void aborted_file_tga_cleanup()
{
free(work_bmap);
free(work_pal);
fclose(work_in);
}
