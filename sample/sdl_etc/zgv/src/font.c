/* Zgv v3.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * font.c - provides a font of sorts for use via svgalib, and
 *		code to use (nicer) BDF fonts converted with bdf2h.
 */


/* all the line-based font stuff is hard-coded and a bit ugly, so you
 * probably won't want much to look at that (hint) :-)
 */

 
#include <stdio.h>
#include <string.h>
#include "zgv_io.h"
#include "zgv.h"
#include "font.h"
#include "fontbmps.h"
#include "rc_config.h"
#include "rcfile.h"

#define TABSIZE  64   /* size of a tab, in pixels */


static int go_through_the_motions=0;
  /* if 1, we don't draw it, just do the width */

static int stop_after_this_x=NO_CLIP_FONT;

static int squish_x=0;


/* the copyright/license for the bitmap fonts says that all
 * copies of the fonts must contain the copyright. Since a copy
 * was embedded in the program above (`fontbmps.h'), we'd
 * best have this:
 */
static char *bitmap_fonts_copyright=
"The bitmap fonts zgv uses are: \
Copyright 1984-1989, 1994 Adobe Systems Incorporated. \
Copyright 1988, 1994 Digital Equipment Corporation.";


/* prototypes */
int vgadrawtext(int x, int y, int siz, char *str);
void fontcircle(int x, int y, int r);
void fontc_l(int x, int y, int r);
void fontc_u(int x, int y, int r);
void fontc_r(int x, int y, int r);
void fontc_left(int x, int y, int r);
void fontc_ul(int x, int y, int r);
void fontc_ur(int x, int y, int r);
void fontc_ll(int x, int y, int r);
void fontc_lr(int x, int y, int r);
int vgatextsize(int sizearg, char *str);
void set_max_text_width(int width);
static int vgadrawtext_bmp(int xpos,int ypos,int siz,char *str);


/* unused dummy routine to stop -Wall complaining about the copyright
 * message not being used.
 */
static void its_too_damn_picky_sometimes()
{
printf(bitmap_fonts_copyright);
}


/* needed for 360x480 mode */
#define alt_vga_drawline(x1,y1,x2,y2) \
	(squish_x?vga_drawline((x1)>>1,y1,(x2)>>1,y2):\
        	  vga_drawline(x1,y1,x2,y2))
#define alt_vga_drawpixel(x1,y1) \
	(squish_x?vga_drawpixel((x1)>>1,y1):\
        	  vga_drawpixel(x1,y1))


