/* zgv 5.9 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2005 Russell Marks. See README for license details.
 *
 * zgv_io.c - functional replacements for svgalib routines when not using
 *		svgalib itself as the backend.
 *
 * SDL mouse support by Dimitar Zhekov.
 */


#include "rc_config.h"
#include "rcfile.h"
#include "zgv.h"
#include "zgv_io.h"


 /* required by all */
volatile int zgv_io_timer_flag = 0;
static volatile int timer_in_use = 0;


#ifndef BACKEND_SVGALIB

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include "readnbkey.h"


/* zgv's main requirement modes-wise is that you have at least one
 * 640x480, 800x600, 1024x768, or 1280x1024 256-colour mode. Without
 * that you can't run the file selector. The viewer will happily pick
 * up on any mode the file selector can use, though of course 256-colour
 * wouldn't be its first choice. :-) But the viewer doesn't actually
 * impose any additional mode requirements, at least.
 *
 * Pixel formats used for zgl_*box* and zvga_drawscan*:
 *
 * 4-bit	like 8-bit, but only uses values 0..15
 * 8-bit	0..255 indicies into palette, one per byte
 * 15-bit	0rrrrrgggggbbbbb, little-endian, blue at low end.
 *		Don't bother with 15-bit unless you have to.
 * 16-bit	rrrrrggggggbbbbb, little-endian, blue at low end.
 * 24-bit	3 bytes per pixel, in BGR order.
 * 32-bit	4 bytes per pixel, in BGR0 order.
 *
 * zgv is reasonably smart about using 15/16 and 24/32-bit modes as
 * appropriate, so only supporting those which are most convenient for
 * you (e.g. 16, 32) is acceptable. A sensible set of depths to
 * support is probably 8, 16, and 32.
 *
 * Colours for zvga_*pal* are in the range 0..63 for each of R, G, B.
 *
 * If you're emulating palette-based modes in direct-colour modes,
 * don't do a redraw when the palette changes - the program itself
 * forces a redraw after palette changes when using non-svgalib
 * backends.
 */


 /* possible modes. All start off with has_mode zero, so make that
  * non-zero for any you have in zvga_init() (unless you do zvga_hasmode()
  * some other way).
  */
static zvga_modeinfo modes[GLASTMODE + 1] =
{
	/* must be in same order as in zgv_io.h */
	{ 0,0,0,0,0,0 },
	{ 640,480,4,1,	16,0 },
	{ 320,200,8,1,	256,0 },
	{ 320,240,8,1,	256,0 },
	{ 320,400,8,1,	256,0 },
	{ 360,480,8,1,	256,0 },
	{ 640,480,8,1,	256,0 },
	{ 800,600,8,1,	256,0 },
	{ 1024,768,8,1,	256,0 },
	{ 1280,1024,8,1,	256,0 },
	{ 1152,864,8,1,	256,0 },
	{ 1600,1200,8,1,	256,0 },
	{ 320,200,15,2,	32768,0 },
	{ 320,240,15,2,	32768,0 },
	{ 640,480,15,2,	32768,0 },
	{ 800,600,15,2,	32768,0 },
	{ 1024,768,15,2,	32768,0 },
	{ 1280,1024,15,2,	32768,0 },
	{ 1152,864,15,2,	32768,0 },
	{ 1600,1200,15,2,	32768,0 },
	{ 320,200,16,2,	65536,0 },
	{ 320,240,16,2,	65536,0 },
	{ 640,480,16,2,	65536,0 },
	{ 800,600,16,2,	65536,0 },
	{ 1024,768,16,2,	65536,0 },
	{ 1280,1024,16,2,	65536,0 },
	{ 1152,864,16,2,	65536,0 },
	{ 1600,1200,16,2,	65536,0 },
	{ 320,200,32,4,	16777216,0 },
	{ 320,240,32,4,	16777216,0 },
	{ 640,480,32,4,	16777216,0 },
	{ 800,600,32,4,	16777216,0 },
	{ 1024,768,32,4,	16777216,0 },
	{ 1280,1024,32,4,	16777216,0 },
	{ 1152,864,32,4,	16777216,0 },
	{ 1600,1200,32,4,	16777216,0 },
	{ 320,200,24,3,	16777216,0 },
	{ 320,240,24,3,	16777216,0 },
	{ 640,480,24,3,	16777216,0 },
	{ 800,600,24,3,	16777216,0 },
	{ 1024,768,24,3,	16777216,0 },
	{ 1280,1024,24,3,	16777216,0 },
	{ 1152,864,24,3,	16777216,0 },
	{ 1600,1200,24,3,	16777216,0 }
};

