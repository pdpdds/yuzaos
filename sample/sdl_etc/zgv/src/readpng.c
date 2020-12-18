/* Zgv v3.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * readpng.c - interface to pnglib, derived from their example.c
 *              and readjpeg.c.
 */

#include <stdio.h>
#include <string.h>
 //#include <unistd.h>
#include <stdlib.h>
#include <png.h>
#include <setjmp.h>	/* after png.h to avoid horrible thing in pngconf.h */
#include "zgv.h"
#include "3deffects.h"
#include "readpng.h"
#include "readpnm.h"
#include "vgadisp.h"   /* for pixelsize */
#include "rc_config.h"
#include "rcfile.h"


/* prototypes */
void aborted_file_png_cleanup(void);
int read_png_file(char* filename, hffunc howfarfunc, byte** palette);
void unpack_bits(int bit_depth, unsigned char* rowptr, int width, int height);


hffunc howfar;

/* we need to use all this stuff from vgadisp.c */
extern int width, height, numcols;
extern byte* theimage;
static byte* pal;

/* our png clean up routine (for zgv.c) needs this */
FILE* global_png_infile;

/* must be global to allow aborting in mid-read */
static png_structp png_ptr;
static png_infop info_ptr;
static int number_passes, ilheight, dithering;

static int use_errmsg = 0;



/* we call this (from zgv.c) if we aborted.
 * we use the setjmpbuf from zgv.c.
 */
void aborted_file_png_cleanup()
{
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	free(pal);
	fclose(global_png_infile);
}


void my_png_error(png_structp png_ptr, const char* message)
{
	strncpy(jpeg_png_errmsg, message, JPEG_PNG_ERRMSG_SIZE - 1);
	jpeg_png_errmsg[JPEG_PNG_ERRMSG_SIZE - 1] = 0;
	use_errmsg = 1;

	//20190102
	for (;;);
	/* cleanup is done after jump back, so just do that now... */
	//longjmp(png_ptr->jmpbuf, 1);
}


/* No warning messages */
void my_png_warning(png_structp png_ptr, const char* message)
{
}


static void dither_png(hffunc howfarfunc, int number_passes, int ilheight)
{
	unsigned char* ptr;
	int y;

	/* ok, we have 3*width*height allocated (actually 3*width*(height+2),
	 * but let's not be pedantic :-)). This currently contains the
	 * 24-bit image. If we dither this into the same space, it works
	 * ok and doesn't overwrite anything as it goes. We then, for each
	 * line, have to copy it into the right place in the new 8-bit image.
	 * plus we should call the howfar func. as dithering counts as the
	 * 2nd 50% of loading.
	 */
	for (y = 0, ptr = theimage; y < height; y++, ptr += width * 3)
	{
		ditherline(ptr, y, width);
		if (y > 0) memcpy(theimage + y * width, ptr, width);
		if (howfarfunc != NULL) howfarfunc(ilheight + y * number_passes, ilheight * 2);
	}

	ditherfinish();
	pixelsize = 1;
}