/* returns width of text drawn in pixels */
int vgadrawtext(int x,int y,int siz,char *str)
{
int f,a,b,c,s1,s2,s3,s4,s5,s6,gap;

/* needed for both line and bitmap fonts */
squish_x=(vga_getcurrentmode()==G360x480x256);

if(!cfg.linetext || siz==0) return(vgadrawtext_bmp(x,y,siz,str));

/* bold not supported by linetext version, use siz=2 instead */
if(siz==USE_BOLD_FONT) siz=2;

if(squish_x) x*=2;	/* since vga_drawline/pixel will divide by 2 */

b=y;
a=x;
s1=siz; s2=s1<<1; s3=s1+s2; s4=s2<<1; s5=s4+s1; s6=s3<<1;
gap=s1; if(gap==0) gap=1;

for(f=0;f<strlen(str);f++)
  {
  /* s3+s4 is the size that an ellipsis will take up (s3), plus the
   * widest possible letter (M = s4).
   */
  if(a-x>stop_after_this_x-s3-s4)
    {
    int tmp;
    
    /* print an ellipsis... well, three^H^H^H^H^Htwo dots :-) */
    /* blast the width restriction to stop possible infinite recursion */
    tmp=stop_after_this_x;
    set_max_text_width(NO_CLIP_FONT);
    a+=vgadrawtext(a,y,siz,"..");
    stop_after_this_x=tmp;
    /* now give up */
    break;
    }
  c=str[f];
  
  /*** 1st step: cover some common occurances ***/
  if(!go_through_the_motions)   /* only draw it if we really want to */
    {
    if(strchr("abdgopq68",c)!=NULL)   /* common circle position */
      fontcircle(a+s1,b+s3,siz);
    else
      {    /* common part-circle positions */
      if(strchr("cehmnrs",c)!=NULL)
        fontc_ul(a+s1,b+s3,siz);
      if(strchr("ehmnrBS35",c)!=NULL)
        fontc_ur(a+s1,b+s3,siz);
      if(strchr("cetuyCGJOQSUl035",c)!=NULL)
        fontc_ll(a+s1,b+s3,siz);
      if(strchr("suyBCDGJOQSU035",c)!=NULL)
        fontc_lr(a+s1,b+s3,siz);
      /* common line */
      if(strchr("BDEFHKLMNPR",c)!=NULL)
        alt_vga_drawline(a,b,a,b+s4);
      }
    }

  /*** 2nd step: fill in rest - this is the *really* long, messy bit :-) ***/
  
  /*** 2a: lowercase letters ***/
  if(!go_through_the_motions)
    switch(c)
      {
      case 'a':
        alt_vga_drawline(a+s2,b+s2,a+s2,b+s4); break;
      case 'b':
        alt_vga_drawline(a,b,a,b+s4); break;
      case 'c':
        alt_vga_drawline(a+s1,b+s2,a+s2,b+s2);
        alt_vga_drawline(a+s1,b+s4,a+s2,b+s4); break;
      case 'd':
        alt_vga_drawline(a+s2,b,a+s2,b+s4); break;
      case 'e':
        alt_vga_drawline(a,b+s3,a+s2,b+s3);
        alt_vga_drawline(a+s1,b+s4,a+s2,b+s4); break;
      case 'f':
        fontc_ul(a+s1,b+s1,siz); alt_vga_drawline(a,b+s1,a,b+s4);
        alt_vga_drawline(a,b+s2,a+s1,b+s2); break;
      case 'g':
        alt_vga_drawline(a+s2,b+s2,a+s2,b+s5);
        fontc_l(a+s1,b+s5,siz); break;
      case 'h':
        alt_vga_drawline(a,b,a,b+s4); alt_vga_drawline(a+s2,b+s3,a+s2,b+s4); break;
      case 'i':
        alt_vga_drawpixel(a,b+s1);
        alt_vga_drawline(a,b+s2,a,b+s4);
        a+=-s1+1; break;
      case 'j':
        alt_vga_drawline(a+s1,b+s2,a+s1,b+s5);
        fontc_lr(a,b+s5,siz);
        alt_vga_drawpixel(a+s1,b+s1); break;
      case 'k':
        alt_vga_drawline(a,b,a,b+s4); alt_vga_drawline(a,b+s3,a+s1,b+s2);
        alt_vga_drawline(a,b+s3,a+s1,b+s4); break;
      case 'l':
        alt_vga_drawline(a,b,a,b+s3); break;
      case 'm':
        alt_vga_drawline(a,b+s2,a,b+s4); alt_vga_drawline(a+s2,b+s3,a+s2,b+s4);
        alt_vga_drawline(a+s4,b+s3,a+s4,b+s4); fontc_u(a+s3,b+s3,siz);
        break;
      case 'n':
        alt_vga_drawline(a,b+s2,a,b+s4); alt_vga_drawline(a+s2,b+s3,a+s2,b+s4);
        break;
      case 'p':
        alt_vga_drawline(a,b+s2,a,b+s6); break;
      case 'q':
        alt_vga_drawline(a+s2,b+s2,a+s2,b+s6); break;
      case 'r':
        alt_vga_drawline(a,b+s2,a,b+s4); break;
      case 's':
        alt_vga_drawline(a,b+s3,a+s2,b+s3); alt_vga_drawline(a+s1,b+s2,a+s2,b+s2);
        alt_vga_drawline(a,b+s4,a+s1,b+s4);
        break;
      case 't':
        alt_vga_drawline(a,b+s1,a,b+s3); alt_vga_drawline(a,b+s2,a+s1,b+s2);
        break;
      case 'u':
        alt_vga_drawline(a,b+s2,a,b+s3); alt_vga_drawline(a+s2,b+s2,a+s2,b+s4);
        break;
      case 'v':
        alt_vga_drawline(a,b+s2,a+s1,b+s4); alt_vga_drawline(a+s1,b+s4,a+s2,b+s2);
        break;
      case 'w':
        alt_vga_drawline(a,b+s2,a+s1,b+s4); alt_vga_drawline(a+s1,b+s4,a+s2,b+s3);
        alt_vga_drawline(a+s2,b+s3,a+s3,b+s4); alt_vga_drawline(a+s3,b+s4,a+s4,b+s2);
        break;
      case 'x':
        alt_vga_drawline(a,b+s2,a+s2,b+s4); alt_vga_drawline(a,b+s4,a+s2,b+s2);
        break;
      case 'y':
        alt_vga_drawline(a,b+s2,a,b+s3);
        alt_vga_drawline(a+s2,b+s2,a+s2,b+s5);
        fontc_l(a+s1,b+s5,siz); break;
      case 'z':
        alt_vga_drawline(a,b+s2,a+s2,b+s2); alt_vga_drawline(a+s2,b+s2,a,b+s4);
        alt_vga_drawline(a,b+s4,a+s2,b+s4); break;
      
      /*** 2b: uppercase letters ***/
  
      case 'A':
        alt_vga_drawline(a,b+s4,a+s1,b); alt_vga_drawline(a+s1,b,a+s2,b+s4);
        alt_vga_drawline(a+(s1>>1),b+s2,a+s2-(s1>>1),b+s2); break;
      case 'B':
        fontc_r(a+s1,b+s1,siz);
        alt_vga_drawline(a,b,a+s1,b); alt_vga_drawline(a,b+s2,a+s1,b+s2);
        alt_vga_drawline(a,b+s4,a+s1,b+s4); break;
      case 'C':
        fontc_u(a+s1,b+s1,siz); alt_vga_drawline(a,b+s1,a,b+s3);
        break;
      case 'D':
        alt_vga_drawline(a,b,a+s1,b);
        alt_vga_drawline(a,b+s4,a+s1,b+s4); fontc_ur(a+s1,b+s1,siz);
        alt_vga_drawline(a+s2,b+s1,a+s2,b+s3); break;
      case 'E':
        alt_vga_drawline(a,b,a+s2,b); alt_vga_drawline(a,b+s2,a+s1,b+s2);
        alt_vga_drawline(a,b+s4,a+s2,b+s4); break;
      case 'F':
        alt_vga_drawline(a,b,a+s2,b); alt_vga_drawline(a,b+s2,a+s1,b+s2);
        break;
      case 'G':
        fontc_u(a+s1,b+s1,siz); alt_vga_drawline(a,b+s1,a,b+s3);
        alt_vga_drawline(a+s1,b+s2,a+s2,b+s2); alt_vga_drawline(a+s2,b+s2,a+s2,b+s3);
        break;
      case 'H':
        alt_vga_drawline(a,b+s2,a+s2,b+s2); alt_vga_drawline(a+s2,b,a+s2,b+s4);
        break;
      case 'I':
        alt_vga_drawline(a,b,a+s2,b); alt_vga_drawline(a+s1,b,a+s1,b+s4);
        alt_vga_drawline(a,b+s4,a+s2,b+s4); break;
      case 'J':
        alt_vga_drawline(a+s2,b,a+s2,b+s3); break;
      case 'K':
        alt_vga_drawline(a+s2,b,a,b+s2); alt_vga_drawline(a,b+s2,a+s2,b+s4); break;
      case 'L':
        alt_vga_drawline(a,b+s4,a+s2,b+s4); break;
      case 'M':
        alt_vga_drawline(a,b,a+s1+(s1>>1),b+s2);
        alt_vga_drawline(a+s1+(s1>>1),b+s2,a+s3,b);
        alt_vga_drawline(a+s3,b,a+s3,b+s4); a-=s1; break;
      case 'N':
        alt_vga_drawline(a,b,a+s2,b+s4); alt_vga_drawline(a+s2,b+s4,a+s2,b); break;
      case 'Q':
        alt_vga_drawline(a+s1,b+s3,a+s2,b+s4);
        /* FALLS THROUGH and adds an O, finishing the Q */ 
      case 'O': case '0':   /* all other numbers done later */
        fontc_u(a+s1,b+s1,siz); alt_vga_drawline(a,b+s1,a,b+s3);
        alt_vga_drawline(a+s2,b+s1,a+s2,b+s3); break;
      case 'R':
        alt_vga_drawline(a+s1,b+s2,a+s2,b+s4);
        /* FALLS THROUGH and adds a P, finishing the R */
      case 'P':
        fontc_r(a+s1,b+s1,siz); alt_vga_drawline(a,b,a+s1,b);
        alt_vga_drawline(a,b+s2,a+s1,b+s2); break;
      case 'S':
        fontc_left(a+s1,b+s1,siz); fontc_ur(a+s1,b+s1,siz); break;
      case 'T':
        alt_vga_drawline(a,b,a+s2,b); alt_vga_drawline(a+s1,b,a+s1,b+s4); break;
      case 'U':
        alt_vga_drawline(a,b,a,b+s3); alt_vga_drawline(a+s2,b,a+s2,b+s3); break;
      case 'V':
        alt_vga_drawline(a,b,a+s1,b+s4); alt_vga_drawline(a+s1,b+s4,a+s2,b); break;
      case 'W':
        alt_vga_drawline(a,b,a+s1,b+s4); alt_vga_drawline(a+s1,b+s4,a+s2,b+s2);
        alt_vga_drawline(a+s2,b+s2,a+s3,b+s4); alt_vga_drawline(a+s3,b+s4,a+s4,b);
        break;
      case 'X':
        alt_vga_drawline(a,b,a+s2,b+s4); alt_vga_drawline(a+s2,b,a,b+s4); break;
      case 'Y':
        alt_vga_drawline(a,b,a+s1,b+s2); alt_vga_drawline(a+s2,b,a+s1,b+s2);
        alt_vga_drawline(a+s1,b+s2,a+s1,b+s4); break;
      case 'Z':
        alt_vga_drawline(a,b,a+s2,b); alt_vga_drawline(a+s2,b,a,b+s4);
        alt_vga_drawline(a,b+s4,a+s2,b+s4); break;
      
      /*** 2c: numbers ***/
    
      /* 0 already done */
      case '1':
        alt_vga_drawline(a,b+s1,a+s1,b); alt_vga_drawline(a+s1,b,a+s1,b+s4);
        alt_vga_drawline(a,b+s4,a+s2,b+s4); break;
      case '2':
        fontc_u(a+s1,b+s1,siz); alt_vga_drawline(a+s2,b+s1,a,b+s4);
        alt_vga_drawline(a,b+s4,a+s2,b+s4); break;
      case '3':
        fontc_u(a+s1,b+s1,siz); fontc_lr(a+s1,b+s1,siz); break;
      case '4':
        alt_vga_drawline(a+s1,b+s4,a+s1,b); alt_vga_drawline(a+s1,b,a,b+s2);
        alt_vga_drawline(a,b+s2,a+s2,b+s2); break;
      case '5':
        alt_vga_drawline(a+s2,b,a,b); alt_vga_drawline(a,b,a,b+s2);
        alt_vga_drawline(a,b+s2,a+s1,b+s2); break;
      case '6':
        fontc_u(a+s1,b+s1,siz); alt_vga_drawline(a,b+s1,a,b+s3); break;
      case '7':
        alt_vga_drawline(a,b,a+s2,b); alt_vga_drawline(a+s2,b,a+s1,b+s4); break;
      case '9':
        alt_vga_drawline(a+s2,b,a+s2,b+s4);
        /* FALLS THROUGH and does top circle of 8 to complete the 9 */
      case '8':
        fontcircle(a+s1,b+s1,siz); break;
      
      /* 2d: some punctuation (not much!) */
      
      case '-':
        alt_vga_drawline(a,b+s2,a+s1,b+s2); break;
      case '.':
        alt_vga_drawpixel(a,b+s4); a+=-s1+1; break;
      case '(':
        fontc_ul(a+s1,b+s1,siz); fontc_ll(a+s1,b+s3,siz);
        alt_vga_drawline(a,b+s1,a,b+s3); break;
      case ')':
        fontc_ur(a,b+s1,siz); fontc_lr(a,b+s3,siz);
        alt_vga_drawline(a+s1,b+s1,a+s1,b+s3); break;
      case '%':
        alt_vga_drawpixel(a,b);
        alt_vga_drawpixel(a+s2,b+s4);
        /* FALLS THROUGH drawing the slash to finish the % */
      case '/':
        alt_vga_drawline(a,b+s4,a+s2,b); break;
      case '?':
        fontc_u(a+s1,b+s1,siz);
        alt_vga_drawline(a+s2,b+s1,a+s1,b+s2);
        alt_vga_drawline(a+s1,b+s2,a+s1,b+s3);
        alt_vga_drawpixel(a+s1,b+s4);
        break;
      }

    
  /*** 3rd part: finally, move along for the next letter ***/
  /* we do this even if go_through_the_motions is set */
  if(strchr("ltfijk-. ()",c)!=NULL)
    a+=s1;
  else
    {
    if(strchr("?/%abcdeghnopqrsuvxyz"
              "ABCDEFGHIJKLNOPQRSTUVXYZ0123456789",c)!=NULL)
      a+=s2;
    else
      {
      if(strchr("mwMW",c)!=NULL)
        a+=s4;
      else
        {
        if(c==9)
          {
          /* congratulations madam, it's a tab */
          a=((a/TABSIZE)+1)*TABSIZE;
          }
        else
          {
          /* oh, don't know this one. do an underscore */
          /* (we don't if go_through_the_motions is set) */
          if(!go_through_the_motions)
            alt_vga_drawline(a,b+s4,a+s2,b+s4);
          a+=s2;
          }
        }
      }
    }
  a+=gap;   /* and add a gap */
  }
return(a-x);
}


