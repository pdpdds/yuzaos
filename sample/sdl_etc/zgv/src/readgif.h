/* Zgv v5.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1999 Russell Marks. See README for license details.
 *
 * readgif.h - prototypes for readgif.c
 */

extern int gif_delaycount;
extern int *gif_delay;

extern int read_gif_file(char *giffn,hffunc howfarfunc,byte **theimageptr, 
			byte **palptr,int *pixelsize,PICINFO *ginfo);
extern void aborted_file_gif_cleanup(void);
