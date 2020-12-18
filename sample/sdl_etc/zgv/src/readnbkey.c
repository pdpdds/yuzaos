/* zgv 5.9 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-2005 Russell Marks. See README for license details.
 *
 * readnbkey.c - reads a non-blocking key. Header file provides #defines
 *		 for weirdo keys and such. Also includes a certain amount
 *		 of code for mouse reading.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
//#include <sys/time.h>
#include <sys/types.h>
//#include <sys/ioctl.h>
#include "zgv_io.h"
#include "zgv.h"
#include "readnbkey.h"
#include "mousecur.h"


#ifndef BACKEND_SVGALIB


#ifdef BACKEND_SDL

#include <SDL.h>

#define BACKEND_SDL_OR_SVGALIB

int readnbkey(int ttyfd)
{
return(zgv_io_readnbkey());	/* defined in zgv_io.c */
}

int wait_for_keys_or_mouse(int fd)
{
int key;

if((key = zgv_io_waitevent()) == RK_NO_KEY && has_mouse)
  mouse_update(),click_update();

return(key);
}

#else	/* !BACKEND_SDL */

/* generic redirector for non-svgalib non-SDL backends. Currently there's
 * no provision for mouse support in these.
 */

int readnbkey(int ttyfd)
{
return(zgv_io_readnbkey());	/* defined in zgv_io.c */
}

int wait_for_keys_or_mouse(int fd)
{
return(zgv_io_waitkey());	/* defined in zgv_io.c */
}

int mousecur_wait_for_keys_or_mouse(int fd)
{
return(zgv_io_waitkey());
}

void click_update(void) {}

void save_mouse_pos(void) {}

void restore_mouse_pos(void) {}

void restore_mouse_pos_with_size(int width,int height) {}

int is_end_click_left(void) { return 0; }
int is_end_click_right(void) { return 0; }
int is_start_click_left(void) { return 0; }
int is_start_click_right(void) { return 0; }

#endif	/* !BACKEND_SDL */


#else	/* BACKEND_SVGALIB */


#include <vgakeyboard.h>	/* for SCANCODE_{F1,F11} */
#ifdef OSTYPE_LINUX
#include <linux/kd.h>
#endif
#ifdef OSTYPE_FREEBSD
#include <sys/consio.h>
#include <sys/kbio.h>
#endif


#define BACKEND_SDL_OR_SVGALIB


#ifdef OSTYPE_LINUX

/* historically, some Linux keymaps had F11 and F12 generating the
 * same strings as shift-F1 and shift-F2. (This made things less
 * painful for people using keyboards with only 10 function keys.) The
 * standard UK keymap was like this, for example. However, in the
 * not-so-very-distant past, the standard keymaps were standardised to
 * all (?) have distinct strings for F11 and F12, effectively shifting
 * everything else up, and giving you two more function keys. Great,
 * except this totally broke zgv, which assumed the F11 = shift-F1
 * approach.
 *
 * So here we look for the kind of keymap we have, so we can emulate the
 * old behaviour on `new'-style (logical, i.e. it makes sense :-)) keymaps.
 */
int is_logical_keymap(int ttyfd)
{
struct kbentry ent1,ent2;

/* this is horrible, but I really do need the scancodes to check this :-(
 * as the mapping is done at that level.
 */
ent1.kb_table=K_NORMTAB;
ent1.kb_index=SCANCODE_F11;
ent2.kb_table=K_SHIFTTAB;
ent2.kb_index=SCANCODE_F1;

if(ioctl(ttyfd,KDGKBENT,&ent1) || ioctl(ttyfd,KDGKBENT,&ent2))
  return(0);

if(ent1.kb_value==ent2.kb_value)
  return(0);

return(1);
}

#endif	/* OSTYPE_LINUX */


int getnbkey(int ttyfd)
{
unsigned char c=0;
int ret=read(ttyfd,&c,1);

if(ret==1 && c==0)
  return(RK_CTRLSPACE);

return((int)c);
}


/* these's some duplicated code between the Linux and FreeBSD versions
 * of this routine, but on balance it's probably less confusing this
 * way.
 */

#ifdef OSTYPE_LINUX

/* returns a 'normal' ASCII value, or one of the values in readnbkey.h,
 * or zero if no key was pressed.
 */
