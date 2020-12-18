/* Zgv v3.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * readxbm.h
 */

extern int read_xbm_file(char *filename,hffunc howfarfunc,
	unsigned char **bmap,unsigned char **pal,int *output_type,
	PICINFO *pp);
extern void aborted_file_xbm_cleanup(void);
