/* zgv 5.5 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2001 Russell Marks. See README for license details.
 *
 * 3deffects.c - provides the `3d' style boxes, text etc.
 *                used by zgv.c (for file selector) and vgadisp.c
 *                (for help screen)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "zgv_io.h"
#include "3deffects.h"
#include "font.h"
#include "readnbkey.h"
#include "zgv.h"
#include "rc_config.h"
#include "rcfile.h"
#include "mousecur.h"
#include "rbmenu.h"	/* for rbm_find_and_fix_ui_cols() */
#include <getenv.h>

int msgbox_draw_ok=0;	/* 1 if ok to draw msgbox in current mode */

static unsigned char *save_mem=NULL;
static int save_x,save_y,save_width,save_height;

/* records whether last msgbox() call restored the screen successfully;
 * if so, the caller can avoid a redraw by checking this via
 * msgbox_did_restore().
 */
static int last_msgbox_did_restore=1;


/* in 15/16/24-bit, can't use vga_setcolor(), so we have a routine
 * to take care of that.
 * (in theory this might be better off as a macro, but we assume
 * it's inlined by gcc.)
 */
static inline void setcolour(int col)
{
if(vga_getcolors()<=256)
  vga_setcolor(col);
else
  vga_setrgbcolor((col>>16)&255,(col>>8)&255,col&255);
}


/* produce tacky 3d text */
void drawtext3d(int x,int y,int s,char *str,
                int isout,int light,int dark,int txt)
{
setcolour(isout?light:dark);
vgadrawtext(x-1,y-1,s,str);
setcolour(isout?dark:light);
vgadrawtext(x+1,y+1,s,str);
setcolour(txt);
vgadrawtext(x,y,s,str);
}


/* restore sanity */
void undrawtext3d(int x,int y,int s,char *str)
{
setcolour(idx_medium);
vgadrawtext(x-1,y-1,s,str);
vgadrawtext(x+1,y+1,s,str);
vgadrawtext(x,y,s,str);
}


/* render each bock in 3d */
void draw3dbox(int x1,int y1,int x2,int y2,int depth,
               int isout,int light,int dark)
{
int f;

for(f=0;f<depth;f++)
  {
  setcolour(isout?light:dark);
  vga_drawline(x1+f,y2-f,x1+f,y1+f);
  vga_drawline(x1+f,y1+f,x2-f,y1+f);
  setcolour(isout?dark:light);
  vga_drawline(x2-f,y1+f,x2-f,y2-f);
  vga_drawline(x2-f,y2-f,x1+f,y2-f);
  }
}


/* Undraw each relevant bock */
void undraw3dbox(int x1,int y1,int x2,int y2,int depth)
{
int f;

setcolour(idx_medium);
for(f=0;f<depth;f++)
  {
  vga_drawline(x1+f,y2-f,x1+f,y1+f);
  vga_drawline(x1+f,y1+f,x2-f,y1+f);
  vga_drawline(x2-f,y1+f,x2-f,y2-f);
  vga_drawline(x2-f,y2-f,x1+f,y2-f);
  }
}


void drawbutton(int x1,int y1,int x2,int y2,char *str,int centred,
                int light,int dark,int realblack,int txt)
{
setcolour(light);
vga_drawline(x1,y2-1,x1,y1);
vga_drawline(x1,y1,x2-1,y1);
setcolour(realblack);
vga_drawline(x2,y1,x2,y2);
vga_drawline(x2,y2,x1,y2);
setcolour(dark);
vga_drawline(x1+2,y2-1,x2-1,y2-1);
vga_drawline(x2-1,y2-1,x2-1,y1+2);
if(str && str[0])
  {
  setcolour(txt);
  if(centred)
    vgadrawtext(x1+((x2-x1+1)-vgatextsize(3,str))/2,y1+6,3,str);
  else
    vgadrawtext(x1+5,y1+4,3,str);	/* mainly for rbmenu.c */
  }
}


/* save old contents of area to put right-button menu on, and draw it.
 * we know it's either a 4-bit or 8/15/16/24/32=bit mode.
 */
