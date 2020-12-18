/* Zgv v2.7 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1995 Russell Marks. See README for license details.
 *
 * resizepic.c - the resizepic() routine.
 */


#include <stdio.h>
#include <stdlib.h>
#include "resizepic.h"


/* resamples from 'theimage' to a new malloc'ed area, a pointer to which is
 * the return value. 
 * width x height	- size of source image
 * sw_ask x sh_ask	- requested size of dest. image
 * input image must be 8-bit, output image is 24-bit
 * new width x height in put back in sw_ask and sh_ask
 */
unsigned char *resizepic(unsigned char *theimage,
  unsigned char *palr,unsigned char *palg,unsigned char *palb,
  int width,int height,int *sw_ask,int *sh_ask)
{
int a,b,x,yp,yw;
long y,sw,sh,lastyp;
int c,pixwide,pixhigh;
int scrnwide,scrnhigh;
int bigimage;
unsigned char *rline;
int tmp2,tr,tg,tb,tn;
int xypos;


scrnwide=*sw_ask;
scrnhigh=*sh_ask;

if((rline=malloc(scrnwide*scrnhigh*3))==NULL) return(0);
for(x=0;x<(scrnwide*scrnhigh*3);x++)
  rline[x]=0;

/* try landscapey */
sw=scrnwide; sh=(int)((scrnwide*((long)height))/((long)width));
if(sh>scrnhigh)
  /* no, oh well portraity then */
  { sh=scrnhigh; sw=(int)((scrnhigh*((long)width))/((long)height)); }

/* fix things for very thin images */
if(sh==0) sh++;
if(sw==0) sw++;

*sw_ask=sw; *sh_ask=sh;

/* so now our zoomed image will be sw x sh */
bigimage=(width>sw)?1:0;   /* 1 if image has been reduced, 0 if made bigger */
if(bigimage)
  /* it's been reduced - easy, just make 'em fit in less space */
  {
  lastyp=-1;
  pixhigh=(int)(((float)height)/((float)sh)+0.5);
  pixwide=(int)(((float)width)/((float)sw)+0.5);
  pixhigh++;
  pixwide++;
  for(y=0;y<height;y++)
    {
    yp=(y*sh)/height;
    if(yp!=lastyp)
      {
      yw=y*width;
      /* we try to resample it a bit */
      for(x=0;x<width;x++,yw++)
        {
        tr=tg=tb=tn=0;
        for(b=0;(b<pixhigh)&&(y+b<height);b++)
          for(a=0;(a<pixwide)&&(x+a<width);a++)
            {
            tmp2=*(theimage+yw+a+b*width);
            tr+=palr[tmp2];
            tg+=palg[tmp2];
            tb+=palb[tmp2];
            tn++;
            }
        tr/=tn; tg/=tn; tb/=tn;
        xypos=3*(((x*sw)/width)+yp*scrnwide);
        rline[xypos]=tr;
        rline[xypos+1]=tg;
        rline[xypos+2]=tb;
        }
      lastyp=yp;
      }
    }
  }
else
  /* we just leave it the same size */
  {
  *sw_ask=width; *sh_ask=height;
  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
      {
      c=theimage[y*width+x];
      rline[3*(y*scrnwide+x)  ]=palr[c];
      rline[3*(y*scrnwide+x)+1]=palg[c];
      rline[3*(y*scrnwide+x)+2]=palb[c];
      }
  }
  
return(rline);
}
