/* Zgv v2.7 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1995 Russell Marks. See README for license details.
 *
 * font.h - external prototypes for font.c
 */
 
#define NO_CLIP_FONT  0x7FFFFFFF

/* this as `siz' arg specifically requests the siz==2 `thicktext'
 * font, whether cfg.thicktext is non-zero or not.
 */
#define USE_BOLD_FONT	-1

extern int vgadrawtext(int x,int y,int siz,char *str);
extern int vgatextsize(int sizearg,char *str);
extern void set_max_text_width(int width);
