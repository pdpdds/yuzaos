/* zgv 5.5 - GIF, JPEG, PNM and BMP viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * readbmp.c - BMP loader.
 *
 * BMP support by Carsten Engelmann (based on readpnm.c).
 */

/* portability fixes RJM 2001 May 25 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
//#include <sys/stat.h>
#include "zgv.h"
#include "readbmp.h"
#include "readpnm.h"	/* for dithering routines */
#include "rcfile.h"
#include "rc_config.h"

/* prototypes */
static int flip (byte * image, unsigned int w, unsigned int h);


/* for aborted_file_bmp_cleanup() */
static unsigned char *work_bmap, *work_pal;
static FILE *work_in;


#define BMPGET2(x)	((x)[0]|((x)[1]<<8))
#define BMPGET4(x)	((x)[0]|((x)[1]<<8)|((x)[2]<<16)|((x)[3]<<24))

void read_bch(BITMAPCOREHEADER *bchp,unsigned char *ptr)
{
bchp->bcSize=BMPGET4(ptr);	ptr+=4;
bchp->bcWidth=BMPGET2(ptr);	ptr+=2;
bchp->bcHeight=BMPGET2(ptr);	ptr+=2;
bchp->bcPlanes=BMPGET2(ptr);	ptr+=2;
bchp->bcBitCount=BMPGET2(ptr);
}

void read_bih(BITMAPINFOHEADER *bihp,unsigned char *ptr)
{
bihp->biSize=BMPGET4(ptr);		ptr+=4;
bihp->biWidth=BMPGET4(ptr);		ptr+=4;
bihp->biHeight=BMPGET4(ptr);		ptr+=4;
bihp->biPlanes=BMPGET2(ptr);		ptr+=2;
bihp->biBitCount=BMPGET2(ptr);		ptr+=2;
bihp->biCompression=BMPGET4(ptr);	ptr+=4;
bihp->biSizeImage=BMPGET4(ptr);		ptr+=4;
bihp->biXPelsPerMeter=BMPGET4(ptr);	ptr+=4;
bihp->biYPelsPerMeter=BMPGET4(ptr);	ptr+=4;
bihp->biClrUsed=BMPGET4(ptr);		ptr+=4;
bihp->biClrImportant=BMPGET4(ptr);
}