int readnbkey(int ttyfd)
{
static unsigned char keybuf[1024];
static int logical_keymap=0,first=1;
int f,left;

/* see comment above is_logical_keymap() for what this is for */
if(first)
  {
  first=0;
  logical_keymap=is_logical_keymap(ttyfd);
  }

/* this gets all the characters sent by the key into an ASCIIZ string.
 * the no-waiting-for-keys approach depends on the way Linux dumps
 * the string into our input all at once; this is kind of nasty,
 * but zgv *is* Linux-specific, right? :-) (errrm...)
 */
/* now only does this if first char is esc, to save losing keys in
 * other situations (e.g. when getting input)
 */
f=0;
for(left=sizeof(keybuf)-1;left>0 && (keybuf[f++]=getnbkey(ttyfd))!=0;left--)
  if(keybuf[0]!=27) break;
keybuf[f]=0;	/* end the string */

/* convert Esc-Esc to Esc, potentially useful if you've mapped Esc to
 * Esc-Esc for convenience with ncurses-based programs as Sergei
 * Ivanov does. Which is probably why he suggested this addition. :-)
 */
if(keybuf[0]==27 && keybuf[1]==27)
  return(27);

/* convert Esc-A..Z and Esc-a..z to Meta-a..z, and other Esc-foo to
 * Meta-foo, by converting any 2-char string starting with Esc.
 * (It also allows the end char to be Esc, to avoid quitting when
 * someone holds a key down in some situations. :-))
 */
if(keybuf[0]==27 && keybuf[1] && (!keybuf[2] || keybuf[2]==27))
  return(128+tolower(keybuf[1]));

/* also convert Meta-A..Z to Meta-a..z. */
if(keybuf[0]>=128+'A' && keybuf[0]<=128+'Z' && keybuf[1]==0)
  return(keybuf[0]+32);


/* Deal with function keys, cursors, and the like.
 * this relies on a minimum-unique string approach rather than
 * checking for the trailing ~ (on function keys) or NUL (on everything :-)).
 */
if(keybuf[0]==27 && keybuf[1]=='[')
  {
  switch(keybuf[2])
    {
    case 'A':	return(RK_CURSOR_UP);
    case 'B':	return(RK_CURSOR_DOWN);
    case 'C':	return(RK_CURSOR_RIGHT);
    case 'D':	return(RK_CURSOR_LEFT);
    case '[':
      switch(keybuf[3])
        {
        case 'A':	return(RK_F1);
        case 'B':	return(RK_F2);
        case 'C':	return(RK_F3);
        case 'D':	return(RK_F4);
        case 'E':	return(RK_F5);
        }
      break;
    case '1':
      switch(keybuf[3])
        {
        case '~':	return(RK_HOME);
        case '7':	return(RK_F6);
        case '8':	return(RK_F7);
        case '9':	return(RK_F8);
        }
      break;
    case '2':
      switch(keybuf[3])
        {
        case '~':	return(RK_INSERT);
        case '0':	return(RK_F9);
        case '1':	return(RK_F10);
        case '3':	return(RK_SHIFT_F1);	/* or F11 */
        case '4':	return(RK_SHIFT_F2);	/* or F12 */
        case '5':	return(logical_keymap?RK_SHIFT_F1:RK_SHIFT_F3);
        case '6':	return(logical_keymap?RK_SHIFT_F2:RK_SHIFT_F4);
        case '8':	return(logical_keymap?RK_SHIFT_F3:RK_SHIFT_F5);
        case '9':	return(logical_keymap?RK_SHIFT_F4:RK_SHIFT_F6);
        }
      break;
    case '3':
      switch(keybuf[3])
        {
        case '~':	return(RK_DELETE);
        case '1':	return(logical_keymap?RK_SHIFT_F5:RK_SHIFT_F7);
        case '2':	return(logical_keymap?RK_SHIFT_F6:RK_SHIFT_F8);
        case '3':	return(logical_keymap?RK_SHIFT_F7:RK_SHIFT_F9);
        case '4':	return(logical_keymap?RK_SHIFT_F8:RK_SHIFT_F10);
        }
      break;
    case '4':	return(RK_END);
    case '5':	return(RK_PAGE_UP);
    case '6':	return(RK_PAGE_DOWN);
    }
  }

/* otherwise... */
return(*keybuf);
}

#endif	/* OSTYPE_LINUX */

#ifdef OSTYPE_FREEBSD

