/* Zgv v3.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * scrollbar.h - defines for scrollbar.c
 */

extern void draw_scrollbar_empty(void);
extern void draw_scrollbar_main(int startfrom,int dirsiz,int num_onscreen);
extern void undraw_scrollbar_slider(void);
extern int scrollbar_slider_xpos(void);
extern int scrollbar_slider_width(void);
extern int scrollbar_conv_drag_to_curent(int mx,int dirsiz);

/* defines for the overall bar (including the arrows either end) */
#define SCRLBAR_XPOS		5
#define SCRLBAR_WIDTH		(vga_getxdim()-10)
#define SCRLBAR_YPOS		(vga_getydim()-16-5)
#define SCRLBAR_HEIGHT		16

/* defines for both arrows */
#define SCRLBAR_ARROW_WIDTH	SCRLBAR_HEIGHT
#define SCRLBAR_ARROW_HEIGHT	SCRLBAR_HEIGHT

/* defines for left arrow */
#define SCRLBAR_LEFTARROW_XPOS	SCRLBAR_XPOS
#define SCRLBAR_LEFTARROW_YPOS	SCRLBAR_YPOS

/* defines for the main bar itself (not the arrows either end) */
#define SCRLBAR_MAIN_XPOS	(SCRLBAR_XPOS+SCRLBAR_ARROW_WIDTH)
#define SCRLBAR_MAIN_YPOS	SCRLBAR_YPOS
#define SCRLBAR_MAIN_WIDTH	(SCRLBAR_WIDTH-2*(SCRLBAR_ARROW_WIDTH))
#define SCRLBAR_MAIN_HEIGHT	SCRLBAR_HEIGHT

/* defines for right arrow */
#define SCRLBAR_RIGHTARROW_XPOS	(SCRLBAR_MAIN_XPOS+SCRLBAR_MAIN_WIDTH)
#define SCRLBAR_RIGHTARROW_YPOS	SCRLBAR_YPOS

