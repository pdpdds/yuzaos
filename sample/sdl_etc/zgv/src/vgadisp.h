/* zgv 5.2 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2000 Russell Marks. See README for license details.
 *
 * vgadisp.h - prototypes for vgadisp.c
 */


/* required for config reading/writing by zgv.c */
/* pixelsize is also reqd. by readjpeg.c */
extern int curvgamode,zoom,virtual,vkludge,brightness,pixelsize;
extern double contrast,picgamma;

/* reqd. by zgv.c:makexv332() */
extern unsigned char *theimage,*image_palette;
extern int width,height;

extern int pic_incr;	/* for ^p and ^n */

/* picture orientation override (used by Alt-s), needed so we can be
 * sure it's disabled when they get back to the file selector.
 */
extern int orient_override;

extern int readpicture(char *giffn,hffunc howfarfunc,int show_dont_tell,
	int quick,int *real_width,int *real_height);
extern void aborted_file_cleanup(void);
