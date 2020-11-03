/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        LibUnix.h                        **/
/**                                                         **/
/** This file contains definitions and declarations for     **/
/** some commonly used Unix/X11 routines.                   **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2003                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef LIBUNIX_H
#define LIBUNIX_H
#ifdef UNIX

#include <X11/Xlib.h>

#ifdef MITSHM
#include <X11/extensions/XShm.h>
#endif

#define MAXTIMERFREQ 100 /* Maximal frequency for AddTimer() */
#define USE_SHM      0x0001 /* Use MITSHM extension          */
#define MAXHISTORY   256
#define MAXCOLORS    256

#ifndef PIXEL_TYPE_DEFINED
#define PIXEL_TYPE_DEFINED
#ifdef BPP32
typedef unsigned int pixel;
#else
#ifdef BPP24
typedef unsigned int pixel;
#else
#ifdef BPP16
typedef unsigned short pixel;
#else
#ifdef BPP8
typedef unsigned char pixel;
#endif
#endif
#endif
#endif
#endif

/** ARGC/ARGV ************************************************/
/** Assign argc/argv from main() to these variables.        **/
/*************************************************************/
extern int ARGC;
extern char **ARGV;

/** InitLibUnix() ********************************************/
/** This function is called to obtain the display and other **/
/** values at startup. It returns 0 on failure.             **/
/*************************************************************/
int InitLibUnix(Display *AppDisplay);

/** X11Window() **********************************************/
/** Open a window of a given size with a given title.       **/
/*************************************************************/
Window X11Window(const char *Title,int W,int H);

/** Bitmap Images ********************************************/
typedef struct
{
  char *Data;
  int W,H,D,Attrs;
  XImage *XImg;
#ifdef MITSHM
  XShmSegmentInfo SHMInfo;
#endif
} Image;

void X11PutImage(Window Wnd,Image *Img,int DX,int DY,int SX,int SY,int W,int H);
int  X11NewImage(Image *Img,int W,int H,int Attrs);
void X11FreeImage(Image *Img);
int  X11ScaleImage(Image *Dst,Image *Src,int DX,int DY,int SX,int SY,int DW,int DH,int SW,int SH);

/*** View Ports **********************************************/
typedef struct
{
  Window Wnd;
  Image Img,Img1;
  int X,Y,WH,WW,W,H;
} Port;

int  X11NewPort(Port *Prt,char *Title,int W,int H,int Attrs);
void X11FreePort(Port *Prt);
int  X11SetPort(Port *Prt,int X,int Y,int W,int H);
int  X11ZoomPort(Port *Prt,int W,int H);
void X11DrawPort(Port *Prt,int X,int Y,int W,int H);
void X11RefreshPort(Port *Prt);

/** X11SetColor() ********************************************/
/** Allocate a new color. Returns 0 (black) on failure.     **/
/*************************************************************/
unsigned int X11SetColor(unsigned char N,unsigned char R,unsigned char G,unsigned char B);

/** X11FreeColors() ******************************************/
/** Free all allocated colors.                              **/
/*************************************************************/
void X11FreeColors(void);

/** TimerSignal() ********************************************/
/** Establishes signal handler called with given frequency  **/
/** (Hz). Returns 0 if failed.                              **/
/*************************************************************/
int TimerSignal(int Freq,void (*Handler)(int));

/** AddTimer() ***********************************************/
/** Establishes a periodically called routine at a given    **/
/** frequency (1..MAXTIMERFREQ Hz). Returns 0 if failed.    **/
/*************************************************************/
int AddTimer(int Freq,void (*Handler)(void));

/** DelTimer() ***********************************************/
/** Removes timers established with AddTimer().             **/
/*************************************************************/
void DelTimer(void (*Handler)(void));

#endif /* UNIX */
#endif /* LIBUNIX_H */
