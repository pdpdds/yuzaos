/* zgv 5.5 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * scrollbar.c - code for the file selector's horizontal scrollbar.
 */

/* This isn't terribly generic, and would need a certain amount of
 * hacking to get working in other situations.
 */

#include <stdio.h>
#include "zgv_io.h"
#include "3deffects.h"
#include "zgv.h"
#include "scrollbar.h"


static int oldxpos=-1,oldwidth=-1;


void draw_scrollbar_arrows()
{
drawbutton(SCRLBAR_LEFTARROW_XPOS,SCRLBAR_LEFTARROW_YPOS,
           SCRLBAR_LEFTARROW_XPOS+SCRLBAR_ARROW_WIDTH-1,
           SCRLBAR_LEFTARROW_YPOS+SCRLBAR_ARROW_WIDTH-1,
           NULL,0,idx_light,idx_dark,idx_blacknz,0);
vga_setcolor(idx_blacknz);
vga_drawline(SCRLBAR_LEFTARROW_XPOS+3,
             SCRLBAR_LEFTARROW_YPOS+SCRLBAR_ARROW_HEIGHT/2,
             SCRLBAR_LEFTARROW_XPOS+SCRLBAR_ARROW_WIDTH-4,
             SCRLBAR_LEFTARROW_YPOS+SCRLBAR_ARROW_HEIGHT-4);
vga_drawline(SCRLBAR_LEFTARROW_XPOS+SCRLBAR_ARROW_WIDTH-4,
             SCRLBAR_LEFTARROW_YPOS+3,
             SCRLBAR_LEFTARROW_XPOS+SCRLBAR_ARROW_WIDTH-4,
             SCRLBAR_LEFTARROW_YPOS+SCRLBAR_ARROW_HEIGHT-4);
vga_setcolor(idx_light);
vga_drawline(SCRLBAR_LEFTARROW_XPOS+3,
             SCRLBAR_LEFTARROW_YPOS+SCRLBAR_ARROW_HEIGHT/2-1,
             SCRLBAR_LEFTARROW_XPOS+SCRLBAR_ARROW_WIDTH-4,
             SCRLBAR_LEFTARROW_YPOS+3);

drawbutton(SCRLBAR_RIGHTARROW_XPOS,SCRLBAR_RIGHTARROW_YPOS,
           SCRLBAR_RIGHTARROW_XPOS+SCRLBAR_ARROW_WIDTH-1,
           SCRLBAR_RIGHTARROW_YPOS+SCRLBAR_ARROW_WIDTH-1,
           NULL,0,idx_light,idx_dark,idx_blacknz,0);
vga_setcolor(idx_blacknz);
vga_drawline(SCRLBAR_RIGHTARROW_XPOS+3,
             SCRLBAR_RIGHTARROW_YPOS+SCRLBAR_ARROW_HEIGHT-4,
             SCRLBAR_RIGHTARROW_XPOS+SCRLBAR_ARROW_WIDTH-4,
             SCRLBAR_RIGHTARROW_YPOS+SCRLBAR_ARROW_HEIGHT/2);
vga_setcolor(idx_light);
vga_drawline(SCRLBAR_RIGHTARROW_XPOS+SCRLBAR_ARROW_WIDTH-4,
             SCRLBAR_RIGHTARROW_YPOS+SCRLBAR_ARROW_HEIGHT/2-1,
             SCRLBAR_RIGHTARROW_XPOS+3,
             SCRLBAR_RIGHTARROW_YPOS+3);
vga_drawline(SCRLBAR_RIGHTARROW_XPOS+3,
             SCRLBAR_RIGHTARROW_YPOS+3,
             SCRLBAR_RIGHTARROW_XPOS+3,
             SCRLBAR_RIGHTARROW_YPOS+SCRLBAR_ARROW_HEIGHT-4);
}


void draw_scrollbar_main_empty()
{
draw3dbox(SCRLBAR_MAIN_XPOS,SCRLBAR_MAIN_YPOS,
          SCRLBAR_MAIN_XPOS+SCRLBAR_MAIN_WIDTH-1,
          SCRLBAR_MAIN_YPOS+SCRLBAR_MAIN_HEIGHT-1,
          1,0,idx_light,idx_dark);
}


/* draw a complete scrollbar but without a slider.
 * this is called when there's no previous scrollbar onscreen.
 */
void draw_scrollbar_empty()
{
oldxpos=-1; oldwidth=-1;
draw_scrollbar_arrows();
draw_scrollbar_main_empty();
}


void draw_scrollbar_slider(int xpos,int width,int undraw)
{
drawbutton(SCRLBAR_MAIN_XPOS+xpos,
           SCRLBAR_MAIN_YPOS,
           SCRLBAR_MAIN_XPOS+xpos+width-1,
           SCRLBAR_MAIN_YPOS+SCRLBAR_MAIN_HEIGHT-1,
           NULL,0,
           undraw?idx_medium:idx_light,
           undraw?idx_medium:idx_dark,
           undraw?idx_medium:idx_blacknz,
           0);
}


/* for use from zgv.c */
void undraw_scrollbar_slider()
{
if(oldxpos!=-1)
  {
  draw_scrollbar_slider(oldxpos,oldwidth,1);
  draw_scrollbar_main_empty();
  }
oldxpos=oldwidth=-1;
}


void draw_scrollbar_main(int startfrom,int dirsiz,int num_onscreen)
{
/* first find out `percentage', not out of 100 but out of SCRLBAR_MAIN_WIDTH */

/* note that num_onscreen isn't necessarily the number onscreen; it's the
 * *maximum possible* number onscreen. This is easy to compensate for though.
 */
int xpos,width;
int real_num_onscreen=num_onscreen;

if(startfrom+num_onscreen>dirsiz+1)
  real_num_onscreen=dirsiz-startfrom+1;

/* so now our `percentage' is easy: */
width=(real_num_onscreen*SCRLBAR_MAIN_WIDTH)/dirsiz;

/* start position in same terms is a little awkward since startfrom is
 * *one*-based, not zero-based.
 */
xpos=((startfrom-1)*SCRLBAR_MAIN_WIDTH)/dirsiz;

/* undraw old one (saves clearing several pixel lines) */
if(oldxpos!=-1)
  draw_scrollbar_slider(oldxpos,oldwidth,1);
draw_scrollbar_main_empty();
draw_scrollbar_slider(xpos,width,0);
oldxpos=xpos; oldwidth=width;
}


int scrollbar_slider_xpos()
{
return(oldxpos);
}

int scrollbar_slider_width()
{
return(oldwidth);
}


/* convert relative mouse position to curent */
int scrollbar_conv_drag_to_curent(int mx,int dirsiz)
{
int newpos,ent;

newpos=mx-SCRLBAR_MAIN_XPOS;

/* may be out of range, deal with that. */
if(newpos<0) newpos=0;
if(newpos>SCRLBAR_MAIN_WIDTH-1) newpos=SCRLBAR_MAIN_WIDTH-1;

/* now run that through a rearranged version of the stuff we used to
 * calculate xpos.
 */
ent=(dirsiz*newpos)/SCRLBAR_MAIN_WIDTH+1;
if(ent<1) ent=1;
if(ent>dirsiz) ent=dirsiz;

return(ent);
}
