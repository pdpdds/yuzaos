/* Zgv v3.3 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1999 Russell Marks. See README for license details.
 *
 * modesel.h
 */

struct modedesc_tag
  {
  int mode;		/* svgalib's mode number */
  int width,height;
  int bitspp;	       /* 4/8/15/16/24 - bits zgv should write to each pixel */
  			/* NB: above means *32-bit modes have bitspp==24*! */
  int bytespp;		/* 1/2/3/4 - bytes taken by each pixel in video mem */
  int fs_key;		/* key used to get it in FS, RK_NO_KEY if you can't */
  int is_tab_key;	/* if 1, the viewer key must come after a Tab */
  int viewer_key;	/* key used to get it in the viewer (else RK_NO_KEY) */
  };

extern struct modedesc_tag modedesc[];

extern void check_modedesc_array(void);
