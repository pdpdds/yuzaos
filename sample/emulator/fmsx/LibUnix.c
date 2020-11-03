/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                        LibUnix.c                        **/
/**                                                         **/
/** This file contains implementation for some commonly     **/
/** used Unix/X11 routines.                                 **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2003                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifdef UNIX

#include "LibUnix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

struct Timer
{
  struct Timer *Next;
  int Freq;
  void (*Handler)(void);
};

static Display *Dsp      = NULL;
static Screen *Scr       = NULL;
static Colormap CMap;

static struct { unsigned int Pen,RGB; } Colors[MAXCOLORS];
static unsigned long CHPens[MAXHISTORY];
static unsigned int CHRGB[MAXHISTORY];
static int CHCount;

struct Timer *FirstTimer = NULL;
static int TimerLock     = 0;

static void MasterTimer(int Arg);

/** ARGC/ARGV ************************************************/
/** Assign argc/argv from main() to these variables.        **/
/*************************************************************/
int ARGC    = 0;
char **ARGV = NULL;

/** InitLibUnix() ********************************************/
/** This function is called to obtain the display and other **/
/** values at startup. It returns 0 on failure.             **/
/*************************************************************/
int InitLibUnix(Display *AppDisplay)
{
  int J;

  /* X11 display */
  Dsp=AppDisplay;
  Scr=DefaultScreenOfDisplay(Dsp);
  CMap=DefaultColormapOfScreen(Scr);

  /* Color allocation */
  CHCount=0;
  for(J=0;J<MAXCOLORS;J++)
  {
    Colors[J].Pen = BlackPixelOfScreen(Scr);
    Colors[J].RGB = 0;
  }

  /* Timers */  
  FirstTimer=NULL;
  TimerLock=0;

  return(1);
}

/** MasterTimer() ********************************************/
/** The main timer handler which is called MAXTIMERFREQ     **/
/** times a second. It then calls user-defined timers.      **/
/*************************************************************/
static void MasterTimer(int Arg)
{
  static unsigned int Counter=0;
  register struct Timer *P;

  if(!TimerLock)
    for(P=FirstTimer;P;P=P->Next)
      if(!(Counter%P->Freq)) (P->Handler)();

  Counter++;
  signal(Arg,MasterTimer);
}

/** X11PutImage() ********************************************/
/** Copy image to a window, centering it.                   **/
/*************************************************************/
void X11PutImage(Window Wnd,Image *Img,int DX,int DY,int SX,int SY,int W,int H)
{
  /* Need to initalize library first */
  if(!Dsp) return;
  
  /* If image not initialized, fall out */
  if(!Img->Data) return;
  
  /* If incorrect parameters, fall out */
  if((SX+W>Img->W)||(SY+H>Img->H)) return;

  /* Wait for all X11 requests to complete */
  XSync(Dsp,False);

#ifdef MITSHM
  if(Img->Attrs&USE_SHM)
    XShmPutImage
    (
      Dsp,Wnd,DefaultGCOfScreen(Scr),Img->XImg,
      SX,SY,DX,DY,W,H,False
    );
  else
#endif
    XPutImage
    (
      Dsp,Wnd,DefaultGCOfScreen(Scr),Img->XImg,
      SX,SY,DX,DY,W,H
    );
}