static int current_mode = 0;


/* mouse stuff can be ignored */
void zmouse_close(void) {}
void zmouse_setscale(int s) {}
void zmouse_setxrange(int x1, int x2) {}
void zmouse_setyrange(int y1, int y2) {}

/* only used by mouse code, so... */
void zgl_putboxmask(int x, int y, int w, int h, void* dp) {}

/* these can also be ignored */
void zvga_disabledriverreport(void) {}
void zvga_lockvc(void) {}
void zvga_unlockvc(void) {}
int zvga_oktowrite(void) { return 1; }
void zvga_runinbackground(int stat) {}
int zgl_setcontextvga(int m) { return 0; }
/* only used by vgadisp.c, which avoids it for non-svgalib backends */
void zgl_putboxpart(int x, int y, int w, int h, int bw, int bh, void* b,
	int xo, int yo) {}

/* simple generic stuff */
int zvga_lastmodenumber(void) { return GLASTMODE; }
int zvga_getcolors(void) { return modes[current_mode].colors; }
int zvga_getcurrentmode(void) { return current_mode; }
int zvga_getxdim(void) { return modes[current_mode].width; }
int zvga_getydim(void) { return modes[current_mode].height; }


#ifdef BACKEND_SDL

#include <SDL.h>
#include "mousecur.h"

static int zmouse_state = 0, zmouse_xpos = 0, zmouse_ypos = 0;

int zmouse_getbutton(void) { return zmouse_state; }
int zmouse_getx(void) { return zmouse_xpos; }
int zmouse_gety(void) { return zmouse_ypos; }
void zmouse_setposition(int x, int y) { }//if (current_mode) SDL_WarpMouse(x, y); }
int zvga_getmousetype(void) { return cfg.svgalib_mouse; }

int zmouse_init_return_fd(char* dev, int type, int samplerate)
{
	return cfg.svgalib_mouse ? (cfg.mousekludge = 0) : -1;
}

int zmouse_update(void) { return SDL_GetMouseState(&zmouse_xpos, &zmouse_ypos); }

static int caller_pal[768];
static SDL_Color sdl_pal[256];
static int palchanged = 0;
static int scrnchange_line_min = (1 << 30), scrnchange_line_max = -1;

SDL_Surface* surface = NULL;

#define SCRNMODE_FLAGS	(cfg.fullscreen?SDL_FULLSCREEN:0)


static void pal_update(void)
{
	if (palchanged)
	{
		palchanged = 0;
		//SDL_SetColors(surface, sdl_pal, 0, 256);
		SDL_SetPaletteColors(surface->format->palette, sdl_pal, 0, 256);
	}
}


static Uint32 native_col_to_sdl(int c, int gl_func)
{
	switch (modes[current_mode].colors)
	{
	case 16: case 256:	/* 16 should be impossible though */
		return(c);

	case 32768:
		if (gl_func)
			return(SDL_MapRGB(surface->format,
			((c >> 10) & 31) * 255 / 31, ((c >> 5) & 31) * 255 / 31, (c & 31) * 255 / 31));
		/* else, falls through */

	case 65536:
		if (gl_func)
			return(SDL_MapRGB(surface->format,
			((c >> 11) & 31) * 255 / 31, ((c >> 5) & 63) * 255 / 63, (c & 31) * 255 / 31));
		/* else, falls through */

	default:	/* 24/32-bit */
		return(SDL_MapRGB(surface->format, (c >> 16) & 255, (c >> 8) & 255, c & 255));
	}
}


void zgl_fillbox(int x, int y, int w, int h, int c)
{
	SDL_Rect rect = { x,y,w,h };

	pal_update();

	SDL_FillRect(surface, &rect, native_col_to_sdl(c, 1));

	if (y < scrnchange_line_min) scrnchange_line_min = y;
	if (y + h - 1 > scrnchange_line_max) scrnchange_line_max = y + h - 1;
}


