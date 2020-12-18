/* zgv 5.3 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * readtiff.c - read TIFF files. Based on xzgv's readtiff.c.
 */

#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>
//#include <sys/file.h>  /* for open et al */
#include <tiffio.h>

#include "zgv.h"
#include "vgadisp.h"
#include "rcfile.h"
#include "rc_config.h"
#include "readpnm.h"	/* for dithering stuff */

#include "readtiff.h"


/* redefine CLOSE_AND_RET to use TIFFClose */
#undef CLOSE_AND_RET
#define CLOSE_AND_RET(x)	do { TIFFClose(in); return(x); } while(0)


static unsigned char *work_bmap,*work_pal;
static TIFF *work_in;


void aborted_file_tiff_cleanup(void)
{
free(work_bmap);
free(work_pal);
TIFFClose(work_in);
}


/* output_type returns how many bytes per pixel needed for display */
int read_tiff_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **palette,int *output_type,PICINFO *pp)
{
TIFF *in;
FILE *tmp;
unsigned char *src,*dst,*ptr,*pal,*image;
int width,height;
int f,y,numpix,w3,r,g,b;
int bytepp=3;

*bmap=NULL;

if(((*palette)=malloc(768))==NULL)
  return(_PICERR_NOMEM);
pal=work_pal=*palette;

make_332_palette(pal);

TIFFSetErrorHandler(NULL);	/* no error messages */
TIFFSetWarningHandler(NULL);	/* no warning messages either */

/* first see if we can open it. This should catch *most* file-not-found
 * errors, so we can avoid worrying them with the `corrupt' business. :-)
 */
if((tmp=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

fclose(tmp);

if((in=work_in=TIFFOpen(filename,"r"))==NULL)
  return(_PICERR_CORRUPT);	/* probably a missing TIFF directory? */

TIFFGetField(in,TIFFTAG_IMAGEWIDTH,&width);
TIFFGetField(in,TIFFTAG_IMAGELENGTH,&height);

if(*output_type==1 || cfg.jpeg24bit==0)		/* dither? */
  {
  bytepp=1;
  if(ditherinit(width)==0)
    CLOSE_AND_RET(_PICERR_NOMEM);
  }

/* an extra width*3 guarantees there'll be at least one line
 * spare for the flip afterwards, but we need twice that to be
 * certain the dithering has room.
 */
numpix=width*height;
if(WH_BAD(width,height) ||
   (image=*bmap=work_bmap=malloc(numpix*sizeof(uint32)+width*3*2))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

/* XXX what about hffunc!? */
if(!TIFFReadRGBAImage(in,width,height,(uint32 *)image,0))
  {
  /* caller frees pal/image on error, so just this... */
  CLOSE_AND_RET(_PICERR_CORRUPT);
  }

TIFFClose(in);


/* This is a pretty crappy way to work :-), but the alternative
 * way (supplying routines to write any contiguous/planar RGBA chunks
 * you get passed) is, stunningly, even worse.
 */

/* RGBA to BGR */
src=dst=image;

/* first two pixels are a bit awkward */
for(f=0;f<2 && f<numpix;f++,dst+=3)
  {
  r=*src++; g=*src++; b=*src++; src++;
  dst[2]=b; dst[1]=g; *dst=r;
  }

/* the rest can be done a bit more sanely */
for(f=2;f<numpix;f++,dst+=3)
  {
  dst[2]=*src++;
  dst[1]=*src++;
  *dst=*src++;
  src++;
  }

/* flip the image vertically */
src=image;
w3=width*3;
dst=image+(height-1)*w3;
ptr=dst+w3;	/* use extra space for temp row */
for(f=0;f<height/2;f++)
  {
  memcpy(ptr,src,w3);
  memcpy(src,dst,w3);
  memcpy(dst,ptr,w3);
  src+=w3;
  dst-=w3;
  }

/* XXX would be better to combine this with one of the above steps.
 * Would also be hairy, which is why I've not done it. :-)
 */
if(bytepp==1)
  {
  for(y=0,ptr=image;y<height;y++,ptr+=width*3)
    {
    ditherline(ptr,y,width);
    if(y>0) memcpy(image+y*width,ptr,width);
    if(howfarfunc!=NULL) howfarfunc(height+y,height*2);
    }
  
  ditherfinish();
  }

*bmap=realloc(*bmap,numpix*bytepp);

/* If the realloc() fails (which it can't?), the memory will still be
 * there as before, so we can still return ok.
 */

*output_type=bytepp;
pp->width=width;
pp->height=height;
pp->numcols=256;

return(_PIC_OK);
}
