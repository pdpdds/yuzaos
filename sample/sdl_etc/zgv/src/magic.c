/* zgv 5.3 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * magic.c - Determines type of image file.
 */

#include <stdio.h>
#include <string.h>
#include "magic.h"


int magic_ident(char *filename)
{
FILE *in;
unsigned char buf[6];

if((in=fopen(filename,"rb"))==NULL)
  return(_IS_BAD);

memset(buf,0,sizeof(buf));
fread(buf,1,sizeof(buf),in);
fclose(in);

/* We use the following rules:
 * P?M files must have 'P', then a digit; '1'<=digit<='6'.
 * GIF files must have "GIF"
 * JPEG files must have first byte=0xff, second byte=0xd8 (M_SOI)
 * BMP files must start with "BM"
 * PNG files must have 0x89, then "PN". (there's more, but zgv uses these)
 * PCX files have 0x0A, version byte, then 0x01.
 * mrf files must have "MRF"
 * PRF files must have "PRF"
 * XBM files must have "#defin" (this is fairly bogus!)
 * XPM files must have slash then "* XPM"
 * TIFF files must have either "II*\0" or "MM\0*"
 * TGA files suck rocks ;-) (heuristics in this case)
 */

/* xvpics look a bit like P?M files */
if(!strncmp(buf,"P7 ",3))
  return(_IS_XVPIC);

/* PBM/PGM/PPM */
if(buf[0]=='P' && buf[1]>='1' && buf[1]<='6')
  return(_IS_PNM);

/* GIF */
if(strncmp(buf,"GIF",3)==0)
  return(_IS_GIF);
  
/* JPEG */
if(buf[0]==0xff && buf[1]==0xd8)
  return(_IS_JPEG);

/* BMP */
if(buf[0]=='B' && buf[1]=='M')
  return(_IS_BMP);

/* PNG */
if(buf[0]==0x89 && buf[1]=='P' && buf[2]=='N')
  return(_IS_PNG);		/* XXX should test the rest I s'pose */

/* PCX */
if(buf[0]==10 && buf[2]==1)
  return(_IS_PCX);

/* mrf */
if(strncmp(buf,"MRF",3)==0)
  return(_IS_MRF);

/* PRF */
if(strncmp(buf,"PRF",3)==0)
  return(_IS_PRF);

/* XBM */
if(strncmp(buf,"#defin",6)==0)
  return(_IS_XBM);

/* XPM */
if(strncmp(buf,"/* XPM",6)==0)
  return(_IS_XPM);

/* TIFF */
if(memcmp(buf,"II*\0",4)==0 || memcmp(buf,"MM\0*",4)==0)	/* TIFF */
  return(_IS_TIFF);

/* TGA */
/* this is hairy, since TGA files don't have a magic number.
 * we make a guess based on some of the image info.
 * (whether it has a colourmap or not, and the type)
 */
if((buf[1]==1 && (buf[2]==1 || buf[2]==9)) ||
   (buf[1]<=1 && (buf[2]==2 || buf[2]==10)))
  return(_IS_TGA);

/* if we got here, it might be PCD, so read the 3 bytes after skipping
 * 2048 bytes.
 */

#ifdef PCD_SUPPORT
/* have to check that it can still be opened, Linux *is* a multi-tasking
 * OS after all :-)
 */
if((in=fopen(filename,"rb"))!=NULL)
  {
  buf[0]=buf[1]=buf[2]=0;
  fseek(in,2048,SEEK_SET);
  fread(buf,1,3,in);
  fclose(in);
  
  if(strncmp(buf,"PCD",3)==0)
    return(_IS_PCD);
  }
#endif

/* if no valid header */
return(_IS_BAD);
}