void zgl_getbox(int x, int y, int w, int h, void* dp)
{
	int xx, yy;
	int wm = w * modes[current_mode].bytesperpixel;
	int xm = x * modes[current_mode].bytesperpixel;
	unsigned char* src, * dst;

	pal_update();

	if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) == -1)
		return;

	dst = dp;
	for (yy = y; yy < y + h; yy++, (DWORD)dp += wm)
	{
		/* memcpy is a library call, and thus disallowed :-( */
		src = (char*)surface->pixels + yy * surface->pitch + xm;
		for (xx = 0; xx < wm; xx++)
			*dst++ = *src++;
	}

	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);
}


void zgl_putbox(int x, int y, int w, int h, void* dp)
{
	int xx, yy;
	int wm = w * modes[current_mode].bytesperpixel;
	int xm = x * modes[current_mode].bytesperpixel;
	unsigned char* src, * dst;

	pal_update();

	if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) == -1)
		return;

	src = dp;
	for (yy = y; yy < y + h; yy++, (DWORD)dp += wm)
	{
		/* memcpy is a library call, and thus disallowed :-( */
		dst = (char*)surface->pixels + yy * surface->pitch + xm;
		for (xx = 0; xx < wm; xx++)
			*dst++ = *src++;
	}

	if (y < scrnchange_line_min) scrnchange_line_min = y;
	if (y + h - 1 > scrnchange_line_max) scrnchange_line_max = y + h - 1;

	if (SDL_MUSTLOCK(surface))
		SDL_UnlockSurface(surface);
}


int zvga_clear(void)
{
	pal_update();

	SDL_FillRect(surface, NULL, native_col_to_sdl(0, 0));

	scrnchange_line_min = 0;
	scrnchange_line_max = modes[current_mode].height - 1;

	return 0;
}


/* zvga_drawline is a slightly hacked version of:
 *
 * digline: draw digital line from (x1,y1) to (x2,y2),
 * calling a user-supplied procedure at each pixel.
 * Does no clipping.  Uses Bresenham's algorithm.
 *
 * Paul Heckbert	3 Sep 85
 *
 * ...which is from "Graphics Gems", Academic Press, 1990.
 */

 /* absolute value of a */
#define ABS(a)		(((a)<0) ? -(a) : (a))

/* take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)		(((a)<0) ? -1 : 1)

int zvga_drawline(int x1, int y1, int x2, int y2)
{
	/* I've left the indentation as it was, mainly 'cos I can't be bothered
	 * to fix it :-)
	 */
	Uint32 col = native_col_to_sdl(current_colour, 0);
	SDL_Rect rect = { 0,0,1,1 };
	int d, x, y, ax, ay, sx, sy, dx, dy;

	pal_update();
	dx = x2 - x1;  ax = ABS(dx) << 1;  sx = SGN(dx);
	dy = y2 - y1;  ay = ABS(dy) << 1;  sy = SGN(dy);

	if (y1 < y2)
	{
		if (y1 < scrnchange_line_min) scrnchange_line_min = y1;
		if (y2 > scrnchange_line_max) scrnchange_line_max = y2;
	}
	else
	{
		if (y2 < scrnchange_line_min) scrnchange_line_min = y2;
		if (y1 > scrnchange_line_max) scrnchange_line_max = y1;
	}

	x = x1;
	y = y1;
	if (ax > ay) {		/* x dominant */
		d = ay - (ax >> 1);
		for (;;) {
			rect.x = x; rect.y = y;
			SDL_FillRect(surface, &rect, col);
			if (x == x2) return 0;
			if (d >= 0) {
				y += sy;
				d -= ax;
			}
			x += sx;
			d += ay;
		}
	}
	else {			/* y dominant */
		d = ax - (ay >> 1);
		for (;;) {
			rect.x = x; rect.y = y;
			SDL_FillRect(surface, &rect, col);
			if (y == y2) return 0;
			if (d >= 0) {
				x += sx;
				d -= ay;
			}
			y += sy;
			d += ax;
		}
	}

	return 0;
}