/* version for FreeBSD */
int readnbkey(int ttyfd)
{
static unsigned char keybuf[1024];
int f,left;

/* this gets all the characters sent by the key into an ASCIIZ string.
 * the no-waiting-for-keys approach depends on the string being dumped
 * into our input all at once; this is kind of nasty.
 */
/* now only does this if first char is esc, to save losing keys in
 * other situations (e.g. when getting input)
 */
f=0;
for(left=sizeof(keybuf)-1;left>0 && (keybuf[f++]=getnbkey(ttyfd))!=0;left--)
  if(keybuf[0]!=27) break;
keybuf[f]=0;	/* end the string */

/* convert Esc-Esc to Esc, potentially useful if you've mapped Esc to
 * Esc-Esc for convenience with ncurses-based programs as Sergei
 * Ivanov does. Which is probably why he suggested this addition. :-)
 */
if(keybuf[0]==27 && keybuf[1]==27)
  return(27);

/* convert Esc-A..Z and Esc-a..z to Meta-a..z, and other Esc-foo to
 * Meta-foo, by converting any 2-char string starting with Esc.
 * (It also allows the end char to be Esc, to avoid quitting when
 * someone holds a key down in some situations. :-))
 */
if(keybuf[0]==27 && keybuf[1] && (!keybuf[2] || keybuf[2]==27))
  return(128+tolower(keybuf[1]));

/* also convert Meta-A..Z to Meta-a..z. */
if(keybuf[0]>=128+'A' && keybuf[0]<=128+'Z' && keybuf[1]==0)
  return(keybuf[0]+32);


/* Deal with function keys, cursors, and the like.
 * this relies on a minimum-unique string approach rather than
 * checking for the trailing ~ (on function keys) or NUL (on everything :-)).
 */
if(keybuf[0]==0x7f)
  return(RK_DELETE);

if(keybuf[0]==27 && keybuf[1]=='[')
  {
  switch(keybuf[2])
    {
    case 'A':	return(RK_CURSOR_UP);
    case 'B':	return(RK_CURSOR_DOWN);
    case 'C':	return(RK_CURSOR_RIGHT);
    case 'D':	return(RK_CURSOR_LEFT);
    case 'M':	return(RK_F1);
    case 'N':	return(RK_F2);
    case 'O':	return(RK_F3);
    case 'P':	return(RK_F4);
    case 'Q':	return(RK_F5);
    case 'R':	return(RK_F6);
    case 'S':	return(RK_F7);
    case 'T':	return(RK_F8);
    case 'U':	return(RK_F9);
    case 'V':	return(RK_F10);
    case 'W':	return(RK_F11);
    case 'X':	return(RK_F12);
    case 'H':	return(RK_HOME);
    case 'F':	return(RK_END);
    case 'I':	return(RK_PAGE_UP);
    case 'G':	return(RK_PAGE_DOWN);
    case 'L':	return(RK_INSERT);
    case 'Y':	return(RK_SHIFT_F1);
    case 'Z':	return(RK_SHIFT_F2);
    case 'a':	return(RK_SHIFT_F3);
    case 'b':	return(RK_SHIFT_F4);
    case 'c':	return(RK_SHIFT_F5);
    case 'd':	return(RK_SHIFT_F6);
    case 'e':	return(RK_SHIFT_F7);
    case 'f':	return(RK_SHIFT_F8);
    case 'g':	return(RK_SHIFT_F9);
    case 'h':	return(RK_SHIFT_F10);
    
    /* XXX it seems RK_SHIFT_F11 and RK_SHIFT_F12 cannot be detected... */
    }
  }

/* otherwise... */
return(*keybuf);
}

#endif	/* OSTYPE_FREEBSD */


/* XXX vgadisp.c expects any SIGALRM to interrupt the vga_waitevent()
 * call. It's possible to avoid this assumption, but I'd rather not unless
 * I have to. And since I don't, I don't. :-)
 */
int wait_for_keys_or_mouse(int fd)
{
fd_set kybd;
int res;

FD_ZERO(&kybd);
FD_SET(fd,&kybd);
/* if mouse data is ready this runs mouse_update itself. */
res=vga_waitevent(has_mouse?VGA_MOUSEEVENT:0,&kybd,NULL,NULL,NULL);
if(FD_ISSET(fd,&kybd))
  return(readnbkey(fd));

if(has_mouse)
  mouse_update(),click_update();
return(0);
}


#endif	/* BACKEND_SVGALIB */


