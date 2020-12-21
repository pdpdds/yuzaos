/* zgv 5.9 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2005 Russell Marks. See README for license details.
 *
 * mousecur.c - mouse cursor routines.
 *
 * These all check has_mouse and return if zero, so there's no need
 * to check that before calling them.
 *
 * SDL mouse support by Dimitar Zhekov.
 */

#ifdef BACKEND_SDL

#include <wchar.h>
#include <SDL.h>
#include <stdlib.h>
#include "zgv.h"
#include <stdio.h>

static int usenarrow = 0;

#define MOUSECUR_XSIZE	16
#define MOUSECUR_YSIZE	16
#define MOUSECUR_PIXMAP_SIZE	(((MOUSECUR_XSIZE+7)/8)*MOUSECUR_YSIZE)

/* data for zgv's mouse cursor, see non-SDL version */
static Uint8 mouse_data[MOUSECUR_PIXMAP_SIZE] =
{
0x00, 0x00, 0x60, 0x00, 0x78, 0x00, 0x3e, 0x00,
0x3f, 0x80, 0x1f, 0xe0, 0x1f, 0xf8, 0x0f, 0x80,
0x0f, 0x80, 0x06, 0x40, 0x06, 0x20, 0x02, 0x10,
0x02, 0x08, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00
};

static Uint8 mouse_mask[MOUSECUR_PIXMAP_SIZE] =
{
0x60, 0x00, 0xf8, 0x00, 0xfe, 0x00, 0x7f, 0x80,
0x7f, 0xe0, 0x3f, 0xf8, 0x3f, 0xfc, 0x1f, 0xf8,
0x1f, 0xc0, 0x0f, 0xe0, 0x0f, 0x70, 0x07, 0x38,
0x07, 0x1c, 0x02, 0x0e, 0x00, 0x04, 0x00, 0x00
};

static Uint8 mouse_data_narrow[MOUSECUR_PIXMAP_SIZE / 2] =
{
0x00, 0x40, 0x60, 0x70, 0x78, 0x3c, 0x3e, 0x38,
0x38, 0x14, 0x14, 0x12, 0x12, 0x00, 0x00, 0x00
};

static Uint8 mouse_mask_narrow[MOUSECUR_PIXMAP_SIZE / 2] =
{
0x40, 0xe0, 0xf0, 0xf8, 0xfc, 0x7e, 0x7f, 0x7e,
0x7c, 0x3e, 0x3e, 0x3f, 0x3f, 0x12, 0x00, 0x00
};

static Uint8 mouse_data_hidden[1] = { 0 }, mouse_mask_hidden[1] = { 0 };

static SDL_Cursor* mouse_cursor = NULL, * mouse_cursor_narrow, * mouse_cursor_hidden;


void mousecur_init(int blackcol, int whitecol)
{
	SDL_Surface* surface = SDL_GetVideoSurface();

	if (!has_mouse || !surface) return;

	if (!mouse_cursor)
	{
		mouse_cursor = SDL_CreateCursor(mouse_data, mouse_mask, MOUSECUR_XSIZE,
			MOUSECUR_YSIZE, 0, 0);
		mouse_cursor_narrow = SDL_CreateCursor(mouse_data_narrow, mouse_mask_narrow,
			MOUSECUR_XSIZE / 2, MOUSECUR_YSIZE, 0, 0);
		mouse_cursor_hidden = SDL_CreateCursor(mouse_data_hidden, mouse_mask_hidden,
			1, 1, 0, 0);
		if (!mouse_cursor || !mouse_cursor_narrow || !mouse_cursor_hidden)
		{
			printf("zgv: SDL create cursor failed - %s\n"
				"     mouse support disabled...\n", SDL_GetError());
			has_mouse = 0;
			return;
		}
		SDL_ShowCursor(SDL_ENABLE);
	}

	usenarrow = ((surface->w == 320 && surface->h == 400) ||
		(surface->w == 360 && surface->h == 480));
}

void mousecur_on(void)
{
	if (has_mouse) SDL_SetCursor(usenarrow ? mouse_cursor_narrow : mouse_cursor);
}

