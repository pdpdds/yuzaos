/* Zgv v2.7 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1995 Russell Marks. See README for license details.
 *
 * readxvpic.c - load xv-style thumbnail files.
 *		 the routine which does the actual loading is
 *		 read_xv332() in readpnm.c.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zgv.h"
#include "readpnm.h"	/* where read_xv332() is declared */
#include "readxvpic.h"


/* no file abort possible, so no aborted file vars */



int read_xvpic(filename,howfarfunc,bmap,pal,output_type,pp)
char *filename;
hffunc howfarfunc;
unsigned char **bmap;
unsigned char **pal;
int *output_type;  /* returns how many bytes per pixel needed for display */
PICINFO *pp;
{
int w,h;

*bmap=NULL;
*pal=NULL;

if((*pal=calloc(768,1))==NULL)
  return(_PICERR_NOMEM);

make_332_palette(*pal);

if(read_xv332(filename,bmap,&w,&h)==_PICERR_NOFILE)
  return(_PICERR_NOFILE);

pp->width=w;
pp->height=h;
pp->numcols=256;

*output_type=1;

return(_PIC_OK);  
}
