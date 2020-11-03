/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                           Unix.c                        **/
/**                                                         **/
/** This file contains Unix/X-dependent subroutines and     **/
/** drivers. It includes common drivers from Common.h.      **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2003                 **/
/**               Elan Feingold   1995                      **/
/**               Ville Hallik    1996                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifdef UNIX
#ifndef SDL

/** Private #includes ****************************************/
#include "MSX.h"
#include "LibUnix.h"
#include "Sound.h"

/** Standard Unix/X #includes ********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

/** Public parameters ****************************************/
char *Title="fMSX Unix/X 2.7";  /* Window/command line title */
int SaveCPU   = 1;              /* 1 = freeze when focus out */
int UseSHM    = 1;              /* 1 = use MITSHM            */
int UseSound  = 44100;          /* Sound driver frequency    */
int UseZoom   = 1;              /* Window zoom factor        */
int UseStatic = 0;              /* 1 = use static palette    */
int SyncFreq  = 0;              /* Screen update frequency   */
char *Disks[2][MAXDISKS+1];     /* Disk names for each drive */

/** Various variables ****************************************/
#define WIDTH  272
#define HEIGHT 228

static volatile byte TimerReady;
static unsigned int BPal[256],XPal[80],XPal0; 
static char *XBuf;
static Port Prt;
static byte JoyState;
static byte XKeyMap[16];
static int CurDisk[2];

/** Various X-related variables ******************************/
static Display *Dsp;
static Colormap DefaultCMap;
static GC DefaultGC;
static unsigned long White,Black;

/** Sound-related definitions ********************************/
#ifdef SOUND
static int SndSwitch = (1<<MAXCHANNELS)-1;
static int SndVolume = 255/MAXCHANNELS;
#endif

/** These functions are called on signals ********************/
static void OnBreak(int Arg) { ExitNow=1;signal(Arg,OnBreak); }
static void OnTimer(int Arg) { TimerReady=1;signal(Arg,OnTimer); }

/** Keyboard bindings ****************************************/
struct { word Code; byte Pos,Mask; } Keys[256] =
{
  {' ',8,0x01},{'\'',2,0x01},
  {'[',1,0x20},{'\\',1,0x10},{']',1,0x40},
  {',',2,0x04},{'-',1,0x04},{'.',2,0x08},{'/',2,0x10},
  {'0',0,0x01},{'1',0,0x02},{'2',0,0x04},{'3',0,0x08},
  {'4',0,0x10},{'5',0,0x20},{'6',0,0x40},{'7',0,0x80},
  {'8',1,0x01},{'9',1,0x02},{';',1,0x80},{'=',1,0x08},
  {'`',2,0x02},{'a',2,0x40},{'b',2,0x80},{'c',3,0x01},
  {'d',3,0x02},{'e',3,0x04},{'f',3,0x08},{'g',3,0x10},
  {'h',3,0x20},{'i',3,0x40},{'j',3,0x80},{'k',4,0x01},
  {'l',4,0x02},{'m',4,0x04},{'n',4,0x08},{'o',4,0x10},
  {'p',4,0x20},{'q',4,0x40},{'r',4,0x80},{'s',5,0x01},
  {'t',5,0x02},{'u',5,0x04},{'v',5,0x08},{'w',5,0x10},
  {'x',5,0x20},{'y',5,0x40},{'z',5,0x80},
  {XK_BackSpace,  7,0x20},
  {XK_Tab,        7,0x08},
  {XK_Return,     7,0x80},
  {XK_Escape,     7,0x04},
  {XK_Left,       8,0x10},
  {XK_Up,         8,0x20},
  {XK_Right,      8,0x80},
  {XK_Down,       8,0x40},
  {XK_F1,         6,0x20},
  {XK_F2,         6,0x40},
  {XK_F3,         6,0x80},
  {XK_F4,         7,0x01},
  {XK_F5,         7,0x02},
  {XK_Shift_L,    6,0x01}, /* SHIFT      */
  {XK_Shift_R,    6,0x08}, /* CAPSLOCK   */
  {XK_Control_L,  6,0x02}, /* CONTROL    */
  {XK_Control_R,  6,0x02}, /* CONTROL    */
  {XK_Alt_L,      6,0x04}, /* GRAPH      */
  {XK_Alt_R,      6,0x04}, /* GRAPH      */
  {XK_Delete,     8,0x08}, /* DELETE     */
  {XK_Insert,     8,0x04}, /* INSERT     */
  {XK_Page_Down,  6,0x10}, /* COUNTRY    */
  {XK_Page_Up,    7,0x10}, /* STOP/BREAK */
  {XK_Home,       8,0x02}, /* HOME/CLS   */
  {XK_End,        7,0x40}, /* SELECT     */
  {0,0,0x00}               /** The End. **/
};

