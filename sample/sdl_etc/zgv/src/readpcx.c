/* Zgv v3.3 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1999 Russell Marks. See README for license details.
 *
 * readpcx.c - PCX decoder. This is a quick hack so I can use/index a
 *		clip-art CD directly. :-)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zgv.h"
#include "readpcx.h"
#include "readpnm.h"	/* for dithering routines */
#include "rcfile.h"
#include "rc_config.h"

struct pcxhed
  {
  byte manuf,ver,encod,bpp;		/* 0 - 3 gen format */
  byte x1lo,x1hi,y1lo,y1hi;		/* 4 - 11 size */
  byte x2lo,x2hi,y2lo,y2hi;
  byte unused1[4];			/* 12 - 15 scrn size */
  byte pal16[48];			/* 16 - 63 4-bit palette */
  byte reserved;			/* 64 reserved */
  byte nplanes;				/* 65 num of bitplanes */
  byte bytelinelo,bytelinehi;		/* 66 - 67 bytes per line */
  byte unused2[60];			/* 68 - 127 unused */
  };  /* palette info is after image data */


/* for aborted_file_pcx_cleanup() */
static unsigned char *work_bmap,*work_pal;
static FILE *work_in;


/* prototypes */
static void dispbyte(unsigned char *ptr,int *xp,int *yp,int c,int w,int h,
	int real_bpp,int byteline,int dithering,int *planep,int *pmaskp);



