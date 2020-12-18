/* Zgv v3.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * readpng.h - external prototypes for readpng.c
 */

extern void aborted_file_png_cleanup(void);
extern int read_png_file(char *filename, hffunc howfarfunc, byte **palette);
