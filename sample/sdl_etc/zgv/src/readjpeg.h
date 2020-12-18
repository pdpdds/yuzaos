/* zgv 5.2 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2000 Russell Marks. See README for license details.
 *
 * readjpeg.h - external prototypes for readjpeg.c
 */
 
extern void aborted_file_jpeg_cleanup(void);
extern int read_JPEG_file(char *filename,hffunc howfarfunc,byte **palette,
	int quick,int *real_width,int *real_height);
