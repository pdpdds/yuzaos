/* Zgv v3.2 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1999 Russell Marks. See README for license details.
 *
 * helppage.c - for the online help page displays.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "zgv_io.h"
#include "3deffects.h"
#include "font.h"
#include "readnbkey.h"
#include "rc_config.h"
#include "rcfile.h"
#include "mousecur.h"
#include "zgv.h"

#include "helppage.h"


void helpscreenon(void)
{
unsigned char *tmp;

if(vga_getcurrentmode()!=fs_vgamode)
  vga_setmode(fs_vgamode);
else
  vga_clear();

/* be careful not to suddenly use a border (possibly) for help in
 * 256-colour mode.
 */
if(fs_vgamode==G640x480x16)
  vga_setpalette(0,cfg.medium.r,cfg.medium.g,cfg.medium.b);
else
  {
  int n,w=vga_getxdim(),h=vga_getydim();
  int col=3;
  
  vga_setpalette(col,cfg.medium.r,cfg.medium.g,cfg.medium.b);
  
  /* clear screen with `medium' (i.e. background) colour. */
  if((tmp=malloc(w))!=NULL)
    {
    memset(tmp,col,w);
    for(n=0;n<h;n++)
      vga_drawscanline(n,tmp);
    free(tmp);
    }
  }

vga_setpalette(1,cfg.dark.r,cfg.dark.g,cfg.dark.b);
vga_setpalette(2,cfg.light.r,cfg.light.g,cfg.light.b);
vga_setpalette(15,cfg.black.r,cfg.black.g,cfg.black.b);
mousecur_init(15,2);	/* needed so mouse pointer looks ok */

draw3dbox(0,0,vga_getxdim()-1,vga_getydim()-1,1,1, 2,1);
}


/* the caller is expected to reset the mode and redraw the screen afterwards,
 * if necessary. help is the array pointer (see vgadisp.c and zgv.c).
 * in each string, the left side (key(s)) is separated from the right
 * (description) by a backslash, but you don't *have* to have one.
 * In fact, to help with the video mode help, there's now three columns;
 * use another backslash to get to the third one. If three columns are
 * used, the columns are made narrower.
 *
 * The caller is reponsible for saving mouse before (as this calls
 * restore_mouse_pos()), and for saving after if desired.
 */
void showhelp(int ttyfd,char *title,char *help[])
{
static char tmpbuf[256];
/* colpos[0][] is for two cols, colpos[1][] is for three */
static int colpos_640[2][3]={{90,90+192,0},{40,40+192,40+192*2}};
static char *pkey_no_mouse_str="- press a key to return -";
static char *pkey_mouse_str="- click button or press a key to return -";
int colpos[2][3];
char *pkey_str=(has_mouse?pkey_mouse_str:pkey_no_mouse_str);
int f,g,gotkey,key;
char *ptr1,*ptr2;
int mleft=0,mright=0;
int threecol;
int scrn_w,scrn_h;
int hdr_ypos,subhdr_ypos,linestart,lineheight;

helpscreenon();
restore_mouse_pos();

scrn_w=vga_getxdim(); scrn_h=vga_getydim();

hdr_ypos=scrn_h/12;
subhdr_ypos=(scrn_h*2)/13;
linestart=(scrn_h*11)/48;
lineheight=scrn_h/24;

for(g=0;g<2;g++)
  for(f=0;f<3;f++)
    colpos[g][f]=scrn_w/2+colpos_640[g][f]-320;

gotkey=0;
vga_setcolor(15);
vgadrawtext((scrn_w-vgatextsize(5,title))/2,hdr_ypos,5,title);
vgadrawtext((scrn_w-vgatextsize(3,pkey_str))/2,subhdr_ypos,3,pkey_str);

f=-1;
while(strlen(help[++f]))
  {
  memcpy(tmpbuf,help[f],sizeof(tmpbuf)-1);
  tmpbuf[sizeof(tmpbuf)-1]=0;
  ptr1=strchr(tmpbuf,'\\');
  ptr2=NULL;
  if(ptr1)
    {
    *ptr1=0;
    ptr2=strchr(ptr1+1,'\\');
    if(ptr2) *ptr2=0;
    }
  threecol=(ptr1 && ptr2);
  vgadrawtext(colpos[threecol][0],linestart+f*lineheight,3,tmpbuf);
  if(ptr1)
    vgadrawtext(colpos[threecol][1],linestart+f*lineheight,3,ptr1+1);
  if(ptr2)
    vgadrawtext(colpos[threecol][2],linestart+f*lineheight,3,ptr2+1);
  }

while(!gotkey)
  {
  key=mousecur_wait_for_keys_or_mouse(ttyfd);
  if(has_mouse) mleft=is_end_click_left(),mright=is_end_click_right();
  if(key!=0 || mleft || mright)
    gotkey=1;
  }
}