/** X11NewImage() ********************************************/
/** Build a new Image structure of given dimensions.        **/
/*************************************************************/
int X11NewImage(Image *Img,int W,int H,int Attrs)
{
  XVisualInfo VInfo;
  int Depth,J,I;

  /* Set data fields to ther defaults */
  Img->Data=NULL;Img->W=Img->H=Img->D=Img->Attrs=0;

  /* Need to initalize library first */
  if(!Dsp) return(0);

  /* Image depth we are going to use */
#ifdef BPP32
  Depth=32;
#else
#ifdef BPP24
  Depth=24;
#else
#ifdef BPP16
  Depth=16;
#else
#ifdef BPP8
  Depth=8;
#else
  Depth=DefaultDepthOfScreen(Scr);
#endif
#endif
#endif
#endif

  /* Get appropriate Visual */
  I=XScreenNumberOfScreen(Scr);
  for(J=7;J>=0;J--)
    if(XMatchVisualInfo(Dsp,I,Depth,J,&VInfo)) break;
  if(J<0) return(0);

#ifdef MITSHM
  if(Attrs&USE_SHM)
  {
    Img->XImg =
      XShmCreateImage
      (Dsp,VInfo.visual,Depth,ZPixmap,NULL,&Img->SHMInfo,W,H);
    if(!Img->XImg) return(0);

    Img->SHMInfo.shmid =
      shmget
      (IPC_PRIVATE,Img->XImg->bytes_per_line*Img->XImg->height,IPC_CREAT|0777);
    if(Img->SHMInfo.shmid==-1) return(0);

    Img->XImg->data=Img->SHMInfo.shmaddr=shmat(Img->SHMInfo.shmid,0,0);
    if(!Img->XImg->data)
    {
      shmctl(Img->SHMInfo.shmid,IPC_RMID,0);
      return(0);
    }

    /* Can write into shared segment */
    Img->SHMInfo.readOnly=False;

    /* Attach segment to X display and make sure it is done */
    J=XShmAttach(Dsp,&Img->SHMInfo);
    XSync(Dsp,False);

    /* We do not need an ID any longer */
    shmctl(Img->SHMInfo.shmid,IPC_RMID,0);

    /* If attachment failed, break out */
    if(!J) return(0);
  }
  else
#endif
  {
    Img->XImg =
      XCreateImage
      (Dsp,VInfo.visual,Depth,ZPixmap,0,NULL,W,H,Depth,0);
    if(!Img->XImg) return(0);

    Img->XImg->data=(char *)malloc(Img->XImg->bytes_per_line*Img->XImg->height);
    if(!Img->XImg->data) return(0);
  }

  Img->Data=Img->XImg->data;
  Img->W=W;
  Img->H=H;
  Img->D=Depth;
  Img->Attrs=Attrs;
  return(1);
}

/** X11FreeImage() *******************************************/
/** Free Image structure allocated by X11NewImage.          **/
/*************************************************************/
void X11FreeImage(Image *Img)
{
  /* Need to initalize library first */
  if(!Dsp) return;

#ifdef MITSHM
  if(Img->Attrs&USE_SHM)
    if(Img->SHMInfo.shmaddr)
    { XShmDetach(Dsp,&Img->SHMInfo);shmdt(Img->SHMInfo.shmaddr); }
#endif

  if(Img->XImg) XDestroyImage(Img->XImg);
}

/** X11ScaleImage() ******************************************/
/** Copy Src image into Dst image with scaling.             **/
/*************************************************************/
int X11ScaleImage(Image *Dst,Image *Src,int DX,int DY,int SX,int SY,int DW,int DH,int SW,int SH)
{
  register int X,U,XRatio,Y,V,YRatio;
  register int DS,SS;

  /* Can't scale images with different depths */
  if(Src->D!=Dst->D) return(0);

  /* Compute ratios */
  XRatio=(SW<<16)/DW;
  YRatio=(SH<<16)/DH;

  /* Compute line widths */
  DS=Dst->XImg->bytes_per_line;
  SS=Src->XImg->bytes_per_line;

  /* Different pointers used depending on depth */
  switch(Dst->D)
  {
    case 8:
      {
        register unsigned char *DP,*SP;  

        DP=(unsigned char *)Dst->Data+DY*DS+DX;
        for(Y=V=0;Y<DH;Y++)
        {
          SP=(unsigned char *)Src->Data+(SY+(V>>16))*SS+SX;
          for(X=U=0;X<DW;X++)
          {
            DP[X]=SP[U>>16];
            U+=XRatio;
          }
          DP+=Dst->W;
          V+=YRatio;
        }
      }
      return(1);

    case 16:
      {
        register unsigned short *DP,*SP;  

        DP=(unsigned short *)(Dst->Data+DY*DS)+DX;
        for(Y=V=0;Y<DH;Y++)
        {
          SP=(unsigned short *)(Src->Data+(SY+(V>>16))*SS)+SX;
          for(X=U=0;X<DW;X++)
          {
            DP[X]=SP[U>>16];
            U+=XRatio;
          }
          DP=(unsigned short *)((char *)DP+DS);
          V+=YRatio;
        }
      }
      return(1);

    case 24:
    case 32:
      {
        register unsigned int *DP,*SP;  

        DP=(unsigned int *)(Dst->Data+DY*DS)+DX;
        for(Y=V=0;Y<DH;Y++)
        {
          SP=(unsigned int *)(Src->Data+(SY+(V>>16))*SS)+SX;
          for(X=U=0;X<DW;X++)
          {
            DP[X]=SP[U>>16];
            U+=XRatio;
          }
          DP=(unsigned int *)((char *)DP+DS);
          V+=YRatio;
        }
      }
      return(1);
  }

  /* Incorrect depth */
  return(0);
}