int read_pcx_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **pal,int *output_type,PICINFO *pp)
{
FILE *in;
int w,h,bytepp,x,y,yy,byteline,plane,pmask;
unsigned char *ptr;
struct pcxhed header;
int count,waste;
long bytemax,bytesdone;
byte inbyte,inbyte2;
int real_bpp;		/* how many bpp file really is */
int dithering=0;

*bmap=NULL;
*pal=NULL;

if((in=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

fread(&header,1,sizeof(struct pcxhed),in);
if(header.manuf!=10 || header.encod!=1)
  CLOSE_AND_RET(_PICERR_UNSUPPORTED);

/* header.bpp=1, header.nplanes=1 = 1-bit.
 * header.bpp=1, header.nplanes=4 = 4-bit.
 * header.bpp=8, header.nplanes=1 = 8-bit.
 * header.bpp=8, header.nplanes=3 = 24-bit.
 * anything else gives an `unsupported' error.
 */
real_bpp=0; bytepp=1;
switch(header.bpp)
  {
  case 1:
    switch(header.nplanes)
      {
      case 1: real_bpp=1; break;
      case 4: real_bpp=4; break;
      }
    break;
  
  case 8:
    switch(header.nplanes)
      {
      case 1: real_bpp=8; break;
      case 3: real_bpp=24; bytepp=3; break;
      }
    break;
  }

if(!real_bpp)
  CLOSE_AND_RET(_PICERR_UNSUPPORTED);

if((*pal=calloc(768,1))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

w=(header.x2lo+256*header.x2hi)-(header.x1lo+256*header.x1hi)+1;
h=(header.y2lo+256*header.y2hi)-(header.y1lo+256*header.y1hi)+1;
byteline=header.bytelinelo+256*header.bytelinehi;

if(w==0 || h==0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if(bytepp==3 && (*output_type==1 || cfg.jpeg24bit==0))	/* dither? */
  {
  int r,g,b;
  
  bytepp=1;
  dithering=1;
  
  if(ditherinit(w)==0)
    CLOSE_AND_RET(_PICERR_NOMEM);

  ptr=*pal;
  for(r=0;r<8;r++)
    for(g=0;g<8;g++)	/* colours are 3:3:2 */
      for(b=0;b<4;b++)
        {
        *ptr=r*255/7; ptr[256]=g*255/7; ptr[512]=b*255/3;
        ptr++;
        }
  }

x=0; y=0;
bytemax=w*h;
if(real_bpp==1 || real_bpp==4)
  bytemax=(1<<30);	/* we use a 'y<h' test instead for these files */

/* the normal +2 lines in case we're dithering a 24-bit file */
if(WH_BAD(w,h) || (*bmap=malloc(w*(h+2)*bytepp))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

/* need this if more than one bitplane */
memset(*bmap,0,w*h*bytepp);

bytesdone=0;
ptr=*bmap;

/* save stuff in case of abort */
work_in=in; work_bmap=ptr; work_pal=*pal;

/* start reading image */
for(yy=0;yy<h;yy++)
  {
  if(howfarfunc!=NULL) howfarfunc(y,h);
  
  plane=0; pmask=1;
  
  y=yy;
  x=0;
  while(y==yy)
    {
    inbyte=fgetc(in);
    if(inbyte<192)
      {
      dispbyte(ptr,&x,&y,inbyte,w,h,real_bpp,
      	byteline,dithering,&plane,&pmask);
      bytesdone++;
      }
    else
      {
      inbyte2=fgetc(in);
      inbyte&=63;
      for(count=0;count<inbyte;count++)
        dispbyte(ptr,&x,&y,inbyte2,w,h,real_bpp,
        	byteline,dithering,&plane,&pmask);
      bytesdone+=inbyte;
      }
    }
  
  /* dither if required */
  if(dithering)
    ditherline(ptr+yy*w*bytepp,yy,w);
  }

/* read palette */
switch(real_bpp)
  {
  case 1:
    ptr=*pal;
    ptr[0]=ptr[256]=ptr[512]=0;
    ptr[1]=ptr[257]=ptr[513]=255;
    break;
  
  case 4:
    /* 4-bit, palette is embedded in header */
    ptr=*pal;
    for(x=0;x<16;x++)
      {
      ptr[  0]=header.pal16[x*3  ];
      ptr[256]=header.pal16[x*3+1];
      ptr[512]=header.pal16[x*3+2];
      ptr++;
      }
    break;
  
  case 8:
    /* 8-bit */
    waste=fgetc(in);                    /* ditch splitter byte */
    /* palette already allocated */
    ptr=*pal;
    for(x=0;x<256;x++)
      {
      ptr[0]  =fgetc(in);
      ptr[256]=fgetc(in);
      ptr[512]=fgetc(in);
      ptr++;
      }
    break;
  
  case 24:
    /* no palette */
    break;
  }

pp->width=w;
pp->height=h;
pp->numcols=((real_bpp<8)?(1<<header.nplanes):256);

*output_type=bytepp;

if(dithering)
  ditherfinish();
  
fclose(in);
return(_PIC_OK);  
}


static void dispbyte(unsigned char *ptr,int *xp,int *yp,int c,int w,int h,
	int real_bpp,int byteline,int dithering,int *planep,int *pmaskp)
{
switch(real_bpp)
  {
  case 1: case 4:
    {
    /* mono or 4-bit */
    int f;
    unsigned char *dstptr;
    
    if((*yp)>=h) return;
    
    dstptr=ptr+(*yp)*w+*xp;
    w=byteline*8;
    
    for(f=0;f<8;f++)
      {
      *dstptr++|=(c&(0x80>>(f&7)))?(*pmaskp):0;
      (*xp)++;
      if(*xp>=w)
        {
        if(real_bpp==1) { (*xp)=0,(*yp)++; return; }
        /* otherwise, it's 4 bpp */
        (*xp)=0;
        (*planep)++; (*pmaskp)<<=1;
        if(*planep==4) { (*yp)++; return; }
        }
      if((*yp)>=h) return;
      }
    }
    break;
  
  case 8:
    *(ptr+(*yp)*w+*xp)=c;
    (*xp)++; if(*xp>=w) (*xp)=0,(*yp)++;
    break;
  
  case 24:
    if(dithering)
      *(ptr+(*yp)*w+(*xp)*3+(2-(*planep)))=c;	/* use 8-bit line ptr */
    else
      *(ptr+((*yp)*w+*xp)*3+(2-(*planep)))=c;
    (*xp)++;
    if(*xp>=w)
      {
      (*xp)=0;
      (*planep)++; /* no need to change pmask */
      if(*planep==3) { (*yp)++; return; }
      }
    break;
  }
}


void aborted_file_pcx_cleanup()
{
free(work_bmap);
free(work_pal);
fclose(work_in);
}
