/* Zgv v2.7 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1995 Russell Marks. See README for license details.
 *
 * resizepic.h - proto for resizepic().
 */


extern unsigned char *resizepic(unsigned char *theimage,
  unsigned char *palr,unsigned char *palg,unsigned char *palb,
  int width,int height,int *sw_ask,int *sh_ask);
