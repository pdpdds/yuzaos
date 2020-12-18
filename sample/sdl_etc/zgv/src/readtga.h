/* Zgv v2.7 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1995 Russell Marks. See README for license details.
 *
 * readtga.h
 */

struct tgahed
  {
  unsigned char idfieldlen,hascmap,type;
  unsigned char cmapstart_lo,cmapstart_hi,cmaplen_lo,cmaplen_hi;
  unsigned char cmapdepth;
  unsigned char xorg_lo,xorg_hi,yorg_lo,yorg_hi;
  unsigned char width_lo,width_hi,height_lo,height_hi;
  unsigned char bpp,desc;
  };


extern int read_tga_file(char *filename,hffunc howfarfunc,
	unsigned char **bmap,unsigned char **pal,int *output_type,
	PICINFO *pp);

extern void aborted_file_tga_cleanup();

extern int tga_need_flip;