/* never mind. Character building, wasn't it? (groan) */

void fontcircle(int x,int y,int r)
{
fontc_u(x,y,r);
fontc_l(x,y,r);
}

void fontc_l(int x,int y,int r)
{
fontc_ll(x,y,r);
fontc_lr(x,y,r);
}

void fontc_u(int x,int y,int r)
{
fontc_ul(x,y,r);
fontc_ur(x,y,r);
}

void fontc_r(int x,int y,int r)
{
fontc_ur(x,y,r);
fontc_lr(x,y,r);
}

void fontc_left(int x,int y,int r)
{
fontc_ul(x,y,r);
fontc_ll(x,y,r);
}

void fontc_ul(int x,int y,int r)
{
int r34;

if(go_through_the_motions) return;
r34=((r*3)>>2);
alt_vga_drawline(x-r,y,x-r34,y-r34);
alt_vga_drawline(x-r34,y-r34,x,y-r);
}

void fontc_ur(int x,int y,int r)
{
int r34;

if(go_through_the_motions) return;
r34=((r*3)>>2);
alt_vga_drawline(x+r,y,x+r34,y-r34);
alt_vga_drawline(x+r34,y-r34,x,y-r);
}

void fontc_ll(int x,int y,int r)
{
int r34;

if(go_through_the_motions) return;
r34=((r*3)>>2);
alt_vga_drawline(x-r,y,x-r34,y+r34);
alt_vga_drawline(x-r34,y+r34,x,y+r);
}