/** InitMachine() ********************************************/
/** Allocate resources needed by Unix/X-dependent code.     **/
/*************************************************************/
int InitMachine(void)
{
  Screen *Scr;
  int J,I;

  /* Reset all variables */
  memset(XKeyMap,0xFF,sizeof(XKeyMap));
  XBuf=NULL;JoyState=0x00;
  CurDisk[0]=CurDisk[1]=0;

  /* Correct configuration values */
  UseZoom  = UseZoom>4? 4:UseZoom<1? 1:UseZoom;
  SyncFreq = SyncFreq>60? 60:SyncFreq<0? 0:SyncFreq;

  /* Open display */
  if(Verbose) printf("Initializing Unix/X drivers:\n  Opening display...");
  if(!(Dsp=XOpenDisplay(NULL))) { if(Verbose) puts("FAILED");return(0); }

  /* Set internal variables */
  Scr=DefaultScreenOfDisplay(Dsp);
  White=WhitePixelOfScreen(Scr);
  Black=BlackPixelOfScreen(Scr);
  DefaultGC=DefaultGCOfScreen(Scr);
  DefaultCMap=DefaultColormapOfScreen(Scr);

  /* Keys do not autorepeat */
  XAutoRepeatOff(Dsp);

  /* Initialize LibUnix toolkit */
  if(Verbose) printf("OK\n  Initializing LibUnix...");
  if(!InitLibUnix(Dsp)) { if(Verbose) puts("FAILED");return(0); }

  /* Create a viewport */
  if(Verbose) printf("OK\n  Opening window...");
  if(!X11NewPort(&Prt,Title,WIDTH,HEIGHT,UseSHM? USE_SHM:0))
  { if(Verbose) puts("FAILED");return(0); }
  if(Verbose) printf("%dbpp...OK\n",Prt.Img.D);
  XBuf=Prt.Img.Data;
  
  /* Set image/window size */
  X11SetPort(&Prt,0,0,WIDTH,HEIGHT);
  X11ZoomPort(&Prt,UseZoom*WIDTH,UseZoom*HEIGHT);

  /* Reset the palette */
  for(J=0;J<16;J++) XPal[J]=Black;
  XPal0=Black;

  /* Set SCREEN8 colors */
  for(J=0;J<64;J++)
  {
    I=(J&0x03)+(J&0x0C)*16+(J&0x30)/2;
    XPal[J+16]=X11SetColor(J+16,(J&0x30)*255/48,(J&0x0C)*255/12,(J&0x03)*255/3);
    BPal[I]=BPal[I|0x04]=BPal[I|0x20]=BPal[I|0x24]=XPal[J+16];
  }

#ifdef SOUND
  /* Initialize sound */   
  if(InitSound(UseSound,Verbose))
    SetChannels(SndVolume,SndSwitch);
#endif

  /* Catch all signals */
  signal(SIGHUP,OnBreak);signal(SIGINT,OnBreak);
  signal(SIGQUIT,OnBreak);signal(SIGTERM,OnBreak);

  /* Establish timer signal if needed */
  if(SyncFreq) TimerSignal(SyncFreq,OnTimer);

  /* Flush events (including spurious Resize events */
  XSync(Dsp,True);

  return(1);
}

/** TrashMachine() *******************************************/
/** Deallocate all resources taken by InitMachine().        **/
/*************************************************************/
void TrashMachine(void)
{
  if(Verbose) printf("Shutting down...\n");

  if(Dsp)
  {
    X11FreeColors();
    X11FreePort(&Prt);
    XAutoRepeatOn(Dsp);
    XCloseDisplay(Dsp);
  }

#ifdef SOUND
  StopSound();
  TrashSound();
#endif
}

/** PutImage() ***********************************************/
/** Put an image on the screen.                             **/
/*************************************************************/
void PutImage(void)
{
  /* Wait if needed */
  if(SyncFreq) while(!TimerReady&&!ExitNow);

  /* Copy image */
  X11DrawPort(&Prt,0,0,WIDTH,HEIGHT);

  /* Reset timer flag */
  TimerReady=0;
}

