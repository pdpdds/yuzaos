/* zgv 5.3 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * readprf.c - read PRF files, heavily based on Brian Raiter's `prftopnm'.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zgv.h"
#include "rcfile.h"
#include "rc_config.h"
#include "readpnm.h"	/* for dithering routines */
#include "readprf.h"

#define squaresize	64

/* for aborted_file_prf_cleanup() */
static unsigned char *work_bmap,*work_pal,*work_planebuf;
static FILE *work_in;

static int width,height,bits,planes;
static unsigned char *planebuf[4],*imagebuf;
static int fbitcount,fbyte;



static int bitsin(FILE *in,int nbits,unsigned char *val)
{
int bit;

*val=0;
for(bit=1<<(nbits-1);bit;bit>>=1)
  {
  if(fbitcount++&7)
    fbyte<<=1;
  else
    if((fbyte=fgetc(in))==EOF)
      return(0);
  if(fbyte&0x80)
    *val|=bit;
  }

return(1);
}

static int decodesquare(FILE *in,int ypos,int xpos,int size,int cb,unsigned char pixel)
{
static int const bitsize[]={0,1,2,2,3,3,3,3,4};
unsigned char *square;
unsigned char *sq;
unsigned char byte;
unsigned char count;
int y,n;

if(ypos>=height || xpos>=width)
  return(1);

square=imagebuf+(ypos%squaresize)*width+xpos;

if(size==1)
  {
  byte=0;
  if(cb)
    if(!bitsin(in,cb,&byte))
      return(0);
  *square=pixel|byte;
  return(1);
  }

if(!bitsin(in,bitsize[cb],&count))
  return(0);

cb-=count;
if(count) 
  {
  if(!bitsin(in,count,&byte))
    return(0);
  
  pixel|=byte<<cb;
  }

if(!cb) 
  {
  n=width-xpos;  if(n>size) n=size;
  y=height-ypos; if(y>size) y=size;
  
  for(sq=square;y;y--,sq+=width)
    memset(sq,pixel,n);
  
  return(1);
  }

n=size>>1;

return(decodesquare(in,ypos,xpos,n,cb,pixel)
       && decodesquare(in,ypos,xpos+n,n,cb,pixel)
       && decodesquare(in,ypos+n,xpos,n,cb,pixel)
       && decodesquare(in,ypos+n,xpos+n,n,cb,pixel));
}


int readprfrow(FILE *in,int ypos)
{
int x,z;

if(ypos>=height)
  return(0);

for(z=0;z<planes;z++)
  {
  imagebuf=planebuf[z];
  for(x=0;x<width;x+=squaresize)
    if(!decodesquare(in,ypos,x,squaresize,bits,0))
      return(-1);
  }

return(1);
}


/* output_type returns how many bytes per pixel needed for display */
int read_prf_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **palette,int *output_type,PICINFO *pp)
{
FILE *in;
int f,n;
unsigned char buf[13],*src[4],*dst;
int bytepp,ypos;
int x,y;
int dithering=0;
int maxval;
int lookup[256];

*bmap=NULL;
*palette=NULL;
fbitcount=0;
fbyte=0;

if((in=work_in=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

if(fread(buf,1,13,in)!=13)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if(strncmp(buf,"PRF1",4)!=0)
  CLOSE_AND_RET(_PICERR_BADMAGIC);

width=(buf[4]<<24)|(buf[5]<<16)|(buf[6]<<8)|buf[7];
height=(buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
bits=(buf[12]&31)+1;
planes=((buf[12]>>5)&7)+1;

maxval=(1<<bits)-1;

if(width<=0 || height<=0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

/* XXX should it try to read those with more bits? */
if(bits>8)
  CLOSE_AND_RET(_PICERR_TOOMANYCOLS);

/* we support grey, RGB, and RGBA (but with alpha ignored) */
if(planes!=1 && planes!=3 && planes!=4)
  CLOSE_AND_RET(_PICERR_UNSUPPORTED);

/* always allocated */
if((*palette=work_pal=calloc(768,1))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

for(f=0;f<=maxval;f++)
  lookup[f]=(f*255)/maxval;
for(;f<256;f++)
  lookup[f]=0;

bytepp=3;
if(planes==1)
  {
  unsigned char *pal=*palette;
  
  bytepp=1;
  for(f=0;f<256;f++)
    pal[f]=pal[256+f]=pal[512+f]=lookup[f];
  }

n=width*squaresize;
if(WH_BAD(width,height) || (planebuf[0]=work_planebuf=calloc(n,planes))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);
for(f=1;f<planes;f++)
  planebuf[f]=planebuf[f-1]+n;

if(bytepp==3 && (*output_type==1 || cfg.jpeg24bit==0))	/* dither? */
  {
  dithering=1;
  bytepp=1;
  make_332_palette(*palette);
  if(ditherinit(width)==0)
    {
    free(planebuf[0]);
    CLOSE_AND_RET(_PICERR_NOMEM);
    }
  }

/* add the usual extra 2 lines in case of dithering */
/* width/height check already done, but WTF :-) */
if(WH_BAD(width,height) ||
   (*bmap=work_bmap=malloc(width*(height+2)*planes))==NULL)
  {
  free(planebuf[0]);
  CLOSE_AND_RET(_PICERR_NOMEM);
  }

ypos=0;

while((f=readprfrow(in,ypos)))
  {
  if(f<0)
    {
    if(dithering)
      ditherfinish();
    free(planebuf[0]);
    free(*bmap);
    *bmap=NULL;
    CLOSE_AND_RET(_PICERR_CORRUPT);
    }

  switch(planes)
    {
    case 1:
      memcpy(*bmap+ypos*width,planebuf[0],
             (ypos+squaresize>=height?height-ypos:squaresize)*width);
      break;
      
    case 3: case 4:
      /* more complicated - need to turn separate R,G,B planes into
       * interleaved BGR. And then, possibly fix depth, and
       * possibly dither it.
       */
      for(y=0;y<squaresize && ypos+y<height;y++)
        {
        for(x=0;x<3;x++)
          src[x]=planebuf[x]+y*width;
        
        dst=*bmap+(ypos+y)*width*bytepp;

        for(x=0;x<width;x++)
          {
          *dst++=*src[2]++;
          *dst++=*src[1]++;
          *dst++=*src[0]++;
          }

        /* fix bit-depth if bits<8 */
        if(bits<8)
          {
          for(x=0;x<width*3;x++)
            {
            --dst;
            *dst=lookup[*dst];
            }
          }
        
        if(dithering)
          ditherline(*bmap+(ypos+y)*width,ypos+y,width);
        }
      
      break;
    }

  ypos+=squaresize;
  
  /* difficult to do meaningful howfar for this, but we give it a go... */
  /* the icky *10 stuff is to defeat %10 in zgv.c's showhowfar */
  if(howfarfunc!=NULL)
    howfarfunc((ypos>height?height:ypos)*10,height*10);
  }

if(dithering)
  ditherfinish();

free(planebuf[0]);
fclose(in);

*output_type=bytepp;
pp->width=width;
pp->height=height;
pp->numcols=256;

return(_PIC_OK);
}


void aborted_file_prf_cleanup()
{
free(work_planebuf);
free(work_bmap);
free(work_pal);
fclose(work_in);
}