/** X11Window() **********************************************/
/** Open a window of a given size with a given title.       **/
/*************************************************************/
Window X11Window(const char *Title,int W,int H)
{
  XSetWindowAttributes Attrs;
  XClassHint ClassHint;
  XSizeHints Hints;
  XWMHints WMHints;
  Window Wnd;
  char *P;
  int Q;

  /* Need to initalize library first */
  if(!Dsp) return(0);

  /* Set necessary attributes */
  Attrs.event_mask =
    FocusChangeMask|KeyPressMask|KeyReleaseMask|StructureNotifyMask;

  Attrs.background_pixel=BlackPixelOfScreen(Scr);
  Attrs.backing_store=Always;

  /* Create a window */
  Wnd=XCreateWindow
      (
        Dsp,RootWindowOfScreen(Scr),0,0,W,H,1,
        CopyFromParent,CopyFromParent,CopyFromParent,
        CWBackPixel|CWEventMask|CWBackingStore,&Attrs
      );
  if(!Wnd) return(0);

  /* Set application class hint */
  if(ARGC&&ARGV)
  {
    P=strrchr(ARGV[0],'/');
    ClassHint.res_name  = P? P+1:ARGV[0];
    ClassHint.res_class = P? P+1:ARGV[0];
    XSetClassHint(Dsp,Wnd,&ClassHint);
    XSetCommand(Dsp,Wnd,ARGV,ARGC);
  }

  /* Set hints */
  Q=sizeof(long);
  Hints.flags          = PSize|PMinSize|PMaxSize|PResizeInc;
  Hints.min_width      = ((W/4)/Q)*Q;
  Hints.max_width      = ((W*4)/Q)*Q;
  Hints.base_width     = (W/Q)*Q;
  Hints.width_inc      = Q;
  Hints.min_height     = ((H/4)/Q)*Q;
  Hints.max_height     = ((H*4)/Q)*Q;
  Hints.base_height    = (H/Q)*Q;
  Hints.height_inc     = Q;
  WMHints.input        = True;
  WMHints.flags        = InputHint;

  if(ARGC&&ARGV)
  {
    WMHints.window_group=Wnd;
    WMHints.flags|=WindowGroupHint;
  }

  /* Set hints, title, size */
  XSetWMHints(Dsp,Wnd,&WMHints);
  XSetWMNormalHints(Dsp,Wnd,&Hints);
  XStoreName(Dsp,Wnd,Title);

  /* Do additional housekeeping and return */
  XMapRaised(Dsp,Wnd);
  XClearWindow(Dsp,Wnd);

  /* Done */
  return(Wnd);
}

/** X11SetColor() ********************************************/
/** Allocate a new color. Returns 0 (black color) on        **/
/** failure.                                                **/
/*************************************************************/
unsigned int X11SetColor(unsigned char N,unsigned char R,unsigned char G,unsigned char B)
{
  register unsigned int RGB,New;
  register int J;
  XColor Color;

  RGB=((int)R<<16)|((int)G<<8)|B;

  /* If the same color, do not do anything */
  if(RGB==Colors[N].RGB) return(Colors[N].Pen);

  /* To exchange with history, old color should be valid (not BLACK) */
  if(RGB&&Colors[N].RGB)
  {
    /* See if we got needed color in the history */
    for(J=CHCount-1;J>=0;J--)
      if(RGB==CHRGB[J]) break;

    /* If found color in history... */
    if(J>=0)
    {
      /* Exchange it with the present value */
      New=CHPens[J];
      CHPens[J] = Colors[N].Pen;
      CHRGB[J]  = Colors[N].RGB;
      Colors[N].Pen = New;
      Colors[N].RGB = RGB;
      return(New);
    }
  }

  /* Dispose of the old color */
  if(Colors[N].RGB)
  {
    /* Dump color history if full */
    if(CHCount>=MAXHISTORY)
    {
      XFreeColors(Dsp,CMap,CHPens,MAXHISTORY,0);
      CHCount=0;
    }      

    /* Move old color to the color history */
    CHPens[CHCount] = Colors[N].Pen;
    CHRGB[CHCount]  = Colors[N].RGB;
    CHCount++;
  }

  /* Allocate new color */
  if(!RGB) New=BlackPixelOfScreen(Scr);
  else
  {
    Color.flags = DoRed|DoGreen|DoBlue;
    Color.red   = (int)R<<8;
    Color.green = (int)G<<8;
    Color.blue  = (int)B<<8;
    if(XAllocColor(Dsp,CMap,&Color)) New=Color.pixel;
    else { New=BlackPixelOfScreen(Scr);RGB=0; }
  }

  Colors[N].RGB = RGB;
  Colors[N].Pen = New;
  return(New);
}