/** Keyboard() ***********************************************/
/** Check for keyboard events, parse them, and modify MSX   **/
/** keyboard matrix.                                        **/
/*************************************************************/
void Keyboard(void)
{
  static byte Control=0;
  XEvent E;
  int J,I;

  /* Check for keypresses/keyreleases */
  if(XCheckWindowEvent(Dsp,Prt.Wnd,KeyPressMask|KeyReleaseMask,&E))
  {
    J=XLookupKeysym((XKeyEvent *)&E,0);
    if(E.type==KeyPress)
    {
      /* Special keys pressed... */
      switch(J)
      {
#ifdef DEBUG
        case XK_F11:
          CPU.Trace=!CPU.Trace;
          break;
#endif

        case XK_F6:
          AutoFire=!AutoFire;
          break;

        case XK_F7:
          if(StateName)
          { if(Control) SaveState(StateName); else LoadState(StateName); }
          break;

        case XK_F8:
          I=Control? 1:0;
          if(Disks[I][0])
          {
            CurDisk[I]=(CurDisk[I]+1)%MAXDISKS;
            if(!Disks[I][CurDisk[I]]) CurDisk[I]=0;
            ChangeDisk(I,Disks[I][CurDisk[I]]);
          }
          break;

        case XK_F9:
          UseFont=UseFont? 0:FontBuf? 1:0;
          break;

        case XK_F10:
#ifdef SOUND
          if(!Control) SetChannels(SndVolume,SndSwitch=~SndSwitch);
#endif
          if(Control) MIDILogging(MIDI_TOGGLE);
          break;

        case XK_F12:
          ExitNow=1;
          break;

        case XK_Up:    JoyState|=0x01;break;
        case XK_Down:  JoyState|=0x02;break;
        case XK_Left:  JoyState|=0x04;break;
        case XK_Right: JoyState|=0x08;break;
        case XK_Alt_L: JoyState|=0x10;break;
        case XK_Control_L:
        case XK_Control_R:
          if(J==XK_Control_L) JoyState|=0x20;
          Control=1;
          break;
      }

      /* Key pressed: reset bit in KeyMap */
      for(I=0;Keys[I].Code;I++)
        if(Keys[I].Code==J) { XKeyMap[Keys[I].Pos]&=~Keys[I].Mask;break; }
    }
    else
    {
      /* Special keys released... */
      switch(J)
      {
        case XK_Up:    JoyState&=0xFE;break;
        case XK_Down:  JoyState&=0xFD;break;
        case XK_Left:  JoyState&=0xFB;break;
        case XK_Right: JoyState&=0xF7;break;
        case XK_Alt_L: JoyState&=0xEF;break;
        case XK_Control_L:
        case XK_Control_R:
          if(J==XK_Control_L) JoyState&=0xDF;
          Control=0;
          break;
      }

      /* Key released: set bit in KeyMap */
      for(I=0;Keys[I].Code;I++)
        if(Keys[I].Code==J) { XKeyMap[Keys[I].Pos]|=Keys[I].Mask;break; }
    }
  }

  /* Copy local keymap to the global one */
  memcpy(KeyMap,XKeyMap,sizeof(XKeyMap));

  /* If window has been resized, change the viewport */
  for(E.type=0;XCheckWindowEvent(Dsp,Prt.Wnd,StructureNotifyMask,&E););
  if((E.type==ConfigureNotify)&&!E.xconfigure.send_event)
    X11ZoomPort(&Prt,E.xconfigure.width,E.xconfigure.height);

  /* If saving CPU and focus is out, sleep */
  for(E.type=0;XCheckWindowEvent(Dsp,Prt.Wnd,FocusChangeMask,&E););
  if(SaveCPU&&(E.type==FocusOut))
  {
    XAutoRepeatOn(Dsp);
#ifdef SOUND
    StopSound();
#endif
  
    do
      while(!XCheckWindowEvent(Dsp,Prt.Wnd,FocusChangeMask,&E)&&!ExitNow)
        sleep(1);
    while((E.type!=FocusIn)&&!ExitNow);

    XAutoRepeatOff(Dsp);
#ifdef SOUND
    ResumeSound();
#endif
  }
}

/** Joystick() ***********************************************/
/** Query position of a joystick connected to port N.       **/
/** Returns 0.0.F2.F1.R.L.D.U.                              **/
/*************************************************************/
byte Joystick(register byte N) { return(JoyState); }

/** Mouse() **************************************************/
/** Query coordinates of a mouse connected to port N.       **/
/** Returns F2.F1.Y.Y.Y.Y.Y.Y.Y.Y.X.X.X.X.X.X.X.X.          **/
/*************************************************************/
int Mouse(register byte N)
{
  Window Dummy;
  int X,Y,J,Buttons;

  /* Get mouse coordinates */
  if(!XQueryPointer(Dsp,Prt.Wnd,&Dummy,&Dummy,&J,&J,&X,&Y,&Buttons))
    X=Y=Buttons=0;
  else
  {
    X=X*Prt.W/Prt.WW;
    X-=(Prt.W-256)/2;
    X=X<0? 0:(X>255? 255:X);
    Y=Y*Prt.H/Prt.WH;
    Y-=8;
    Y=Y<0? 0:(Y>211? 211:Y);
    Buttons=(Buttons&Button1Mask? 0x10000:0x00000)
           |(Buttons&Button2Mask? 0x20000:0x00000)
           |(Buttons&Button3Mask? 0x20000:0x00000);
  }

  return((Y<<8)|X|Buttons);
}

/** SetColor() ***********************************************/
/** Set color N (0..15) to R,G,B.                           **/
/*************************************************************/
void SetColor(register byte N,register byte R,register byte G,register byte B)
{
  unsigned int J;

  if(UseStatic) J=BPal[((7*R/255)<<2)|((7*G/255)<<5)|(3*B/255)];
  else J=X11SetColor(N,R,G,B);

  if(N) XPal[N]=J; else XPal0=J;
}

/** Part of the code common for Unix/X and MSDOS drivers *****/ 
#include "Common.h"

#endif
#endif /* UNIX */