int read_png_file(char* filename, hffunc howfarfunc, byte** palette)
{
	static FILE* in;
	unsigned char* rowptr;
	png_uint_32 uw, uh;
	int f, y;
	int bitdepth, colourtype, interlacetype;
	png_color_16 my_background = { 0,0,0,0,0 };

	use_errmsg = 0;
	theimage = NULL;

	if (((*palette) = (byte*)calloc(768, 1)) == NULL)
		return(_PICERR_NOMEM);
	pal = *palette;

	if ((in = global_png_infile = fopen(filename, "rb")) == NULL)
		return(_PICERR_NOFILE);

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
		my_png_error, my_png_warning);
	if (png_ptr == NULL)
	{
		free(pal); fclose(in);
		return(_PICERR_NOMEM);
	}

	if ((info_ptr = png_create_info_struct(png_ptr)) == NULL)
	{
		free(pal); fclose(in);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return(_PICERR_NOMEM);
	}
	//20200102
	/*if (setjmp(png_ptr->jmpbuf))
	{
		// if we get here, there was an error.
		// don't use local variables here, they may have been blasted 
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(global_png_infile);	// this uses the global FILE *. 
		if (!cfg.errignore) free(pal);
		if (cfg.errignore && theimage != NULL && dithering)
			dither_png(NULL, number_passes, ilheight);
		if (use_errmsg)
			return(_PICERR_SEE_ERRMSG);
		else
			return(_PICERR_CORRUPT);	// could actually be anything...
	}*/
	
	png_init_io(png_ptr, in);
	png_read_info(png_ptr, info_ptr);

	/* the uw/uh stuff is to avoid a `pointer mismatch' compiler warning */
	png_get_IHDR(png_ptr, info_ptr, &uw, &uh, &bitdepth,
		&colourtype, &interlacetype, (int*)NULL, (int*)NULL);
	width = uw; height = uh;

	/* now lots and lots of config stuff... */

	if (bitdepth == 16)
		png_set_strip_16(png_ptr);

	png_set_packing(png_ptr);

	if (bitdepth != 1)		/* changing background when 1-bit may cause problems */
		png_set_background(png_ptr, &my_background, 1.0, 0, 1.0);

	/* doing this here will result in it allocating enough
	 * for the 24-bit image even if it will be dithered. This is *required*
	 * given the way we have to dither, in case a file is interlaced.
	 */
	if ((colourtype == PNG_COLOR_TYPE_RGB || colourtype == PNG_COLOR_TYPE_RGB_ALPHA) &&
		(pixelsize == 1 || cfg.jpeg24bit == 0))		/* dither? */
	{
		pixelsize = 3;
		if (ditherinit(width) == 0)
			return(_PICERR_NOMEM);
		make_332_palette(pal);
		dithering = 1;
	}
	else
		dithering = 0;

	if (colourtype == PNG_COLOR_TYPE_GRAY && bitdepth < 8)
		png_set_expand(png_ptr);

	if (colourtype == PNG_COLOR_TYPE_GRAY ||
		colourtype == PNG_COLOR_TYPE_GRAY_ALPHA ||
		colourtype == PNG_COLOR_TYPE_PALETTE)
		pixelsize = 1;

	if (colourtype == PNG_COLOR_TYPE_GRAY ||
		colourtype == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		pixelsize = 1;
		for (f = 0; f < 256; f++)
			pal[f] = pal[256 + f] = pal[512 + f] = f;
	}

	/* XXX should do gamma stuff */

	/* output BGR if applicable, not RGB */
	png_set_bgr(png_ptr);

	number_passes = png_set_interlace_handling(png_ptr);

	/* fix palette (probably not needed now, but will be if I do gamma later) */
	png_read_update_info(png_ptr, info_ptr);

	if (colourtype == PNG_COLOR_TYPE_PALETTE)
	{
		png_colorp cols;
		int palsiz;

		png_get_PLTE(png_ptr, info_ptr, &cols, &palsiz);
		for (f = 0; f < palsiz; f++)
		{
			pal[f] = cols[f].red;
			pal[256 + f] = cols[f].green;
			pal[512 + f] = cols[f].blue;
		}
	}


	/* allocate image memory (with two extra lines for dithering) */
	if (WH_BAD(width, height) ||
		(theimage = (byte*)malloc(pixelsize * width * (height + 2))) == NULL)
		return(_PICERR_NOMEM);


	ilheight = height * number_passes;

	if (cfg.errignore)
		memset(theimage, 0, width * (height + 1) * pixelsize);

	/* read the image */
	for (y = 0; y < ilheight; y++)
	{
		rowptr = theimage + (y % height) * width * pixelsize;
		if (cfg.errignore && number_passes > 1)	/* fully draw interlace bits */
			png_read_rows(png_ptr, NULL, &rowptr, 1);
		else
			png_read_rows(png_ptr, &rowptr, NULL, 1);

		if (howfarfunc != NULL) howfarfunc(y, dithering ? ilheight * 2 : ilheight);
	}

	/* dither if needed */
	if (dithering) dither_png(howfarfunc, number_passes, ilheight);

	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(in);

	return(_PIC_OK);
}
