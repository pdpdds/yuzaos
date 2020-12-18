/* Zgv v2.7 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1995 Russell Marks. See README for license details.
 *
 * readpcx.h
 */

extern int read_pcx_file(char *filename,hffunc howfarfunc,
	unsigned char **bmap,unsigned char **pal,int *output_type,
	PICINFO *pp);
extern void aborted_file_pcx_cleanup(void);