int zvga_drawpixel(int x, int y)
{
	SDL_Rect rect = { x,y,1,1 };

	SDL_FillRect(surface, &rect, native_col_to_sdl(current_colour, 0));

	if (y < scrnchange_line_min) scrnchange_line_min = y;
	if (y > scrnchange_line_max) scrnchange_line_max = y;

	return 0;
}


int zvga_drawscanline(int line, unsigned char* cols)
{
	pal_update();

	zgl_putbox(0, line, modes[current_mode].width, 1, cols);

	if (line < scrnchange_line_min) scrnchange_line_min = line;
	if (line > scrnchange_line_max) scrnchange_line_max = line;

	return 0;
}


int zvga_drawscansegment(unsigned char* cols, int x, int y, int len)
{
	zgl_putbox(x, y, len, 1, cols);

	if (y < scrnchange_line_min) scrnchange_line_min = y;
	if (y > scrnchange_line_max) scrnchange_line_max = y;

	return 0;
}


zvga_modeinfo* zvga_getmodeinfo(int mode)
{
	if (mode<0 || mode>GLASTMODE) return NULL;

	return(modes + mode);
}


int zvga_getpalvec(int start, int num, int* pal)
{
	int f;

	for (f = start; f < start + 3 * num; f++)
		*pal++ = caller_pal[f];

	return num;
}


/* this can be ignored if not supporting any of 320x400x8, 360x480x8,
 * and 640x480x4.
 */
int zvga_getscansegment(unsigned char* cols, int x, int y, int len)
{
	return 0;
}


int zvga_hasmode(int mode)
{
	if (mode<0 || mode>GLASTMODE) return 0;

	return(modes[mode].has_mode);
}


void sdl_exit(void)
{
	SDL_ShowCursor(SDL_ENABLE);
	SDL_Quit();
}


int zvga_init(void)
{
	int ret;
	int f;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		printf("zgv: SDL init failed - %s\n", SDL_GetError());
		exit(1);
	}

	/* in case it was installed setuid root */
	//setgid(getgid());
	//setuid(getuid());

	//atexit(sdl_exit);

	//SDL_WM_SetCaption("zgv-sdl", "zgv-sdl");

	/*for (f = 1; f <= GLASTMODE; f++)
	{
		ret = SDL_VideoModeOK(modes[f].width, modes[f].height,
			modes[f].bitsperpixel,
			SCRNMODE_FLAGS |
			(modes[f].colors <= 256 ? SDL_HWPALETTE : 0));
		if (modes[f].bitsperpixel <= 8)
			modes[f].has_mode = ret;
		else
			modes[f].has_mode = (modes[f].bitsperpixel == ret);
	}*/

	//SDL_EnableKeyRepeat(250, 50);
	//SDL_EnableUNICODE(1);
	SDL_ShowCursor(SDL_DISABLE);

	return 0;
}



int zvga_setcolor(int col)
{
	current_colour = col;
	return 0;
}

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;

