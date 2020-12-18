/* Zgv v5.0 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1999 Russell Marks. See README for license details.
 *
 * readpcd.h
 */

#ifdef PCD_SUPPORT

extern int read_pcd_file(char *filename,hffunc howfarfunc,
	unsigned char **bmap,unsigned char **pal,int *output_type,
	PICINFO *pp);
extern int aborted_file_pcd_cleanup();

#endif
