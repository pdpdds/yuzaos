/* zgv 5.9 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2005 Russell Marks. See README for license details.
 *
 * zgv.h - constants, typedefs.
 */

#define ZGV_VER		"5.9"


#ifdef __FreeBSD__
#define OSTYPE_FREEBSD	1
#else
#define OSTYPE_LINUX	1
#endif


#define _PIC_OK			 0
#define _PICERR_NOFILE		-1
#define _PICERR_NOMEM		-2
#define _PICERR_BADMAGIC	-3
#define _PICERR_NOCOLOURMAP	-4
#define _PICERR_NOIMAGE		-5
#define _PICERR_UNSUPPORTED	-6
#define _PICERR_CORRUPT		-7
#define _PICERR_SEE_ERRMSG	-8
#define _PICERR_ISRLE		-9
#define _PICERR_TOOMANYCOLS	-10
#define _PICERR_BADXCOL		-11

/* not a real error, but used internally by readgif.c */
#define _PICERR_NOMORE		-12

/* macro often used to close input file and return an error from above */
#define CLOSE_AND_RET(x)	do { fclose(in); return(x); } while(0)

/* size of error message buffer used by read{jpeg,png}.c */
#define JPEG_PNG_ERRMSG_SIZE	256

/* required by the PCD support for an unsightly kludge. OTOH, there
 * isn't an obviously nicer way to do it. :-/ The value must be
 * something other than 0, 1, and -1.
 */
#define PIC_INCR_RELOAD_KLUDGE	42


typedef struct
  {
  int width,height;
  int bpp,numcols;
  } PICINFO;

typedef void (*hffunc)(int,int);

typedef unsigned char byte;

extern int idx_light,idx_medium,idx_dark,idx_black,idx_blacknz;
extern int tagview_mode;
extern int zgv_ttyfd;
extern int has_mouse;
extern char jpeg_png_errmsg[];
extern int fs_vgamode;		/* needed by msgbox() and helppage() */

extern void file_details(char *filename,int w,int h,int *need_redraw_ptr);
extern void wait_for_foreground(void);

/* make 15/16-bit colours, used in a few different places */
#define GET15BITCOLOUR(r,g,b) ((((r)&0xf8)<<7)|(((g)&0xf8)<<2)|((b)>>3))
#define GET16BITCOLOUR(r,g,b) ((((r)&0xf8)<<8)|(((g)&0xfc)<<3)|((b)>>3))

/* range check on width and height as a crude way of avoiding overflows
 * when calling malloc/calloc. The maximum we can allow is around 37000,
 * but 32767 at least makes it consistent with xzgv. :-)
 * Adds an extra 2 to height for max-height check, as we usually allocate
 * 2 more lines to allow for dithering.
 */
#define WH_MAX	32767
#define WH_BAD(w,h)	((w)<=0 || (w)>WH_MAX || (h)<=0 || ((h)+2)>WH_MAX)
