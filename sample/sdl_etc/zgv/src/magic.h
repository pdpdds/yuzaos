/* zgv 5.3 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * magic.h
 */

#define _IS_GIF			1
#define _IS_JPEG		2
#define _IS_PNM			3
#define _IS_BMP			4
#define _IS_TGA			5
#define _IS_PNG			6
#define _IS_PCX			7
#define _IS_XVPIC		8
#define _IS_MRF			9
#define _IS_XBM			10
#define _IS_XPM			11
#define _IS_TIFF		12
#define _IS_PCD			13
#define _IS_PRF			14

#define _IS_BAD		 	99

extern int magic_ident(char *filename);
