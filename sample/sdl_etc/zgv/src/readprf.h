/* zgv 5.3 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * readprf.h
 */

extern int read_prf_file(char *filename,hffunc howfarfunc,
	unsigned char **bmap,unsigned char **pal,int *output_type,
	PICINFO *pp);
extern void aborted_file_prf_cleanup(void);