#ifdef BACKEND_SDL_OR_SVGALIB


/* saved mx/my in in range 0..MPOS_SAVE_MAX. */
#define MPOS_SAVE_MAX	16383

static int old_click_status=0,new_click_status=0;


/* update previous-button-state, so the is..click routines work.
 * called by keys_or_mouse routines below, but must be called after
 * mouse_update() if that's not used.
 */
void click_update()
{
old_click_status=new_click_status;
new_click_status=mouse_getbutton();
}


/* as wait_for_keys_or_mouse, but draw/undraw mouse cursor too */
int mousecur_wait_for_keys_or_mouse(int fd)
{
int key;

mousecur_on();	/* draw */
key=wait_for_keys_or_mouse(fd);
mousecur_off();	/* undraw */
return(key);
}


static int saved_mx=0,saved_my=0;

/* save mouse pointer's current position. used when switching over
 * to the panning-follows-mouse model used by vgadisp.c, so that on
 * returning to the file selector (or whatever) we have a sane-ish
 * mouse position.
 *
 * The position is saved/restored in the range 0..MPOS_SAVE_MAX
 * for both x and y, with scaling done to match current video mode.
 */
void save_mouse_pos()
{
int mx,my;

if(!has_mouse) return;

/* if we're trying to save in text mode (happens with `-p'), save as if
 * it was in the middle of the screen.
 */
if(vga_getcurrentmode()==TEXT)
  {
  mx=MPOS_SAVE_MAX/2;
  my=MPOS_SAVE_MAX/2;
  }
else
  {
  mx=(mouse_getx()*MPOS_SAVE_MAX)/(vga_getxdim()-1);
  my=(mouse_gety()*MPOS_SAVE_MAX)/(vga_getydim()-1);
  }
/* just in case */
if(mx>MPOS_SAVE_MAX) mx=MPOS_SAVE_MAX;
if(my>MPOS_SAVE_MAX) my=MPOS_SAVE_MAX;
saved_mx=mx;
saved_my=my;
}


/* restores saved pos, also blanks out click stuff */
void restore_mouse_pos(void)
{
restore_mouse_pos_with_size(vga_getxdim(),vga_getydim());
}


/* restore saved pos assuming given size screen */
void restore_mouse_pos_with_size(int width,int height)
{
int mx,my;

if(!has_mouse) return;

mx=(saved_mx*(width-1))/MPOS_SAVE_MAX;
my=(saved_my*(height-1))/MPOS_SAVE_MAX;
/* just in case */
if(mx>width-1) mx=width-1;
if(my>height-1) my=height-1;

mouse_setposition(mx,my);
old_click_status=new_click_status=0;
}


/* normally you would use is_end_click.. when a button should do
 * something when pressed, to simplify matters. is_start_click..
 * should really be reserved for things like dragging where knowing
 * the button has been pressed (and is still pressed) is important.
 */

/* returns 1 if left button was pressed but isn't now, else 0 */
int is_end_click_left()
{
if((old_click_status&MOUSE_LEFTBUTTON) &&
   !(new_click_status&MOUSE_LEFTBUTTON))
  {
  /* remove the bit, since there won't necessarily be any more events
   * to change it, which could screw things up big time!
   */
  old_click_status&=~MOUSE_LEFTBUTTON;
  return(1);
  }
return(0);
}


/* returns 1 if right button was pressed but isn't now, else 0 */
int is_end_click_right()
{
if((old_click_status&MOUSE_RIGHTBUTTON) &&
   !(new_click_status&MOUSE_RIGHTBUTTON))
  {
  old_click_status&=~MOUSE_RIGHTBUTTON;
  return(1);
  }
return(0);
}


/* returns 1 if left button wasn't pressed but is now, else 0.
 * NB: this really tests `has button been pressed since last update,
 * with no more mouse events since?', so beware.
 */
int is_start_click_left()
{
if(!(old_click_status&MOUSE_LEFTBUTTON) &&
   (new_click_status&MOUSE_LEFTBUTTON))
  return(1);
return(0);
}


/* returns 1 if right button wasn't pressed but is now, else 0 */
int is_start_click_right()
{
if(!(old_click_status&MOUSE_RIGHTBUTTON) &&
   (new_click_status&MOUSE_RIGHTBUTTON))
  return(1);
return(0);
}


#endif	/* BACKEND_SDL_OR_SVGALIB */