static void save_area(int x1,int y1,int x2,int y2)
{
save_x=x1;
save_y=y1;
save_width=x2-x1+1;
save_height=y2-y1+1;

/* max of 4 bytes per pixel */
if((save_mem=malloc(save_width*save_height*4))==NULL) return;

if(vga_getcurrentmode()!=G640x480x16)
  {
  /* use vgagl. mouse cursor does this too, but since we're doing the
   * same thing it won't hurt. :-)
   * (besides which, mouse might not be enabled...)
   */
  gl_setcontextvga(vga_getcurrentmode());
  gl_getbox(save_x,save_y,save_width,save_height,save_mem);
  }
else
  {
  /* 16-colour, use vga_getscansegment */
  unsigned char *ptr=save_mem;
  int y;
  
  for(y=0;y<save_height;y++,ptr+=save_width)
    vga_getscansegment(ptr,save_x,save_y+y,save_width);
  }
}


/* restore old contents of area */
static void restore_area()
{
if(save_mem==NULL) return;	/* ran out of memory, can't do much! */

if(vga_getcurrentmode()!=G640x480x16)
  {
  /* use vgagl again */
  gl_putbox(save_x,save_y,save_width,save_height,save_mem);
  }
else
  {
  /* 16-colour */
  unsigned char *ptr=save_mem;
  int y;
  
  for(y=0;y<save_height;y++,ptr+=save_width)
    vga_drawscansegment(ptr,save_x,save_y+y,save_width);
  }

free(save_mem);
}


static void blank_area(int x1,int y1,int x2,int y2,int col)
{
int scrncol;

if(vga_getcurrentmode()==G640x480x16)
  {
  static unsigned char scanbit[640];
  int f;
  
  memset(scanbit,col,sizeof(scanbit));
  for(f=y1;f<y2;f++)
    vga_drawscansegment(scanbit,x1,f,x2-x1+1);
  return;
  }

switch(vga_getcolors())
  {
  case 32768: scrncol=GET15BITCOLOUR(col>>16,(col>>8)&255,col&255); break;
  case 65536: scrncol=GET16BITCOLOUR(col>>16,(col>>8)&255,col&255); break;
  default:    scrncol=col;
  }

gl_setcontextvga(vga_getcurrentmode());
gl_fillbox(x1,y1,x2-x1+1,y2-y1+1,scrncol);
}


/* see def of last_msgbox_did_restore above for details */
int msgbox_did_restore(void)
{
return(last_msgbox_did_restore);
}


/* if msgbox_draw_ok is zero, it changes to file-selector mode before
 * drawing the box, and obviously this blasts the screen. :-)
 * Otherwise, it saves/restores the area used as needed. This only
 * works for 16 and 256-col modes, but we know we must be on file selector
 * if msgbox_draw_ok is non-zero, so that's ok.
 *
 * NB: for type MSGBOXTYPE_FILEDETAILS, `message' is corrupted by the
 * routine (`\n's overwritten with NULs).
 */
