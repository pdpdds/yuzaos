/* Zgv v3.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * readxbm.c - read an X bitmap.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zgv.h"
#include "readxbm.h"


/* for aborted_file_xbm_cleanup() */
static unsigned char *work_bmap,*work_pal;
static FILE *work_in;


int read_xbm_file(char *filename,hffunc howfarfunc,unsigned char **bmap,
                  unsigned char **pal,int *output_type,PICINFO *pp)
{
static unsigned char buf[1024];
FILE *in;
int w8,w,h,x,y,data;
unsigned char *image,*ptr;
int tmppos;
int i,mask;

*bmap=NULL;
*pal=NULL;

if((in=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

/* the format as defined in "Xlib - C Language X Interface"
 * (in subsection 16.9, "Manipulating Bitmaps") is
 * relatively strict, and this code follows it pretty
 * much to the letter.
 */

/* first two lines must be:
 * #define foo_width <width>
 * #define foo_height <height>
 *
 * Inevitably, some people don't seem able to read the spec, so I've
 * seen one or two files which don't work (they have comments in - but
 * the spec doesn't provide for comments). :-(
 */

if(fgets(buf,sizeof(buf),in)==NULL || strncmp(buf,"#define ",8)!=0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

/* read number after "_width" */
if((ptr=strstr(buf,"_width"))==NULL)
  CLOSE_AND_RET(_PICERR_CORRUPT);
w=atoi(ptr+6);

/* repeat for height */
if(fgets(buf,sizeof(buf),in)==NULL || strncmp(buf,"#define ",8)!=0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

if((ptr=strstr(buf,"_height"))==NULL)
  CLOSE_AND_RET(_PICERR_CORRUPT);
h=atoi(ptr+7);

if(w==0 || h==0)
  CLOSE_AND_RET(_PICERR_CORRUPT);

/* ignore possible hotspot lines
 * (the easiest way is to ignore until we get a line which doesn't
 * start with "#define")
 */
tmppos=ftell(in);
while(fgets(buf,sizeof(buf),in)!=NULL)
  {
  if(strncmp(buf,"#define ",8)!=0) break;
  tmppos=ftell(in);
  }

/* check for "_bits[" and `{' */
if(strstr(buf,"_bits[")==NULL || (ptr=strrchr(buf,'{'))==NULL)
  CLOSE_AND_RET(_PICERR_CORRUPT);


/* seek to just after the brace
 * (bit nasty but it seems like the easiest way to deal with
 * the possibility of hex on the line with the opening brace on)
 */
fseek(in,tmppos+(ptr+1-buf),SEEK_SET);


if((*pal=calloc(768,1))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

(*pal)[0]=(*pal)[256]=(*pal)[512]=0;
(*pal)[1]=(*pal)[257]=(*pal)[513]=255;

w8=(w+7)/8;

if(WH_BAD(w,h) || (*bmap=image=malloc(w*h))==NULL)
  CLOSE_AND_RET(_PICERR_NOMEM);

/* save stuff in case of abort */
work_in=in; work_bmap=(*bmap); work_pal=*pal;


for(y=0;y<h;y++)
  {
  for(x=0;x<w8;x++)
    {
    if(fscanf(in,"%x",&data)!=1)
      {
      y=h;
      break;
      }
    
    fgetc(in);		/* remove comma (or LF or whatever) */
    
    for(i=0,mask=1;i<8;i++,mask<<=1)
      if(x*8+i<w)
        image[y*w+x*8+i]=((data&mask)?0:1);
    }
  
  if(howfarfunc!=NULL) howfarfunc(y,h);
  }

fclose(in);

pp->width=w;
pp->height=h;
pp->numcols=2;

*output_type=1;

return(_PIC_OK);  
}


void aborted_file_xbm_cleanup()
{
free(work_bmap);
free(work_pal);
fclose(work_in);
}
