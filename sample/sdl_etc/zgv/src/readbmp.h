/* Zgv v2.7 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-5 Russell Marks. See README for license details.
 *
 * readbmp.h - header for readbmp.c.
 *
 * BMP support by Carsten Engelmann (cengelm@gwdg.de)
 */

/* these are no longer read directly from the file, so
 * endianness, short/int lengths, and struct packing shouldn't
 * be a problem any more.
 * -rjm 2001 May 25.
 */

typedef struct
{
  unsigned int  biSize;
  unsigned int  biWidth;
  unsigned int  biHeight;
  unsigned short  biPlanes;
  unsigned short  biBitCount;
  unsigned int  biCompression;
  unsigned int  biSizeImage;
  unsigned int  biXPelsPerMeter;
  unsigned int  biYPelsPerMeter;
  unsigned int  biClrUsed;
  unsigned int  biClrImportant;
} BITMAPINFOHEADER;

typedef struct
{
  unsigned int  bcSize;
  unsigned short  bcWidth;
  unsigned short  bcHeight;
  unsigned short  bcPlanes;
  unsigned short  bcBitCount;
} BITMAPCOREHEADER;

extern void aborted_file_bmp_cleanup();
extern int read_bmp_file (char *filename, hffunc howfarfunc,
                          unsigned char **bmap, unsigned char **pal,
                          int *output_type, PICINFO *pp);