/** X11FreeColors() ******************************************/
/** Free all allocated colors.                              **/
/*************************************************************/
void X11FreeColors(void)
{
  register int J,I;
  unsigned long C;

  if(CHCount)
  {
    XFreeColors(Dsp,CMap,CHPens,CHCount,0);
    CHCount=0;
  }

  for(J=0;J<MAXCOLORS;J++)
    if(Colors[J].RGB)
    {
      for(I=J+1;(I<MAXCOLORS)&&(Colors[J].RGB!=Colors[I].RGB);I++);
      if(I==MAXCOLORS)
      {
        C=Colors[J].Pen;
        XFreeColors(Dsp,CMap,&C,1,0);
        Colors[J].Pen = BlackPixelOfScreen(Scr);
        Colors[J].RGB = 0;
      }
    }
}

/** TimerSignal() ********************************************/
/** Establish a signal handler called with given frequency  **/
/** (Hz). Returns 0 if failed.                              **/
/*************************************************************/
int TimerSignal(int Freq,void (*Handler)(int))
{
  struct itimerval TimerValue;

  TimerValue.it_interval.tv_sec  =
  TimerValue.it_value.tv_sec     = 0;
  TimerValue.it_interval.tv_usec =
  TimerValue.it_value.tv_usec    = 1000000L/Freq;
  if(setitimer(ITIMER_REAL,&TimerValue,NULL)) return(0);
  signal(SIGALRM,Handler);
  return(1);
}

/** AddTimer() ***********************************************/
/** Establish a periodically called routine at a given      **/
/** frequency (1..MAXTIMERFREQ Hz). Returns 0 if failed.    **/
/*************************************************************/
int AddTimer(int Freq,void (*Handler)(void))
{
  struct Timer *P;

  /* Freq must be 1..MAXTIMERFREQ */
  if((Freq<1)||(Freq>MAXTIMERFREQ)) return(0);

  /* Lock the timer queue */
  TimerLock=1;

  /* Look if this timer is already installed */
  for(P=FirstTimer;P;P=P->Next)
    if(P->Handler==Handler)
    { P->Freq=Freq;TimerLock=0;return(1); }

  /* Make up a new one if not */
  if(FirstTimer)
  {
    for(P=FirstTimer;P->Next;P=P->Next);
    P->Next=(struct Timer *)malloc(sizeof(struct Timer));
    P=P->Next;
  }
  else
  {
    /* Allocate the first timer */
    FirstTimer=(struct Timer *)malloc(sizeof(struct Timer));
    P=FirstTimer;

    /* Start up the main handler */
    if(P)
      if(!TimerSignal(MAXTIMERFREQ,MasterTimer))
      { free(P);P=NULL; }
  }

  /* Set up the timer and exit */
  if(P) { P->Next=0;P->Handler=Handler;P->Freq=Freq; }
  TimerLock=0;
  return(P!=0);
}

/** DelTimer() ***********************************************/
/** Remove routine added with AddTimer().                   **/
/*************************************************************/
void DelTimer(void (*Handler)(void))
{
  struct itimerval TimerValue;
  struct Timer *P,*T;

  TimerLock=1;

  if(FirstTimer)
  {
    if(FirstTimer->Handler==Handler)
    {
      /* Delete first timer and free the space */
      P=FirstTimer;FirstTimer=P->Next;free(P);

      /* If no timers left, stop the main handler */
      if(!FirstTimer)
      {
         TimerValue.it_interval.tv_sec  =
         TimerValue.it_value.tv_sec     =
         TimerValue.it_interval.tv_usec =
         TimerValue.it_value.tv_usec    = 0;
         setitimer(ITIMER_REAL,&TimerValue,NULL);
         signal(SIGALRM,SIG_DFL);
      }
    }
    else
    {
      /* Look for our timer */
      for(P=FirstTimer;P;P=P->Next)
        if(P->Next->Handler==Handler) break;

      /* Delete the timer and free the space */
      if(P) { T=P->Next;P->Next=T->Next;free(T); }
    }
  }

  TimerLock=0;
}


int X11NewPort(Port *Prt,char *Title,int W,int H,int Attrs)
{
  /* Set viewport parameters */
  Prt->W=Prt->WW=W;
  Prt->H=Prt->WH=H;
  Prt->X=0;
  Prt->Y=0;

  /* Open window */
  Prt->Wnd=X11Window(Title,W,H);
  if(!Prt->Wnd) return(0);

  /* Allocate main image */
  if(!X11NewImage(&Prt->Img,W,H,Attrs))
  {
    XDestroyWindow(Dsp,Prt->Wnd);
    return(0);
  }

  /* Success */
  return(1);
}