int zvga_setmode(int mode)
{
	if (mode == TEXT)
		return -1;

	/*if (!zvga_hasmode(mode))
	{
		printf("zgv: error: bad vga_setmode(%d) call, this is probably a bug.\n", mode);
		exit(1);
	}
	else*/
	{
		int width = 640;
		int height = 480;
		window = SDL_CreateWindow("ImGui SDL2+SW rasterizer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE);
		renderer = SDL_CreateRenderer(window, -1, 0);
		surface = SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

		zvga_clear();
		return 0;
		/*SDL_Surface* ret;

		if (cfg.svgalib_mouse)
			save_mouse_pos();
		if ((ret = SDL_SetVideoMode(modes[mode].width, modes[mode].height,
			modes[mode].bitsperpixel,
			SCRNMODE_FLAGS |
			(modes[mode].colors <= 256 ? SDL_HWPALETTE : 0))));
		{
			current_mode = mode;
			if (modes[mode].colors <= 256)
				zvga_setpalette(0, 0, 0, 0);
			surface = ret;
			if (cfg.svgalib_mouse)
				restore_mouse_pos();

			zvga_clear();
			return 0;
		}*/


	}

	return -1;
}


int zvga_setpalette(int idx, int r, int g, int b)
{
	caller_pal[idx * 3] = r;
	caller_pal[idx * 3 + 1] = g;
	caller_pal[idx * 3 + 2] = b;
	sdl_pal[idx].r = (r * 255) / 63;
	sdl_pal[idx].g = (g * 255) / 63;
	sdl_pal[idx].b = (b * 255) / 63;
	palchanged = 1;

	return 0;
}


int zvga_setpalvec(int start, int num, int* pal)
{
	int f;

	for (f = start; f < start + num; f++)
		zvga_setpalette(f, pal[f * 3], pal[f * 3 + 1], pal[f * 3 + 2]);

	pal_update();

	return num;
}


int zvga_setrgbcolor(int r, int g, int b)
{
	current_colour = ((r << 16) | (g << 8) | b);
	return 0;
}


static int forceesc = 0;


void zgv_io_screen_update(void)
{
	if (forceesc) return;

	if (scrnchange_line_min <= scrnchange_line_max)
	{
		//20200102
		SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		//SDL_UpdateRect(surface, 0, scrnchange_line_min, modes[current_mode].width,
			//scrnchange_line_max - scrnchange_line_min + 1);
	}

	scrnchange_line_min = (1 << 30);
	scrnchange_line_max = -1;
}


static int mouseevent = 0;


int zgv_io_readnbkey(void)
{
	SDL_Event event;
	int key;

	/* if we received SDL_QUIT, return Escs (which exit *everything* in zgv)
	 * continuously, and avoid any screen update. This is the fastest way to
	 * do a clean exit, as ugly as it is.
	 */
	if (forceesc)
		return(27);

	zgv_io_screen_update();

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_USEREVENT:
			/* timer giving us a poke :-) */
			zgv_io_timer_flag = 1;
			break;

		case SDL_KEYDOWN:
			/* check for various special/awkward keys */
			switch (event.key.keysym.sym)
			{
			case SDLK_F1:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F1 : RK_F1);
			case SDLK_F2:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F2 : RK_F2);
			case SDLK_F3:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F3 : RK_F3);
			case SDLK_F4:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F4 : RK_F4);
			case SDLK_F5:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F5 : RK_F5);
			case SDLK_F6:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F6 : RK_F6);
			case SDLK_F7:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F7 : RK_F7);
			case SDLK_F8:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F8 : RK_F8);
			case SDLK_F9:
				return((event.key.keysym.mod & KMOD_SHIFT) ? RK_SHIFT_F9 : RK_F9);
			case SDLK_F10:		return(RK_F10);
			case SDLK_F11:		return(RK_F11);
			case SDLK_F12:		return(RK_F12);
			case SDLK_LEFT:		return(RK_CURSOR_LEFT);
			case SDLK_DOWN:		return(RK_CURSOR_DOWN);
			case SDLK_UP:		return(RK_CURSOR_UP);
			case SDLK_RIGHT:	return(RK_CURSOR_RIGHT);
			case SDLK_HOME:		return(RK_HOME);
			case SDLK_END:		return(RK_END);
			case SDLK_PAGEUP:	return(RK_PAGE_UP);
			case SDLK_PAGEDOWN:	return(RK_PAGE_DOWN);
			case SDLK_INSERT:	return(RK_INSERT);
			case SDLK_DELETE:	return(RK_DELETE);
			case SDLK_RETURN:	return(RK_ENTER);
			default:
				break;
				/* stop complaints */
			}

			if (event.key.keysym.sym == SDLK_SPACE && (event.key.keysym.mod & KMOD_CTRL))
				return(RK_CTRLSPACE);

			/* the rest can be dealt with the Unicode mapping + alt stuff */
			//20200102
			/*key = (event.key.keysym.unicode & 0x7f);
			if (event.key.keysym.mod & (KMOD_ALT | KMOD_META))
				key += 128;*/

			return(key);

		case SDL_QUIT:
			forceesc = 1;
			return(27);

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT)
				zmouse_state |= MOUSE_LEFTBUTTON;
			else
				if (event.button.button == SDL_BUTTON_RIGHT)
					zmouse_state |= MOUSE_RIGHTBUTTON;
			mouseevent = has_mouse;
			return(RK_NO_KEY);

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT)
				zmouse_state &= ~MOUSE_LEFTBUTTON;
			else
				if (event.button.button == SDL_BUTTON_RIGHT)
					zmouse_state &= ~MOUSE_RIGHTBUTTON;
			mouseevent = has_mouse;
			return(RK_NO_KEY);

		case SDL_MOUSEMOTION:
			mouseevent = has_mouse;
			break;

		default:
			break;
		}
	}

	return(RK_NO_KEY);
}