void mousecur_off(void)
{
	if (has_mouse) SDL_SetCursor(mouse_cursor_hidden);
}


#else	/* !BACKEND_SDL */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zgv_io.h"
#include "rc_config.h"
#include "rcfile.h"
#include "zgv.h"

 /* Zgv has more problems with drawing a mouse cursor than most
  * svgalib programs would. For example, since we need to support 640x480
  * 16-colour mode, we can't use any vgagl stuff, or at least not
  * exclusively. And for 320x400/360x480 modes, we have a specially
  * squished pointer. :-)
  */

static int usegl = -1, usenarrow = -1;

/* for area-under-cursor save buffer.
 * width/height may be less than sizes below if near edge of screen.
 */
static int savex, savey, savew, saveh;

static int screen_width, screen_height, screen_bytepp;

#define MOUSECUR_XSIZE	16
#define MOUSECUR_YSIZE	16
#define MOUSECUR_PIXMAP_SIZE	(MOUSECUR_XSIZE*MOUSECUR_YSIZE)

/* next two pixmaps are *4 as must be able to handle up to 32-bpp modes */
static unsigned char mouseptr_save[MOUSECUR_PIXMAP_SIZE * 4];
static unsigned char mouseptr_pixmap[MOUSECUR_PIXMAP_SIZE * 4];
static unsigned char mouseptr_pixmap_orig[MOUSECUR_PIXMAP_SIZE] =
{
	/* 1 is black, 2 is white, 0 is transparent */
	0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
	2,1,1,2,2,0,0,0,0,0,0,0,0,0,0,0,
	2,1,1,1,1,2,2,0,0,0,0,0,0,0,0,0,
	0,2,1,1,1,1,1,2,2,0,0,0,0,0,0,0,
	0,2,1,1,1,1,1,1,1,2,2,0,0,0,0,0,
	0,0,2,1,1,1,1,1,1,1,1,2,2,0,0,0,
	0,0,2,1,1,1,1,1,1,1,1,1,1,2,0,0,
	0,0,0,2,1,1,1,1,1,2,2,2,2,0,0,0,
	0,0,0,2,1,1,1,1,1,2,0,0,0,0,0,0,
	0,0,0,0,2,1,1,2,2,1,2,0,0,0,0,0,
	0,0,0,0,2,1,1,2,0,2,1,2,0,0,0,0,
	0,0,0,0,0,2,1,2,0,0,2,1,2,0,0,0,
	0,0,0,0,0,2,1,2,0,0,0,2,1,2,0,0,
	0,0,0,0,0,0,2,0,0,0,0,0,2,1,2,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static unsigned char mouseptr_pixmap_narrow_orig[MOUSECUR_PIXMAP_SIZE / 2] =
{
	/* 1 is black, 2 is white, 0 is transparent */
	0,2,0,0,0,0,0,0,
	2,1,2,0,0,0,0,0,
	2,1,1,2,0,0,0,0,
	2,1,1,1,2,0,0,0,
	2,1,1,1,1,2,0,0,
	0,2,1,1,1,1,2,0,
	0,2,1,1,1,1,1,2,
	0,2,1,1,1,2,2,0,
	0,2,1,1,1,2,0,0,
	0,0,2,1,2,1,2,0,
	0,0,2,1,2,1,2,0,
	0,0,2,1,2,2,1,2,
	0,0,2,1,2,2,1,2,
	0,0,0,2,0,0,2,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};





void mousecur_init(int blackcol, int whitecol)
{
	int f, val;
	int mode = vga_getcurrentmode();

	if (!has_mouse) return;

	screen_bytepp = vga_getmodeinfo(mode)->bytesperpixel;

	screen_width = vga_getxdim();
	screen_height = vga_getydim();

	mouse_setxrange(0, screen_width - 1);
	mouse_setyrange(0, screen_height - 1);

	switch (mode)
	{
	case G640x480x16:
	case G320x200x256: case G320x240x256:
	case G320x400x256: case G360x480x256:
		usegl = 0; break;
	default:
		usegl = 1;
	}

	usenarrow = (mode == G320x400x256 || mode == G360x480x256);

	if (usegl)
		gl_setcontextvga(vga_getcurrentmode());

	for (f = 0; f < (usenarrow ? MOUSECUR_PIXMAP_SIZE / 2 : MOUSECUR_PIXMAP_SIZE); f++)
	{
		switch (usenarrow ? mouseptr_pixmap_narrow_orig[f] : mouseptr_pixmap_orig[f])
		{
		case 1: /* black */
			val = blackcol; break;
		case 2: /* white (ish :-)) */
			val = whitecol; break;
		default: /* assume zero - transparent */
			val = 0; break;
		}

		switch (screen_bytepp)
		{
		case 4:
			mouseptr_pixmap[f * 4] = (val & 255);
			mouseptr_pixmap[f * 4 + 1] = (val >> 8);
			mouseptr_pixmap[f * 4 + 2] = (val >> 16);
			mouseptr_pixmap[f * 4 + 3] = 0;
			break;
		case 3:
			mouseptr_pixmap[f * 3] = (val & 255);
			mouseptr_pixmap[f * 3 + 1] = (val >> 8);
			mouseptr_pixmap[f * 3 + 2] = (val >> 16);
			break;
		case 2:
			mouseptr_pixmap[f * 2] = (val & 255);
			mouseptr_pixmap[f * 2 + 1] = (val >> 8);
			break;
		default:
			mouseptr_pixmap[f] = val;
		}
	}
}


void mousecur_on()
{
	int small = 0, y;

	if (!has_mouse) return;

	savex = mouse_getx();
	savey = mouse_gety();
	savew = MOUSECUR_XSIZE;
	saveh = MOUSECUR_YSIZE;
	if (usenarrow) savew /= 2;
	/* deal with being near edge of screen */
	if (savex > screen_width - savew)
		savew = screen_width - savex, small = 1;
	if (savey > screen_height - saveh)
		saveh = screen_height - savey, small = 1;

	/* we need to save existing data, then draw the mouse. */
	if (usegl)
	{
		/* gl-using 8/15/16/24/32-bpp version */
		gl_getbox(savex, savey, savew, saveh, mouseptr_save);
		/* since there's no gl_putboxpartmask, and clipping doesn't work for
		 * gl_putboxmask, we have to do either masking or clipping ourselves.
		 * Hmm. I'll pick clipping, I think...
		 */
		if (small)
			/* do it a line at a time */
			for (y = 0; y < saveh; y++)
				gl_putboxmask(savex, savey + y, savew, 1,
					mouseptr_pixmap + MOUSECUR_XSIZE * y * screen_bytepp);
		else
			/* full size, no problem */
			gl_putboxmask(savex, savey, savew, saveh, mouseptr_pixmap);
	}
	else
	{
		static unsigned char scantmp[80];
		unsigned char* saveptr, * inptr, * outptr;
		int x;

		/* 16-col and low-res-8-bit version */

		/* save area */
		saveptr = mouseptr_save;
		for (y = 0; y < saveh; y++, saveptr += savew)
			vga_getscansegment(saveptr, savex, savey + y, savew);

		saveptr = mouseptr_save;
		for (y = 0; y < saveh; y++)
		{
			inptr = mouseptr_pixmap + y * (usenarrow ? MOUSECUR_XSIZE / 2 : MOUSECUR_XSIZE);
			outptr = scantmp;
			for (x = 0; x < savew; x++, inptr++, saveptr++)
				*outptr++ = ((*inptr) ? (*inptr) : (*saveptr));
			vga_drawscansegment(scantmp, savex, savey + y, savew);
		}
	}
}


void mousecur_off()
{
	if (!has_mouse) return;

	/* redraw saved data */
	if (usegl)
		gl_putbox(savex, savey, savew, saveh, mouseptr_save);
	else
	{
		unsigned char* saveptr = mouseptr_save;
		int y;

		for (y = 0; y < saveh; y++, saveptr += savew)
			vga_drawscansegment(saveptr, savex, savey + y, savew);
	}
}


#endif	/* !BACKEND_SDL */