void fontc_lr(int x,int y,int r)
{
int r34;

if(go_through_the_motions) return;
r34=((r*3)>>2);
alt_vga_drawline(x+r,y,x+r34,y+r34);
alt_vga_drawline(x+r34,y+r34,x,y+r);
}


/* this gets how wide text will be */
int vgatextsize(int sizearg,char *str)
{
int r;

go_through_the_motions=1;
r=vgadrawtext(0,0,sizearg,str);
go_through_the_motions=0;
return(r);
}


void set_max_text_width(int width)
{
stop_after_this_x=width;
}



static int vgadrawtext_bmp(int xpos,int ypos,int siz,char *str)
{
static unsigned char scanbit[128];
static int old_colour=-1;
struct fontinfo_tag *fontinfo_ptr;
int f,c,x,y;
int initxpos=xpos;
signed char *ptr,*fontdat;
int ox,oy,w,h,dw;
int yst;
int xrunst=-1;
int *fonttbl;
int fontyofs,fontfh,fontoy;
int widthadd=0;
int m_ellipsis_size;
int dont_use_lines=0;	/* needed for use from rbmenu.c */
int origxpos=xpos;	/* this and above needed for 360x480 mode */

if(vga_getcolors()>256 || squish_x)
  dont_use_lines=1;

if(current_colour!=old_colour && !dont_use_lines)
  memset(scanbit,old_colour=current_colour,sizeof(scanbit));

/* pick font to match siz. */
switch(siz)
  {
  /* special case, small italic font for "version X.Y" text */
  case 0: fontinfo_ptr=&font0; break;
    
  case 5: fontinfo_ptr=&font3; break;
  case 4: widthadd=1;
    /* falls through */
  case 3:
    fontinfo_ptr=&font2;
    if(cfg.thicktext) fontinfo_ptr=&font2b;
    break;
  
  case USE_BOLD_FONT:
    fontinfo_ptr=&font2b;
    break;
  
  /* this doesn't appear normally, but will if someone hacks
   * on zgv blindly. It gently gives them a clue. :-)
   */
  default:
	  printf("unsupported text size - treating siz as 2\n");
    /* falls through */
  
  case 2: fontinfo_ptr=&font1; break;
  }

fontdat=fontinfo_ptr->data;
fonttbl=fontinfo_ptr->table;
fontyofs=fontinfo_ptr->yofs;
fontfh=fontinfo_ptr->fh;
fontoy=fontinfo_ptr->oy;

/* m_ellipsis_size is the size that an ellipsis will take up, plus the
 * widest possible letter (which we assume is M).
 */
m_ellipsis_size=2*(widthadd+fontdat[fonttbl['.'-32]+4])+
		   widthadd+fontdat[fonttbl['m'-32]+4];

for(f=0;f<strlen(str);f++)
  {
  if(xpos-initxpos>stop_after_this_x-m_ellipsis_size)
    {
    int tmp;
    
    /* print an ellipsis... well, three^H^H^H^H^Htwo dots :-) */
    /* blast the width restriction to stop possible infinite recursion */
    tmp=stop_after_this_x;
    set_max_text_width(NO_CLIP_FONT);
    xpos+=vgadrawtext_bmp(xpos,ypos,siz,"..");
    stop_after_this_x=tmp;
    /* now give up */
    break;
    }
  c=str[f];
  if(c=='\t')
    {
    xpos=((xpos/TABSIZE)+1)*TABSIZE;
    continue;
    }
  if(c<32 || c>126) c='_';
  ptr=fontdat+fonttbl[c-32];
  ox=*ptr++; oy=*ptr++;
  w =*ptr++; h =*ptr++;
  dw=*ptr++;
  yst=fontfh-(oy-fontoy)-fontyofs-h;
  
  if(!go_through_the_motions && c!='\t')
    for(y=ypos+yst;y<ypos+yst+h;y++)
      {
      /* use the obvious way for 15/16/24-bit modes
       * (saves having to deal with colours directly).
       * also do pixel-by-pixel for 360x480, as we want to
       * divide x offset by two, which is much easier this way.
       */
      if(dont_use_lines)
        {
        if(squish_x)
          {
          for(x=xpos+ox;x<xpos+ox+w;x++)
            if(*ptr++) vga_drawpixel((x-origxpos)/2+origxpos,y);
          }
        else
          for(x=xpos+ox;x<xpos+ox+w;x++)
            if(*ptr++) vga_drawpixel(x,y);
        }
      else
        {
        for(x=xpos+ox;x<xpos+ox+w;x++)
          /* this is a little faster, I think, especially
           * for large fonts. (It uses horizontal lines where possible.)
           */
          if(*ptr++)
            {if(xrunst==-1) xrunst=x;}
          else
            if(xrunst!=-1)
              {
              if(xrunst==x-1)
                vga_drawpixel(xrunst,y);
              else
                vga_drawscansegment(scanbit,xrunst,y,x-xrunst);
              xrunst=-1;
              }
        
        if(xrunst!=-1)
          {
          if(xrunst==x-1)
            vga_drawpixel(xrunst,y);
          else
            vga_drawscansegment(scanbit,xrunst,y,x-xrunst);
          }
        xrunst=-1;
        }
      }
  
  xpos+=dw+widthadd;
  }

return(xpos-initxpos);
}