int msgbox(int ttyfd,char *message,int replytype,int light,int dark,int txt)
{
static char *fdlabels[7]=
  {"Filename:","Size:","Last modified:","Permissions:",
   "Owner:","Group:","Dimensions:"};
static int palsav[256*3];
int savedpal=0;
int x1,y1,x2,y2,wide,high,key;
struct { int x1,y1,x2,y2; } but1,but2;
char *ptr;
int retval=0;
int save_and_restore=1;
int mouseblack=idx_blacknz;
char *fdetailstr[7];
int fdlabelwidth=130;	/* width of (left-hand) label bit for file details */

last_msgbox_did_restore=0;

if(!msgbox_draw_ok)
  {
  mouseblack=1;		/* mustn't be zero */
  
  if(vga_getxdim()<640 || vga_getydim()<480)
    {
    vga_setmode(fs_vgamode);
    save_and_restore=0;
    }

  switch(vga_getcolors())
    {
    case 256:
      vga_getpalvec(0,256,palsav);
      savedpal=1;
      rbm_find_and_fix_ui_cols(&light,&idx_medium,&dark,&txt,&mouseblack,
                               NULL,NULL,NULL);
      break;
    case 16:
      if(save_and_restore) /* then we haven't changed mode, so... */
        vga_clear();
      save_and_restore=0;
      vga_setpalette(0,0,0,0);
      vga_setpalette(idx_medium=1,cfg.medium.r,cfg.medium.g,cfg.medium.b);
      vga_setpalette(dark=2,cfg.dark.r,cfg.dark.g,cfg.dark.b);
      vga_setpalette(light=3,cfg.light.r,cfg.light.g,cfg.light.b);
      vga_setpalette(txt =4,cfg.black.r,cfg.black.g,cfg.black.b);
      vga_setpalette(mouseblack=5,0,0,0);	/* a non-zero-index black */
      break;
    default:	/* 15/16/24/32-bit */
      idx_medium=4*((cfg.medium.r<<16)|(cfg.medium.g<<8)|cfg.medium.b);
      dark      =4*((cfg.dark.r<<16)|(cfg.dark.g<<8)|cfg.dark.b);
      light     =4*((cfg.light.r<<16)|(cfg.light.g<<8)|cfg.light.b);
      txt       =4*((cfg.black.r<<16)|(cfg.black.g<<8)|cfg.black.b);
    }

  if(has_mouse)
    {
    int scrncol=light;
    
    switch(vga_getcolors())
      {
      case 32768:
        scrncol=GET15BITCOLOUR(4*cfg.light.r,4*cfg.light.g,4*cfg.light.b);
        break;
      case 65536:
        scrncol=GET16BITCOLOUR(4*cfg.light.r,4*cfg.light.g,4*cfg.light.b);
        break;
      }

    mousecur_init(mouseblack,scrncol);
    restore_mouse_pos();
    }
  }

set_max_text_width(vga_getxdim()-70-
                   fdlabelwidth*(replytype==MSGBOXTYPE_FILEDETAILS));

high=70;
if(replytype!=MSGBOXTYPE_FILEDETAILS)
  wide=vgatextsize(3,message);
else
  {
  int f,num,len,maxlen;
  
  /* make the box a bit taller */
  high+=210;
  
  for(f=0;f<7;f++) fdetailstr[f]=" ";
  
  /* split the message string into seven lines, and set width
   * to widest of them. String length doesn't mean anything;
   * it's the onscreen width we care about here.
   */
  num=0;
  while((ptr=strrchr(message,'\n'))!=NULL)
    *ptr=0,num++;

  ptr=message;
  if(num>7) num=7;
  maxlen=0;
  for(f=0;f<num;f++)
    {
    len=vgatextsize(3,ptr);
    if(len>maxlen) maxlen=len;
    fdetailstr[f]=ptr;
    ptr+=strlen(ptr)+1;
    }
  
  /* ok, we also want some room for the labels :-) */
  wide=maxlen+fdlabelwidth;
  }

wide+=40;

x1=((vga_getxdim()-wide)>>1);
y1=((vga_getydim()-high)>>1);
x2=((vga_getxdim()+wide)>>1);
y2=((vga_getydim()+high)>>1);

if(save_and_restore)
  save_area(x1,y1,x2,y2);

blank_area(x1,y1,x2,y2,idx_medium);

/* draw outer box */
drawbutton(x1,y1,x2,y2,NULL,0,light,dark,mouseblack,0);

/* finally, I've got around to doing different types of msgbox! */
switch(replytype)
  {
  /* a box with a single 'OK' button, for warnings, errors, etc. */
  case MSGBOXTYPE_OK:
    drawbutton(but1.x1=((vga_getxdim()-40)>>1),but1.y1=y2-35,
               but1.x2=((vga_getxdim()+40)>>1),but1.y2=y2-10,
               "Ok",1,light,dark,mouseblack,txt);

    setcolour(txt);
    vgadrawtext(x1+20,y1+10,3,message);
    set_max_text_width(NO_CLIP_FONT);

    do
      {
      key=mousecur_wait_for_keys_or_mouse(ttyfd);
      if(has_mouse && is_end_click_left())
        {
        int mx=mouse_getx(),my=mouse_gety();
        if(mx>=but1.x1 && mx<=but1.x2 && my>=but1.y1 && my<=but1.y2)
          key=RK_ENTER;
        }
      }
    while(key!=RK_ESC && key!=RK_ENTER);
    
    retval=1;
    break;

  /* a special case for the file-details dialog. Already have 7
   * strings parsed from message. The layout is based on xzgv's.
   */
  case MSGBOXTYPE_FILEDETAILS:
    {
    static char *desc1="Details from OS";
    static char *desc2="Details from thumbnail";
    static char *desc2_pic="Details from picture";
    int group1_x,group1_y;
    int group2_x,group2_y;
    int f,y,len;
    int dim_from_pic=(fdetailstr[6][0]=='p');
    int got_dim=(fdetailstr[6][1]!='0');

    fdetailstr[6]++;
    
    drawbutton(but1.x1=((vga_getxdim()-40)>>1),but1.y1=y2-35,
               but1.x2=((vga_getxdim()+40)>>1),but1.y2=y2-10,
               "Ok",1,light,dark,mouseblack,txt);

#define GROUP_BOX(x1,y1,x2,y2,desc,desc_col) \
    draw3dbox(x1,y1,x2,y2,1,0,light,dark); \
    draw3dbox(x1+1,y1+1,x2-1,y2-1,1,1,light,dark); \
    setcolour(idx_medium); \
    len=vgatextsize(2,desc); \
    for(f=0;f<2;f++) vga_drawline(x1+5,y1+f,x1+5+10+len,y1+f); \
    setcolour(desc_col); \
    vgadrawtext(x1+5+5,y1-4,2,desc);

    group1_x=x1+10; group1_y=y1+15;
    group2_x=group1_x; group2_y=but1.y1-52;
    GROUP_BOX(group1_x,group1_y,x2-11,group2_y-15,desc1,txt);
    GROUP_BOX(group2_x,group2_y,x2-11,but1.y1-10,
              dim_from_pic?desc2_pic:desc2,
              got_dim?txt:dark);

    y=group1_y+15;
    for(f=0;f<7;f++,y+=24)
      {
      setcolour(txt);
      if(f==6)
        {
        y=group2_y+15;
        if(!got_dim)
          {
          setcolour(dark);
          fdetailstr[f]="unknown";
          }
        }

      vgadrawtext(group1_x+fdlabelwidth-10-vgatextsize(3,fdlabels[f]),y,
                  3,fdlabels[f]);
      vgadrawtext(group1_x+fdlabelwidth,y,3,fdetailstr[f]);
      }
    }

    set_max_text_width(NO_CLIP_FONT);
    
    do
      {
      key=mousecur_wait_for_keys_or_mouse(ttyfd);
      if(has_mouse && is_end_click_left())
        {
        int mx=mouse_getx(),my=mouse_gety();
        if(mx>=but1.x1 && mx<=but1.x2 && my>=but1.y1 && my<=but1.y2)
          key=RK_ENTER;
        }
      }
    while(key!=RK_ESC && key!=RK_ENTER);
    
    retval=1;
    break;

  /* a box with two buttons, 'Yes' and 'No'. Enter or 'y' selects yes,
   * Esc or 'n' selects no.
   */    
  case MSGBOXTYPE_YESNO:
    drawbutton(but1.x1=(vga_getxdim()>>1)-50,but1.y1=y2-35,
               but1.x2=(vga_getxdim()>>1)-10,but1.y2=y2-10,
               "Yes",1,light,dark,mouseblack,txt);
    drawbutton(but2.x1=(vga_getxdim()>>1)+10,but2.y1=y2-35,
               but2.x2=(vga_getxdim()>>1)+50,but2.y2=y2-10,
               "No",1,light,dark,mouseblack,txt);
    
    setcolour(txt);
    vgadrawtext(x1+20,y1+10,3,message);		/* draw message */
    set_max_text_width(NO_CLIP_FONT);
    
    do
      {
      key=mousecur_wait_for_keys_or_mouse(ttyfd);
      if(has_mouse && is_end_click_left())
        {
        int mx=mouse_getx(),my=mouse_gety();
        if(mx>=but1.x1 && mx<=but1.x2 && my>=but1.y1 && my<=but1.y2)
          key='y';
        if(mx>=but2.x1 && mx<=but2.x2 && my>=but2.y1 && my<=but2.y2)
          key='n';
        }
      }
    while(key!=RK_ESC && key!=RK_ENTER && key!='y' && key!='n');

    retval=(key==RK_ENTER || key=='y');
    break;

  default:
    retval=0;
  }

if(save_and_restore)
  {
  restore_area();
  if(savedpal)
    vga_setpalvec(0,256,palsav);
  last_msgbox_did_restore=1;
  }

if(!msgbox_draw_ok && has_mouse)
  save_mouse_pos();

return(retval);
}


