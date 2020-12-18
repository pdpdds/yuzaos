/* zgv 5.8 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2003 Russell Marks. See README for license details.
 *
 * zgv_io.h
 */

#ifndef BACKEND_SVGALIB
#ifndef BACKEND_SDL
#error "you need to choose a backend in config.mk"
#endif
#endif


extern int current_colour;

extern volatile int zgv_io_timer_flag;

extern void zgv_io_screen_update(void);
extern void zgv_io_timer_start(int delay_in_hundredths);
extern void zgv_io_timer_stop(void);


#ifdef BACKEND_SVGALIB

#include <vga.h>
#include <vgagl.h>
#include <vgamouse.h>

/* vgadrawtext_bmp() needs to know what the current colour is, so... */
#define vga_setcolor(x) vga_setcolor(current_colour=(x))

#else	/* !BACKEND_SVGALIB */

#define vga_modeinfo zvga_modeinfo

#define gl_fillbox zgl_fillbox
#define gl_getbox zgl_getbox
#define gl_putbox zgl_putbox
#define gl_putboxmask zgl_putboxmask
#define gl_putboxpart zgl_putboxpart
#define gl_setcontextvga zgl_setcontextvga
#define mouse_close zmouse_close
#define mouse_getbutton zmouse_getbutton
#define mouse_getx zmouse_getx
#define mouse_gety zmouse_gety
#define mouse_init_return_fd zmouse_init_return_fd
#define mouse_setposition zmouse_setposition
#define mouse_setscale zmouse_setscale
#define mouse_setxrange zmouse_setxrange
#define mouse_setyrange zmouse_setyrange
#define mouse_update zmouse_update
#define vga_clear zvga_clear
#define vga_disabledriverreport zvga_disabledriverreport
#define vga_drawline zvga_drawline
#define vga_drawpixel zvga_drawpixel
#define vga_drawscanline zvga_drawscanline
#define vga_drawscansegment zvga_drawscansegment
#define vga_getcolors zvga_getcolors
#define vga_getcurrentmode zvga_getcurrentmode
#define vga_getmodeinfo zvga_getmodeinfo
#define vga_getmousetype zvga_getmousetype
#define vga_getpalvec zvga_getpalvec
#define vga_getscansegment zvga_getscansegment
#define vga_getxdim zvga_getxdim
#define vga_getydim zvga_getydim
#define vga_hasmode zvga_hasmode
#define vga_init zvga_init
#define vga_lastmodenumber zvga_lastmodenumber
#define vga_lockvc zvga_lockvc
#define vga_oktowrite zvga_oktowrite
#define vga_runinbackground zvga_runinbackground
#define vga_setcolor zvga_setcolor
#define vga_setmode zvga_setmode
#define vga_setpalette zvga_setpalette
#define vga_setpalvec zvga_setpalvec
#define vga_setrgbcolor zvga_setrgbcolor
#define vga_unlockvc zvga_unlockvc

#define TEXT		0
#define G640x480x16	1
#define G320x200x256	2
#define G320x240x256	3
#define G320x400x256	4
#define G360x480x256	5
#define G640x480x256	6
#define G800x600x256	7
#define G1024x768x256	8
#define G1280x1024x256	9
#define G1152x864x256	10
#define G1600x1200x256	11
#define G320x200x32K	12
#define G320x240x32K	13
#define G640x480x32K	14
#define G800x600x32K	15
#define G1024x768x32K	16
#define G1280x1024x32K	17
#define G1152x864x32K	18
#define G1600x1200x32K	19
#define G320x200x64K	20
#define G320x240x64K	21
#define G640x480x64K	22
#define G800x600x64K	23
#define G1024x768x64K	24
#define G1280x1024x64K	25
#define G1152x864x64K	26
#define G1600x1200x64K	27
#define G320x200x16M32	28
#define G320x240x16M32	29
#define G640x480x16M32	30
#define G800x600x16M32	31
#define G1024x768x16M32	32
#define G1280x1024x16M32 33
#define G1152x864x16M32	34
#define G1600x1200x16M32 35
#define G320x200x16M	36
#define G320x240x16M	37
#define G640x480x16M	38
#define G800x600x16M	39
#define G1024x768x16M	40
#define G1280x1024x16M	41
#define G1152x864x16M	42
#define G1600x1200x16M	43

#define GLASTMODE	43

#define MOUSE_NONE		0
#define MOUSE_DEFAULTSAMPLERATE	0
#define MOUSE_LEFTBUTTON	1
#define MOUSE_MIDDLEBUTTON	2
#define MOUSE_RIGHTBUTTON	4
  

typedef struct
  {
  int width,height,bitsperpixel,bytesperpixel,colors,has_mode;
  }
zvga_modeinfo;

extern void zgl_fillbox(int x,int y,int w,int h,int c);
extern void zgl_getbox(int x,int y,int w,int h,void *dp);
extern void zgl_putbox(int x,int y,int w,int h,void *dp);
extern void zgl_putboxmask(int x,int y,int w,int h,void *dp);
extern void zgl_putboxpart(int x,int y,int w,int h,int bw,int bh,void *b,
                   int xo,int yo);
extern int zgl_setcontextvga(int m);

extern void zmouse_close(void);
extern int zmouse_getbutton(void);
extern int zmouse_getx(void);
extern int zmouse_gety(void);
extern int zmouse_init_return_fd(char *dev,int type,int samplerate);
extern void zmouse_setposition(int x,int y);
extern void zmouse_setscale(int s);
extern void zmouse_setxrange(int x1,int x2);
extern void zmouse_setyrange(int y1,int y2);
extern int zmouse_update(void);

extern int zvga_clear(void);
extern void zvga_disabledriverreport(void);
extern int zvga_drawline(int x1,int y1,int x2,int y2);
extern int zvga_drawpixel(int x,int y);
extern int zvga_drawscanline(int line,unsigned char *cols);
extern int zvga_drawscansegment(unsigned char *cols,int x,int y,int len);
extern int zvga_getcolors(void);
extern int zvga_getcurrentmode(void);
extern zvga_modeinfo *zvga_getmodeinfo(int mode);
extern int zvga_getmousetype(void);
extern int zvga_getpalvec(int start,int num,int *pal);
extern int zvga_getscansegment(unsigned char *cols,int x,int y,int len);
extern int zvga_getxdim(void);
extern int zvga_getydim(void);
extern int zvga_hasmode(int mode);
extern int zvga_init(void);
extern int zvga_lastmodenumber(void);
extern void zvga_lockvc(void);
extern int zvga_oktowrite(void);
extern void zvga_runinbackground(int stat);
extern int zvga_setcolor(int col);
extern int zvga_setmode(int mode);
extern int zvga_setpalette(int idx,int r,int g,int b);
extern int zvga_setpalvec(int start,int num,int *pal);
extern int zvga_setrgbcolor(int r,int g,int b);
extern void zvga_unlockvc(void);

extern int zgv_io_readnbkey(void);
extern int zgv_io_waitkey(void);
extern int zgv_io_waitevent(void);
extern void zgv_io_fixfsmode(int *mode);

#endif	/* !BACKEND_SVGALIB */
