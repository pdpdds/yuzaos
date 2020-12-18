/* Zgv v5.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1999 Russell Marks. See README for license details.
 *
 * readpcd.c - load PhotoCD image files.
 * Written by Matan Ziv-Av
 */

#ifdef PCD_SUPPORT

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcd.h>
#include "zgv.h"
#include "readpnm.h"
#include "rc_config.h"
#include "rcfile.h"

/* for aborted file stuff */
static unsigned char *work_bmap, *work_pal;
struct PCD_IMAGE *work_img;

int read_pcd_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
  unsigned char **pal,int *output_type,PICINFO *pp)
{
struct PCD_IMAGE img;
int i, x, y, w, h ;

*bmap=NULL;
*pal=NULL;

if(pcd_open(&img,filename)==-1)
  return(_PICERR_NOFILE);

x = y = w = h = 0 ;

if ((pcd_select(&img,cfg.pcdres,0,0,0,pcd_get_rot(&img,0),&x,&y,&w,&h)==-1)||
    (pcd_decode(&img)==-1))return(_PICERR_CORRUPT);

if((*output_type)!=1)*output_type=3;

if(WH_BAD(w,h) || (*bmap=malloc(w*(h+3-*output_type)*(*output_type)))==NULL)
  return(_PICERR_NOMEM);

if((*pal=malloc(768))==NULL)
  return(_PICERR_NOMEM);

if(*output_type==1){
   unsigned char *ptr = *pal;
   int r,g,b;
 
   for (r = 16; r < 256; r += 32)
     for (g = 16; g < 256; g += 32)	/* colours are 3:3:2 */
       for (b = 32; b < 256; b += 64)
	 {
	   *ptr = r;
	   ptr[256] = g;
           ptr[512] = b;
	   ptr++;
	 };
};

work_img=&img;
work_bmap=*bmap;
work_pal=*pal;

if(*output_type==1)ditherinit(w);
for(i=0;i<h;i++)
  {
     pcd_get_image_line(&img,i,(*bmap)+i*w*(*output_type),PCD_TYPE_BGR,0);
     if(*output_type==1)ditherline((*bmap)+i*w,i+1,w);
     if(howfarfunc!=NULL) howfarfunc(i+1,h);
  }
if(*output_type==1)ditherfinish();

pcd_close(&img);

pp->width=w;
pp->height=h;
pp->bpp=24;

return(_PIC_OK);  
}


void aborted_file_pcd_cleanup()
{
free(work_bmap);
free(work_pal); 
pcd_close(work_img);
}

#endif
