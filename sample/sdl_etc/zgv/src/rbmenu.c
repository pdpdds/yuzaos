/* Zgv v3.1 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1998 Russell Marks. See README for license details.
 *
 * rbmenu.c - routines for right-button menus.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "zgv_io.h"
#include "3deffects.h"
#include "font.h"
#include "rc_config.h"
#include "rcfile.h"
#include "zgv.h"	/* needed for vga_setcolor() macro, for text */
#include "rbmenu.h"


/* (x,y) positions of buttons (i.e. two ints for each button),
 * and number of pairs in `array'.
 */
static int *button_positions=NULL;
static int button_ents=0;
static int button_panel_x=0,button_panel_y=0,button_panel_w=0,button_panel_h=0;
static int button_div_x=1;	/* 2 if in 360x480 mode */



/* in 15/16/24-bit, can't use vga_setcolor(), so we have a routine
 * to take care of that.
 * (in theory this might be better off as a macro, but we assume
 * it's inlined by gcc.)
 */
/* XXX this is also in 3deffects.c, want to avoid that duplication... */
static inline void setcolour(int col)
{
if(vga_getcolors()<=256)
  vga_setcolor(col);
else
  vga_setrgbcolor((col>>16)&255,(col>>8)&255,col&255);
}


/* work out number of entries */
static int rbm_num_ents(struct rbm_data_tag menu_data[])
{
int num_ents;

for(num_ents=0;*(menu_data[num_ents].label)!=0;num_ents++) ;
return(num_ents);
}


/* return size of overall menu panel
 * (position of panel is implicitly `top-right-of-screen')
 */
void rbm_xysize(struct rbm_data_tag menu_data[],int *wp,int *hp)
{
int num_ents=rbm_num_ents(menu_data);

/* work out how wide the menu panel needs to be. We start from the top of
 * the screen filling downwards, shifting that part of the menu across to
 * the left to make room for any more entries.
 * (the panel height is fixed (makes things easier) at 480 pixels.)
 */
*hp=RBM_HEIGHT;
*wp=((num_ents+RBM_MAXENTRIES_Y-1)/RBM_MAXENTRIES_Y)*
	(RBM_ENTRY_WIDTH+RBM_LEFT_XSKIP)+RBM_RIGHT_XSKIP;
if(vga_getcurrentmode()==G360x480x256)
  (*wp)/=2;
}


void rbm_draw(struct rbm_data_tag menu_data[],
	int light,int medium,int dark,int black)
{
static unsigned char linebuf[640];
int *button_pos_ptr;
int num_ents=rbm_num_ents(menu_data);
int f,y;
int panel_x,panel_y,panel_w,panel_h,entry_x,entry_y;
int scrn_w=vga_getxdim();
int mode=vga_getcurrentmode();

/* get rid of any old malloced space
 * (not clear if this is more sensible than resizing it, but it's easier :-))
 */
if(button_positions!=NULL) free(button_positions);

if((button_positions=malloc(sizeof(int)*2*num_ents))==NULL)
  return;

button_ents=num_ents;

rbm_xysize(menu_data,&panel_w,&panel_h);
panel_x=scrn_w-panel_w; panel_y=0;

/* we also need to store panel_[xywh] for later use */
button_panel_x=panel_x;
button_panel_y=panel_y;
button_panel_w=panel_w;
button_panel_h=panel_h;
button_div_x=((mode==G360x480x256)?2:1);

/* draw basic empty panel */
switch(mode)
  {
  /* for 16-colour and 8-bit generic-VGA, don't use vgagl */
  case G640x480x16:
  case G320x200x256: case G320x240x256:
  case G320x400x256: case G360x480x256:
    memset(linebuf,medium,scrn_w);
    for(y=0;y<panel_h;y++)
      vga_drawscansegment(linebuf,panel_x,panel_y+y,panel_w);
    break;
  
  /* otherwise use vgagl */
  default:
    /* should be current context already, but it doesn't hurt to make sure */
    gl_setcontextvga(vga_getcurrentmode());
    
    switch(vga_getcolors())
      {
      case 32768:
        /* this looks dodgy, but check the macro def. and you'll see
         * why I've done it like this.
         */
        medium=GET15BITCOLOUR(medium>>16,medium>>8,medium&255);
        gl_fillbox(panel_x,panel_y,panel_w,panel_h,medium);
        break;
      case 65536:
        medium=GET16BITCOLOUR(medium>>16,medium>>8,medium&255);
        /* FALLS THROUGH */
      default:		/* 8-bit or 24-bit */
        /* input is ok for these two depths already */
        gl_fillbox(panel_x,panel_y,panel_w,panel_h,medium);
      }
  }

/* draw edge */
draw3dbox(panel_x,panel_y,panel_x+panel_w-1,panel_y+panel_h-1,
          3,1,light,dark);

/* now draw the buttons on it. */
entry_x=panel_x+RBM_LEFT_XSKIP/button_div_x;
entry_y=panel_y+RBM_TOP_YSKIP;
button_pos_ptr=button_positions;

for(f=0;f<num_ents;f++)
  {
  /* skip any with label "-", which just provide a gap. */
  if(strcmp(menu_data[f].label,"-")!=0)
    {
    drawbutton(entry_x,entry_y,
               entry_x+RBM_ENTRY_WIDTH/button_div_x-1,
               entry_y+RBM_ENTRY_HEIGHT-3,
               menu_data[f].label,0,light,dark,idx_blacknz,
               menu_data[f].active?black:dark);
    }
  
  *button_pos_ptr++=entry_x;
  *button_pos_ptr++=entry_y;
  
  entry_y+=RBM_ENTRY_HEIGHT;
  if(f%RBM_MAXENTRIES_Y==RBM_MAXENTRIES_Y-1)
    {
    entry_x+=(RBM_ENTRY_WIDTH+RBM_LEFT_XSKIP)/button_div_x;
    entry_y=panel_y+RBM_TOP_YSKIP;
    }
  }
}


