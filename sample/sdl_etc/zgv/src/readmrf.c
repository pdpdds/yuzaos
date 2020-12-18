/* Zgv v3.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * readmrf.c - read the 1-bit mono `mrf' format. based on my `mrftopbm'.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zgv.h"
#include "readmrf.h"


/* for aborted_file_mrf_cleanup() */
static unsigned char *work_bmap,*work_pal;
static FILE *work_in;

static int bitbox,bitsleft;



void bit_init()
{
bitbox=0; bitsleft=0;
}


int bit_input(FILE *in)
{
if(bitsleft==0)
  {
  bitbox=fgetc(in);
  bitsleft=8;
  }

bitsleft--;
return((bitbox&(1<<bitsleft))?1:0);
}


void do_square(FILE *in,unsigned char *image,int ox,int oy,int w,int size)
{
int x,y,c;

/* is it all black or white? */

if(size==1 || bit_input(in))
  {
  /* yes, next bit says which. */
  c=bit_input(in);
  for(y=0;y<size;y++)
    for(x=0;x<size;x++)
      image[(oy+y)*w+ox+x]=c;
  }
else
  {
  /* not all one colour, so recurse. */
  size>>=1;
  do_square(in,image,ox,oy,w,size);
  do_square(in,image,ox+size,oy,w,size);
  do_square(in,image,ox,oy+size,w,size);
  do_square(in,image,ox+size,oy+size,w,size);
  }
}



int read_mrf_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **pal,int *output_type,PICINFO *pp)
{
FILE *in;
int w,h,w64,h64,x,y;
unsigned char buf[13],*image;
int totalsq;

*bmap=NULL;
*pal=NULL;

if((in=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

fread(buf,1,13,in);

if(strncmp(buf,"MRF1",4)!=0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if(buf[12]!=0)
  CLOSE_AND_RET(_PICERR_UNSUPPORTED);

w=((buf[4]<<24)|(buf[5]<<16)|(buf[6]<<8)|buf[7]);
h=((buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11]);

if(w==0 || h==0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if((*pal=calloc(768,1))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

(*pal)[0]=(*pal)[256]=(*pal)[512]=0;
(*pal)[1]=(*pal)[257]=(*pal)[513]=255;

/* w64 is units-of-64-bits width, h64 same for height */
w64=(w+63)/64;
h64=(h+63)/64;

if(WH_BAD(w64*64,h64*64) || WH_BAD(w,h) ||
   (*bmap=malloc(w*h))==NULL ||
   (image=calloc(w64*h64*64*64,1))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

/* save stuff in case of abort */
work_in=in; work_bmap=(*bmap); work_pal=*pal;

/* now recursively input squares. */

/* init bit input */
bit_init();

totalsq=w64*h64;

for(y=0;y<h64;y++)
  for(x=0;x<w64;x++)
    {
    /* difficult to do meaningful howfar for this, but we give it a go... */
    /* the icky *10 stuff is to defeat %10 in zgv.c's showhowfar */
    if(howfarfunc!=NULL) howfarfunc((y*w64+x)*10,totalsq*10);
    do_square(in,image,x*64,y*64,w64*64,64);
    }

fclose(in);

/* write real image */
for(y=0;y<h;y++)
  memcpy(*bmap+y*w,image+y*w64*64,w);

free(image);

pp->width=w;
pp->height=h;
pp->numcols=2;

*output_type=1;

return(_PIC_OK);  
}


void aborted_file_mrf_cleanup()
{
free(work_bmap);
free(work_pal);
fclose(work_in);
}