int read_bmp_file (char *filename, hffunc howfarfunc, unsigned char **bmap,
                   unsigned char **pal, int *output_type, PICINFO *pp)
{
  FILE *in;
  int i, j;
  int iswindows = 0;
  int dummy, count, done, bytepp, bytes_in_image;
  unsigned char *buf;
  unsigned char read[2];
  unsigned char *p;
  unsigned char *ptr, *dptr, *hptr;
  unsigned int w;
  unsigned int h;
  unsigned int palsize;
  BITMAPINFOHEADER bih;
  BITMAPCOREHEADER bch;

  *bmap = NULL;
  *pal = NULL;

  if ((work_in = in = fopen (filename, "rb")) == NULL)
    return (_PICERR_NOFILE);

  if ((buf = malloc (54)) == NULL)
    CLOSE_AND_RET(_PICERR_NOMEM);

  fread (buf, 1, 26, in);

  if (buf[0] != 'B')
    CLOSE_AND_RET(_PICERR_BADMAGIC);

  if (buf[1] != 'M')
    CLOSE_AND_RET(_PICERR_BADMAGIC);

  read_bch(&bch,buf+14);

  iswindows=0;
  if (bch.bcSize == 40)		/* truly MS_Windows 3.x ?*/
    {
      fread (buf + 26, 1, 28, in);/* then we need the rest */
      read_bih(&bih,buf+14);
      iswindows = 1;
    }

  if ((p = calloc (768,1)) == NULL)
    CLOSE_AND_RET(_PICERR_NOMEM);

  /* only actually used if using dither 8-bit display of 24-bit image,
   * but defined always
   */
  make_332_palette(p);

  palsize=(buf[10]|(buf[11]<<8)|(buf[12]<<16)|(buf[13]<<24));
  if (iswindows)		/*  MS_Windows 3.x */
    {
      pp->width = w = bih.biWidth;
      pp->height = h = bih.biHeight;
      pp->bpp = bih.biBitCount;

      /* Here the "offbits" -  we need 'em for the
       * palette size - e.g. XV uses different sizes
       */
      palsize=(palsize-54)/4;
    }
  else                         /*  OS/2 V1.1       */
    {
      pp->width = w = bch.bcWidth;
      pp->height = h = bch.bcHeight;
      pp->bpp = bch.bcBitCount;
      palsize=(palsize-26)/3;
      
      /* in case these are used later -rjm */
      bih.biWidth=bch.bcWidth;
      bih.biHeight=bch.bcHeight;
    }
  
  if (w == 0 || h == 0 || palsize > 256)
    CLOSE_AND_RET(_PICERR_CORRUPT);

  if ((pp->bpp >> 3) < 3)
    *output_type = 1;

  /* w is the size of a horizontal line in bytes
   * bih.biWidth is the number of valid pixels in a line
   * the latter one is passed to vgadisp.c as pp->width
   */
  switch (pp->bpp)
    {
    case 1:
      if (w % 32)
        w = (w / 32) * 32 + 32;
      break;
    case 4:
      if (w % 8)
	w = (w / 8) * 8 + 8;
      break;
    case 8:
      if (w % 4)
	w = (w / 4) * 4 + 4;
      break;
    case 24:
      if ((w * 3) % 4)
	w = (w * 3 / 4) * 4 + 4;
      else
	w = w * 3;
      break;
    default:
      CLOSE_AND_RET(_PICERR_CORRUPT);
    }

  if (w == 0 || h == 0)
    CLOSE_AND_RET(_PICERR_CORRUPT);

  if ((*pal = calloc (768,1)) == NULL)
    CLOSE_AND_RET(_PICERR_NOMEM);

  bytepp=1;
  if ((pp->bpp == 24) && (*output_type == 3))
    bytepp = 3;
  if (WH_BAD(w,h) ||
      (work_bmap = *bmap = calloc (w * (h + 2) * bytepp,1)) == NULL)
    CLOSE_AND_RET(_PICERR_NOMEM);

  bytes_in_image=w*h*bytepp;

  /* this should be used whenever adding a file-input-dependent amount
   * to the current position pointer. It's > rather than >= so that we
   * can point *just* past the end having read in data, which is ok.
   */
#define FAIL_FOR_BAD_OFFSET(cur,add) \
  do {if((cur)+(add)>bytes_in_image) CLOSE_AND_RET(_PICERR_CORRUPT);} while(0)
  
  /* if you look at the stuff below, `p' is the palette
   * which would be right to free by the time we're actually
   * reading the image, so...
   */
  work_pal=p;

  free(buf);

  switch (pp->bpp)
    {
    case 1:
      /* 1bit non compressed */
      ptr = *pal;
      fread (ptr , 1, 3, in);
      if (iswindows)
        fread (&dummy, 1, 1, in);
      fread (ptr + 3, 1, 3, in);
      if (iswindows)
        fread (&dummy, 1, 1, in);
      for (i = 0; i < 2; i++)
        {
          p[i] = ptr[3 * i + 2];
          p[i + 256] = ptr[3 * i + 1];
          p[i + 512] = ptr[3 * i];
        }
      free (*pal);
      *pal = p;
      ptr = *bmap;
      for (j = h - 1; j >= 0; j--)
        for (i = 0, count=0 ; i < (w >> 3); i++)
          {
            hptr = ptr + j * pp->width;
            dummy = fgetc (in);
            if (count < pp->width)
              {
                hptr[count] = (dummy & 128)?1:0;count++;
                hptr[count] = (dummy & 64)?1:0;count++;
                hptr[count] = (dummy & 32)?1:0;count++;
                hptr[count] = (dummy & 16)?1:0;count++;
                hptr[count] = (dummy & 8)?1:0;count++;
                hptr[count] = (dummy & 4)?1:0;count++;
                hptr[count] = (dummy & 2)?1:0;count++;
                hptr[count] = dummy & 1;count++;
              }
            if (howfarfunc)
              howfarfunc (h-j, h);
          }
      pp->numcols=2;
      break;
    case 4:
      /* 4bit non compressed */
      ptr = *pal;
      for (i = 0; i < palsize; i++)
	{
	  fread (ptr + 3 * i, 1, 3, in);
	  if (iswindows)
	    fread (&dummy, 1, 1, in);
	}
      for (i = 0; i < palsize; i++)
	{
	  p[i] = ptr[3 * i + 2];
	  p[i + 256] = ptr[3 * i + 1];
	  p[i + 512] = ptr[3 * i];
	}
      free (*pal);
      *pal = p;
      ptr = *bmap;
      if ((!iswindows) || (bih.biCompression == 0))
	{
	  for (j = h - 1; j >= 0; j--)
	    for (i = 0, count = 0; i < (w / 2); i++)
	      {
		dummy = fgetc (in);
		if (count < pp->width)
		  {
		    ptr[count + j * pp->width] = dummy >> 4;
		    count++;
		  }
		if (count < pp->width)
		  {
		    ptr[count + j * pp->width] = dummy & 15;
		    count++;
		  }
	      }
	}
      else
	{
	  /* 4bit RLE compressed */
	  done = 0;
	  count = 0;
	  while (done == 0)
	    {
	      fread (read, 1, 2, in);
	      if (*read)
		{
                  FAIL_FOR_BAD_OFFSET(ptr-*bmap,*read);
		  i = 0;
		  do
		    {
		      *ptr = read[1] >> 4;
		      ptr++;
		      i++;
		      if (i < (read[0]))
			{
			  *ptr = read[1] & 15;
			  ptr++;
			  i++;
			}
		    }
		  while (i < (*read));
		}
	      else if (read[1] == 0)
		{
		  if (howfarfunc)
		    howfarfunc (count, h);
		  count++;
		}
	      else if (read[1] == 1)
		done = 1;
	      else if (read[1] == 2)
		{
                int ofs=fgetc (in);
                int ytmp=fgetc (in);
                
                ofs+=bih.biWidth * ytmp;
                FAIL_FOR_BAD_OFFSET(ptr-*bmap,ofs);
		ptr += ofs;
                count += ytmp; if(howfarfunc) howfarfunc(count,h);
		}
	      else
		{
                  FAIL_FOR_BAD_OFFSET(ptr-*bmap,read[1]);
		  dptr = hptr = malloc (((read[1]+3)>>2) << 1);
		  fread (dptr, 1, ((read[1]+3)>>2) << 1, in);
		  i = 0;
		  do
		    {
		      *ptr = (*dptr) >> 4;
		      i++;
		      ptr++;
		      if (i < read[1])
			{
			  *ptr = (*dptr) & 15;
			  i++;
			  ptr++;
			}
		      dptr++;
		    }
		  while (i < read[1]);
		  free (hptr);
		}
	    }
	  if (_PICERR_NOMEM == flip (*bmap, bih.biWidth, bih.biHeight))
	    CLOSE_AND_RET(_PICERR_NOMEM);
	}
      /*pp->width = w;*/		/* believed bogus -rjm 2001 Apr 10 */
      pp->numcols= 16;
      break;

    case 8:
      /* 8bit non compressed */
      ptr = *pal;
      for (i = 0; i < palsize; i++)
	{
	  fread (ptr + 3 * i, 1, 3, in);
	  if (iswindows)
	    dummy = fgetc (in);
	}
      for (i = 0; i < palsize; i++)
	{
	  p[i] = ptr[3 * i + 2];
	  p[i + 256] = ptr[3 * i + 1];
	  p[i + 512] = ptr[3 * i];
	}
      free (*pal);
      *pal = p;
      ptr = *bmap;
      if ((!iswindows) || (bih.biCompression == 0))
	for (i = h - 1; i >= 0; i--)
	  {
	    fread (ptr + pp->width * i, 1, pp->width, in);
	    if (howfarfunc)
	      howfarfunc (h - i, h);
            if(w>pp->width)
              for(j=pp->width;j<w;j++)
                fgetc(in);
	  }
      else
	/* 8bit RLE compressed */
	{
	  done = 0;
	  count = 0;
	  while (done == 0)
	    {
	      fread (read, 1, 2, in);
	      if (read[0])
                {
                FAIL_FOR_BAD_OFFSET(ptr-*bmap,read[0]);
		for (i = 0; i < (int) read[0]; i++)
		  {
		    *ptr = read[1];
		    ptr++;
		  }
                }
	      else if (read[1] == 0)
		{
		  if (howfarfunc)
		    howfarfunc (count, h);
		  count++;
		}
	      else if (read[1] == 1)
		done = 1;
	      else if (read[1] == 2)
                {
                int ofs=fgetc (in);
                int ytmp=fgetc (in);
                
                ofs+=bih.biWidth * ytmp;
                FAIL_FOR_BAD_OFFSET(ptr-*bmap,ofs);
		ptr += ofs;
                count += ytmp; if(howfarfunc) howfarfunc(count,h);
                }
	      else
		{
                FAIL_FOR_BAD_OFFSET(ptr-*bmap,read[1]);
		fread (ptr, 1, read[1], in);
                if (read[1] % 2)
                  fgetc (in);
                ptr += read[1];
		}
	    }
	  if (_PICERR_NOMEM == flip (*bmap, bih.biWidth, bih.biHeight))
	    CLOSE_AND_RET(_PICERR_NOMEM);
	}
      pp->numcols= 256;
      break;

    case 24:
      /* true color */
      /* skip any bogus `palette' */
      for (i = 0; i < palsize; i++)
	{
        fgetc(in); fgetc(in); fgetc(in);
        if(iswindows) fgetc(in);
	}
      /* don't need temp palette at all, just transfer the 8-bit-dither one */
      free (*pal);
      *pal = p;
      ptr = *bmap;
      if ((*output_type) == 1 || cfg.jpeg24bit==0)
	{
          *output_type=1;
	  /* Truecolor BMPs and 8-bit display */
	  if (ditherinit (pp->width) == 0)
	    CLOSE_AND_RET(_PICERR_NOMEM);
	  for (i = 1; i <= h; i++)
	    {
	      hptr = ptr + pp->width * i;
	      fread (hptr, 1, w, in);
	      ditherline (hptr, i, pp->width);
	      if (howfarfunc)
		howfarfunc (i, h);
	    }
	  ditherfinish ();
	  if (_PICERR_NOMEM == flip (*bmap, pp->width, pp->height))
	    CLOSE_AND_RET(_PICERR_NOMEM);
	  pp->bpp = 8;
          pp->numcols= 256;
	}
      else
	for (i = h - 1; i >= 0; i--)
	  {
	    fread (ptr + pp->width * 3 * i, 1, w, in);
	    if (howfarfunc)
	      howfarfunc (h - i, h);
	  }
      break;
    }

  fclose (in);
  return (_PIC_OK);
}

static int flip (unsigned char * image, unsigned int w, unsigned int h)
{
  unsigned int i;
  unsigned char *hptr;

  if ((hptr = malloc (w)) == 0)
    return (_PICERR_NOMEM);
  for (i = 0; i < (h / 2); i++)
    {
      memcpy (hptr, image + i * w, w);
      memcpy (image + i * w, image + (h - i - 1) * w, w);
      memcpy (image + (h - i - 1) * w, hptr, w);
    }
  free (hptr);
  return (_PIC_OK);
}

void aborted_file_bmp_cleanup ()
{
  free (work_bmap);
  free (work_pal);
  fclose (work_in);
}