/* get line of input (presumed to be a directory, though this is
 * not checked). Expands `~' to $HOME.
 */
char *cm_getline(int ttyfd,char *prompt,int light,int dark,int txt,int med)
{
static char buf[256],*retptr=NULL;
char *homeptr;
int pos,c,tmp;
int x1,y1,x2,y2,boxwidth,boxheight;
int insert=1;
int maxlen=sizeof(buf)-1;
int firsttime=1;
int curofs=0;

if(retptr!=NULL && retptr!=buf)
  {
  free(retptr);
  retptr=NULL;
  }

pos=0; *buf=0;

boxwidth=620;
boxheight=90;
x1=((vga_getxdim()-boxwidth)>>1);
y1=((vga_getydim()-boxheight)>>1);
x2=((vga_getxdim()+boxwidth)>>1);
y2=((vga_getydim()+boxheight)>>1);

save_area(x1,y1,x2,y2);

blank_area(x1,y1,x2,y2,med);

/* draw outer box */
drawbutton(x1,y1,x2,y2,NULL,0,light,dark,idx_blacknz,0);
draw3dbox(x1+9,y2-46,x2-9,y2-9,1,0,light,dark);

/* draw prompt */
setcolour(txt);
vgadrawtext(x1+30,y1+15,3,prompt);

set_max_text_width(560);

do
  {
  if(firsttime)
    c=RK_NO_KEY;
  else
    do
      {
      c=mousecur_wait_for_keys_or_mouse(ttyfd);
      }
    while(c==RK_NO_KEY);
  
  setcolour(med);
  tmp=vgadrawtext(x1+30,y2-35,3,buf);
  vgadrawtext(x1+30+curofs,y2-35,3,"_");
  
  if(firsttime)
    firsttime=0;
  else
    switch(c)
      {
      case 'b'-0x60: case RK_CURSOR_LEFT:  if(pos>0) pos--; break;
      case 'f'-0x60: case RK_CURSOR_RIGHT: if(pos<strlen(buf)) pos++; break;
      case 'a'-0x60: case RK_HOME:	pos=0; break;
      case 'e'-0x60: case RK_END:	pos=strlen(buf); break;
      case 'i'-0x60: case RK_INSERT:	insert=!insert; break;
      case 'd'-0x60: case RK_DELETE:
        /* delete forwards */
        if(pos<strlen(buf)) memmove(buf+pos,buf+pos+1,strlen(buf)-pos);
        break;
      case 8: case 127:
        /* delete backwards */
        if(pos>0)
          {
          memmove(buf+pos-1,buf+pos,strlen(buf+pos)+1);
          pos--;
          }
        break;
      
      default:
        /* if printable, insert it (or overwrite) */
        if(c>=32 && c<127)
          {
          if(insert)
            {
            if(strlen(buf)<maxlen)
              {
              if(pos<=strlen(buf))
                memmove(buf+pos+1,buf+pos,strlen(buf+pos)+1);
              buf[pos++]=c;
              }
            }
          else
            {
            if(strlen(buf)<maxlen || pos<strlen(buf))
              {
              if(pos>=strlen(buf))
                buf[pos+1]=0;
              
              buf[pos++]=c;
              }
            }
          }
      }
  
  /* draw text and cursor */
  setcolour(txt);
  tmp=buf[pos];
  buf[pos]=0;
  curofs=vgatextsize(3,buf);
  buf[pos]=tmp;
  vgadrawtext(x1+30,y2-35,3,buf);
  vgadrawtext(x1+30+curofs,y2-35,3,"_");
  }
while(c!=RK_ENTER && c!=RK_ESC);

set_max_text_width(NO_CLIP_FONT);

restore_area();

if(c==RK_ESC) return(NULL);

/* expand tilde if needed; only dealt with if it's first char,
 * and the ~user form isn't supported.
 */
if(*buf!='~') return(buf);

homeptr=getenv("HOME");
if(homeptr==NULL) homeptr=buf+strlen(buf);	/* point at a NUL */

/* we want length of homeptr string, plus length of existing string
 * minus the first char (the tilde), plus space for the NUL.
 */

if((retptr=malloc(strlen(homeptr)+strlen(buf)))==NULL)
  return(NULL);

/* the memory will be freed on any subsequent call to this routine */

strcpy(retptr,homeptr);
strcat(retptr,buf+1);

return(retptr);
}