void X11RefreshPort(Port *Prt)
{ X11DrawPort(Prt,Prt->X,Prt->Y,Prt->W,Prt->H); }

void X11DrawPort(Port *Prt,int X,int Y,int W,int H)
{
  int CX,CY,CW,CH;

  /* Clip coordinates by the viewport window */
  CX=X-Prt->X;
  CY=Y-Prt->Y;
  if(CX<0) { W-=Prt->X-CX;CX=0;X=Prt->X; }
  if(CY<0) { H-=Prt->Y-CY;CY=0;Y=Prt->Y; }
  if(CX+W>Prt->X+Prt->W) W=Prt->W-CX;
  if(CY+H>Prt->Y+Prt->H) H=Prt->H-CY;

  /* Drop out if clipped out entirely */
  if((W<=0)||(H<=0)) return;

  if((Prt->W==Prt->WW)&&(Prt->H==Prt->WH))
    X11PutImage(Prt->Wnd,&Prt->Img,CX,CY,X,Y,W,H);
  else
  {
    CX=CX*Prt->WW/Prt->W;
    CY=CY*Prt->WH/Prt->H;
    CW=W*Prt->WW/Prt->W;
    CH=H*Prt->WH/Prt->H;
    X11ScaleImage(&Prt->Img1,&Prt->Img,0,0,X,Y,CW,CH,W,H);
    X11PutImage(Prt->Wnd,&Prt->Img1,CX,CY,0,0,CW,CH);
  }
}

void X11FreePort(Port *Prt)
{
  /* Close window and free the main image */
  XDestroyWindow(Dsp,Prt->Wnd);
  X11FreeImage(&Prt->Img);

  /* Free second image if it was allocated */
  if((Prt->W!=Prt->WW)||(Prt->H!=Prt->WH)) X11FreeImage(&Prt->Img1);
}

int X11SetPort(Port *Prt,int X,int Y,int W,int H)
{
  int WasOneImage,Result;

  /* Drop out if dimensions are wrong */
  if((W<=0)||(H<=0)) return(0);

  /* We assume everything is ok, unless something fails */
  Result=1;

  /* Check if we had one or two images */
  WasOneImage=(Prt->W==Prt->WW)&&(Prt->H==Prt->WH);

  /* If we no longer need a secondary buffer, get rid of it */
  if(!WasOneImage&&(Prt->WW==W)&&(Prt->WH==H)) X11FreeImage(&Prt->Img1);

  /* If we need a secondary buffer, allocate it */
  if(WasOneImage&&((Prt->WW!=W)||(Prt->WH!=H)))
    if(!X11NewImage(&Prt->Img1,Prt->WW,Prt->WH,Prt->Img.Attrs))
    {
      /* If failed, set size to 1:1, no secondary image */
      XResizeWindow(Dsp,Prt->Wnd,W,H);
      Prt->WW=W;
      Prt->WH=H;
      Result=0;
    }

  /* Set viewport parameters */
  Prt->W=W;
  Prt->H=H;
  Prt->X=X;
  Prt->Y=Y;

  /* Done */
  return(Result);  
}

int X11ZoomPort(Port *Prt,int W,int H)
{
  int WasOneImage,Result;

  /* Drop out if dimensions are wrong */
  if((W<=0)||(H<=0)) return(0);

  /* Drop out if size is the same */
  if((W==Prt->WW)&&(H==Prt->WH)) return(1);

  /* We assume everything is ok, unless something fails */
  Result=1;

  /* Check if we had one or two images */
  WasOneImage=(Prt->W==Prt->WW)&&(Prt->H==Prt->WH);

  /* If we had a secondary buffer, get rid of it */
  if(!WasOneImage) X11FreeImage(&Prt->Img1);

  /* If we need a secondary buffer, allocate it */
  if((Prt->W!=W)||(Prt->H!=H))
    if(!X11NewImage(&Prt->Img1,W,H,Prt->Img.Attrs))
    {
      /* If failed, set size to 1:1, no secondary image */
      W=Prt->W;
      H=Prt->H;
      Result=0;
    }

  /* Set new window size */
  XResizeWindow(Dsp,Prt->Wnd,W,H);
  Prt->WW=W;
  Prt->WH=H;

  /* Done */
  return(Result);
}

#endif /* UNIX */
