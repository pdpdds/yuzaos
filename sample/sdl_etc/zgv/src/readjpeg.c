/* zgv 5.2 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2000 Russell Marks. See README for license details.
 *
 * readjpeg.c - interface to the IJG's JPEG software, derived from
 *               their example.c.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>
//#include <sys/file.h>  /* for open et al */
#include "3deffects.h"
#include <jpeglib.h>
#include "zgv.h"
#include "readjpeg.h"
#include "vgadisp.h"   /* for pixelsize */
#include "rc_config.h"
#include "rcfile.h"



static hffunc howfar;

/* and we need to use all this stuff from vgadisp.c */
extern int width,height,numcols;
extern byte *theimage;
byte *pal;

/* our jpeg clean up routine (for zgv.c) needs this */
FILE *global_jpeg_infile;

/* The way that libjpeg v5 works means that this more-or-less *has* to
 * global if I want to allow aborting in mid-read. Oh well. :-(
 */
static struct jpeg_decompress_struct cinfo;

/* stuff for error routines */
struct my_error_mgr
  {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
  };

typedef struct my_error_mgr * my_error_ptr;

static int use_errmsg=0;


void jpegerr(char *msgtext)
{
strncpy(jpeg_png_errmsg,msgtext,JPEG_PNG_ERRMSG_SIZE-1);
jpeg_png_errmsg[JPEG_PNG_ERRMSG_SIZE-1]=0;
use_errmsg=1;
}


void my_error_exit(j_common_ptr cinfo)
{
my_error_ptr myerr=(my_error_ptr) cinfo->err;
char buf[JMSG_LENGTH_MAX];

(*cinfo->err->format_message)(cinfo,buf);

jpegerr(buf);	/* report the error message */

/* cleanup is done after jump back, so just do that now... */
longjmp(myerr->setjmp_buffer, 1);
}


/* No warning messages */
void my_output_message(j_common_ptr cinfo)
{
}


/* different error_exit... we call this (from zgv.c) if we aborted from
 * reading a JPEG. we use the setjmpbuf from zgv.c.
 */
void aborted_file_jpeg_cleanup()
{
jpeg_destroy_decompress(&cinfo);
free(pal);
fclose(global_jpeg_infile);
}



int read_JPEG_file(char *filename,hffunc howfarfunc,byte **palette,int quick,
	int *real_width,int *real_height)
{
static FILE *in;
struct my_error_mgr jerr;
int row_stride;		/* physical row width in output buffer */
int tmp,f;
unsigned char *ptr;

use_errmsg=0;
theimage=NULL;
howfar=howfarfunc;
if(real_width) *real_width=0;
if(real_height) *real_height=0;

if(((*palette)=(byte *)calloc(768,1))==NULL)
  return(_PICERR_NOMEM);
pal=*palette;

if((in=global_jpeg_infile=fopen(filename,"rb"))==NULL)
  return(_PICERR_NOFILE);

cinfo.err=jpeg_std_error(&jerr.pub);
jerr.pub.error_exit=my_error_exit;
jerr.pub.output_message=my_output_message;

if(setjmp(jerr.setjmp_buffer))
  {
  jpeg_destroy_decompress(&cinfo);
  fclose(global_jpeg_infile);	/* this uses the global FILE *. */
  free(pal);
  if(use_errmsg)
    return(_PICERR_SEE_ERRMSG);
  else
    return(_PICERR_CORRUPT);	/* could actually be anything... */
  }

/* Now we can initialize the JPEG decompression object. */
jpeg_create_decompress(&cinfo);

jpeg_stdio_src(&cinfo,in);	/* indicate source is the file */

jpeg_read_header(&cinfo,TRUE);

/* setup parameters for decompression */

switch(cfg.jpegspeed)
  {
  case 1:  cinfo.dct_method=JDCT_FLOAT; break;
  case 3:  cinfo.dct_method=JDCT_IFAST; break;
  default: cinfo.dct_method=JDCT_ISLOW;
  }

if(pixelsize==1)	/* if our preference is for 8-bit... */
  {
  cinfo.quantize_colors=TRUE;
  cinfo.desired_number_of_colors=256;
  cinfo.two_pass_quantize=TRUE;
  }


/* fix to greys if greyscale - this is required to read greyscale JPEGs */
if(cinfo.jpeg_color_space==JCS_GRAYSCALE)
  {
  cinfo.out_color_space=JCS_GRAYSCALE;
  cinfo.desired_number_of_colors=256;
  cinfo.quantize_colors=FALSE;
  cinfo.two_pass_quantize=FALSE;
  pixelsize=1;
  for(f=0;f<256;f++)
    pal[f]=pal[256+f]=pal[512+f]=f;
  }

width=cinfo.image_width;
height=cinfo.image_height;

if(real_width) *real_width=width;
if(real_height) *real_height=height;

if(quick)
  {
  int chkw=(1<<cfg.jpegindexstyle)*80,chkh=(1<<cfg.jpegindexstyle)*60;
  
  cinfo.scale_num=1;
  cinfo.scale_denom=1;
  
  while ((width>chkw || height>chkh) && cinfo.scale_denom<8)
    {
    width/=2;
    height/=2;
    cinfo.scale_denom*=2;
    }

  cinfo.dct_method=JDCT_FASTEST;
  cinfo.do_fancy_upsampling=FALSE;

  jpeg_calc_output_dimensions(&cinfo);

  width=cinfo.output_width;
  height=cinfo.output_height;
  }

if(WH_BAD(width,height) ||
   (theimage=(byte *)malloc(pixelsize*width*height))==NULL)
  {
  jpegerr("Out of memory");	/* XXX misleading if width/height are bad */
  longjmp(jerr.setjmp_buffer,1);
  }


jpeg_start_decompress(&cinfo);

/* read the palette (if greyscale, this has already been done) */
if(pixelsize==1 && cinfo.jpeg_color_space!=JCS_GRAYSCALE)
  for(f=0;f<cinfo.actual_number_of_colors;f++)
    {
    pal[    f]=cinfo.colormap[0][f];
    pal[256+f]=cinfo.colormap[1][f];
    pal[512+f]=cinfo.colormap[2][f];
    }

/* read the image */
ptr=theimage; row_stride=pixelsize*width;
if(pixelsize==1)
  while(cinfo.output_scanline<height)
    {
    jpeg_read_scanlines(&cinfo,&ptr,1);
    ptr+=row_stride;
    if(howfar!=NULL) howfar(cinfo.output_scanline,height);
    }
else
  while(cinfo.output_scanline<height)
    {
    jpeg_read_scanlines(&cinfo,&ptr,1);
    for(f=0;f<width;f++) { tmp=*ptr; *ptr=ptr[2]; ptr[2]=tmp; ptr+=3; }
    if(howfar!=NULL) howfar(cinfo.output_scanline,height);
    }

jpeg_finish_decompress(&cinfo);
jpeg_destroy_decompress(&cinfo);
fclose(in);

/* XXX: At this point you may want to check to see whether any corrupt-data
 * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
 */

return(_PIC_OK);
}