int rbm_mousepos_to_key(struct rbm_data_tag menu_data[],int mx,int my)
{
int *button_pos_ptr;
int f,x,y;

if(button_positions==NULL || button_ents==0)
  return(0);

button_pos_ptr=button_positions;

/* if they clicked off the panel, should quit with no effect */
if(mx<button_panel_x || mx>=button_panel_x+button_panel_w ||
   my<button_panel_y || my>=button_panel_y+button_panel_h)
  return(-1);

for(f=0;f<button_ents;f++)
  {
  x=*button_pos_ptr++;
  y=*button_pos_ptr++;
  if(mx>=x && mx<x+RBM_ENTRY_WIDTH/button_div_x &&
     my>=y && my<y+RBM_ENTRY_HEIGHT &&
     menu_data[f].active)
    return(menu_data[f].key);
  }

/* no match */
return(0);
}


/* set/reset active flag on first entry with label containing substr */
void rbm_set_active_flag(struct rbm_data_tag menu_data[],
	char *substr,int active)
{
int f;

for(f=0;*(menu_data[f].label)!=0;f++)
  {
  if(strstr(menu_data[f].label,substr))
    {
    menu_data[f].active=active;
    return;
    }
  }
}


/* hacked version of vgadisp.c's closest() for 8-bit rb menu colour lookup;
 * also used by 3deffects.c's msgbox() similarly for 8-bit dialog colours.
 *
 * pal[rgb]64 are 256-byte arrays of current pal values in range 0..63.
 * not[123] are indicies to avoid checking (or -1 for none).
 * index zero is always avoided, for background and mouse-cursor reasons.
 */
static int rbm_find_closest(int r,int g,int b,
                            int not1,int not2,int not3,int not4,
                            unsigned char *palr64,unsigned char *palg64,
                            unsigned char *palb64)
{
int rgb;
unsigned char *pr,*pg,*pb;
unsigned char distnum;
int xr,xg,xb,dist,distquan,f;

rgb=((b<<12)|(g<<6)|r);
distnum=0;
distquan=20000; /* standard arbitrary bignum */

for(pr=palr64,pg=palg64,pb=palb64,f=0;f<256;f++,pr++,pg++,pb++)
  {
  xr=(r-*pr);
  xg=(g-*pg);
  xb=(b-*pb);
  /* never use idx 0, causes problems for mouse stuff
   * (might be easier to start the loop at idx 1, in fact :-))
   */
  if((dist=xr*xr+xg*xg+xb*xb)<distquan &&
     f!=0 && f!=not1 && f!=not2 && f!=not3 && f!=not4)
    {
    distnum=f;
    distquan=dist;
    if(dist==0) break;  /* premature exit if it can't get any better */
    }
  }
return(distnum);
}


/* find colours when overlaying UI stuff in 8-bit non-selector modes.
 * (It also forces the colours to fit.)
 * See rbm_find_closest() comment above for pal[rgb]64 spec.
 * If any of pal[rgb]64 are NULL, the current onscreen palette is
 * checked directly.
 */
void rbm_find_and_fix_ui_cols(int *lightp,int *mediump,
                              int *darkp,int *blackp,
                              int *mblackp,
                              unsigned char *palr64,unsigned char *palg64,
                              unsigned char *palb64)
{
static unsigned char ownpal[256*3];
static int tmppal[256*3];
int light,medium,dark,black,mblack;

if(!palr64 || !palg64 || !palb64)
  {
  int f;
  
  /* point to our own pal array */
  palr64=ownpal;
  palg64=ownpal+256;
  palb64=ownpal+512;
  
  /* grab palette */
  vga_getpalvec(0,256,tmppal);

  for(f=0;f<256;f++)
    {
    palr64[f]=tmppal[f*3  ];
    palg64[f]=tmppal[f*3+1];
    palb64[f]=tmppal[f*3+2];
    }
  }

/* find closest unique non-zero matches
 * (important for mouse that light/black aren't zero)
 */
light =rbm_find_closest(cfg.light.r,cfg.light.g,cfg.light.b,
                        -1,-1,-1,-1,palr64,palg64,palb64);
medium=rbm_find_closest(cfg.medium.r,cfg.medium.g,cfg.medium.b,
                        light,-1,-1,-1,palr64,palg64,palb64);
dark  =rbm_find_closest(cfg.dark.r,cfg.dark.g,cfg.dark.b,
                        light,medium,-1,-1,palr64,palg64,palb64);
black =rbm_find_closest(cfg.black.r,cfg.black.g,cfg.black.b,
                        light,medium,dark,-1,palr64,palg64,palb64);
mblack=rbm_find_closest(0,0,0,	/* truly black, not whatever cfg.black is */
                        light,medium,dark,black,palr64,palg64,palb64);

/* force colours to fit */
vga_setpalette(light,cfg.light.r,cfg.light.g,cfg.light.b);
vga_setpalette(medium,cfg.medium.r,cfg.medium.g,cfg.medium.b);
vga_setpalette(dark,cfg.dark.r,cfg.dark.g,cfg.dark.b);
vga_setpalette(black,cfg.black.r,cfg.black.g,cfg.black.b);

*lightp=light;
*mediump=medium;
*darkp=dark;
*blackp=black;
*mblackp=mblack;
}
