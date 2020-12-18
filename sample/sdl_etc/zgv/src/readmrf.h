/* Zgv v2.9 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1997 Russell Marks. See README for license details.
 *
 * readmrf.h
 */

extern int read_mrf_file(char *filename,hffunc howfarfunc,
	unsigned char **bmap,unsigned char **pal,int *output_type,
	PICINFO *pp);
extern void aborted_file_mrf_cleanup(void);