int zgv_io_waitkey(void)
{
	int ret;

	while ((ret = zgv_io_readnbkey()) == RK_NO_KEY && !zgv_io_timer_flag)
		SDL_WaitEvent(NULL);

	return(ret);
}


int zgv_io_waitevent(void)
{
	int ret;

	mouseevent = 0;
	while ((ret = zgv_io_readnbkey()) == RK_NO_KEY && !mouseevent && !zgv_io_timer_flag)
		SDL_WaitEvent(NULL);

	return(ret);
}


static SDL_TimerID our_timer;


static Uint32 timer_sig(Uint32 interval, void* data)
{
	if (timer_in_use)
	{
		SDL_Event ev;

		ev.type = SDL_USEREVENT;
		SDL_PushEvent(&ev);
	}

	return(60 * 1000);  /* wait a minute (as one-shots seem to be impossible :-( ) */
}


void zgv_io_timer_start(int delay_in_hundredths)
{
	if (timer_in_use)
		zgv_io_timer_stop();

	timer_in_use = 1;

	zgv_io_timer_flag = 0;

	our_timer = SDL_AddTimer(10 * delay_in_hundredths, timer_sig, NULL);
}


void zgv_io_timer_stop(void)
{
	if (!timer_in_use) return;

	if (our_timer)
		SDL_RemoveTimer(our_timer);

	/* drop any unhandled events (XXX not sure this is the best approach,
	 * but something like this is needed).
	 */
	zgv_io_timer_flag = 0;

	timer_in_use = 0;
}


/* provide for a usable file-selector mode, to avoid problems when
 * using restrictive SDL outputs e.g. a single-mode VESA framebuffer.
 */
void zgv_io_fixfsmode(int* mode)
{
#define TRY_MODE(xx) \
  if(modes[xx].has_mode && cfg.mode_allowed[xx]) \
    { *mode=(xx); return; }

	TRY_MODE(G640x480x256);
	TRY_MODE(G800x600x256);
	TRY_MODE(G1024x768x256);
	TRY_MODE(G1280x1024x256);

	printf("zgv: a 640x480, 800x600, 1024x768, or 1280x1024 mode is required.\n");
	exit(1);
}

#else	/* BACKEND_SDL */

int zmouse_getbutton(void) { return 0; }
int zmouse_getx(void) { return 0; }
int zmouse_gety(void) { return 0; }
int zmouse_init_return_fd(char* dev, int type, int samplerate) { return -1; }
void zmouse_setposition(int x, int y) {}
int zmouse_update(void) { return 0; }
int zvga_getmousetype(void) { return 0; }

#endif	/* BACKEND_SDL */


#else	/* BACKEND_SVGALIB */


/* just the timer routines needed here, and dummy screen_update. */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>


static void timer_sig(int foo)
{
	if (timer_in_use)
		zgv_io_timer_flag = 1;
}


void zgv_io_screen_update(void)
{
	/* nothing needed */
}


void zgv_io_timer_start(int delay_in_hundredths)
{
	struct itimerval itv;
	struct sigaction sa;

	if (timer_in_use)
		zgv_io_timer_stop();

	zgv_io_timer_flag = 0;

	timer_in_use = 1;

	sigemptyset(&sa.sa_mask);	/* SIGALRM implicit */
	sa.sa_handler = timer_sig;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGALRM, &sa, NULL);

	itv.it_value.tv_sec = delay_in_hundredths / 100;
	itv.it_value.tv_usec = (delay_in_hundredths % 100) * 10000;
	itv.it_interval.tv_sec = itv.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &itv, NULL);
}


void zgv_io_timer_stop(void)
{
	struct itimerval itv;

	itv.it_value.tv_sec = itv.it_value.tv_usec = 0;
	itv.it_interval.tv_sec = itv.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &itv, NULL);

	timer_in_use = 0;
}


#endif	/* BACKEND_SVGALIB */
