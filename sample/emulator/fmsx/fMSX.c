/** fMSX: portable MSX emulator ******************************/
/**                                                         **/
/**                          fMSX.c                         **/
/**                                                         **/
/** This file contains generic main() procedure statrting   **/
/** the emulation.                                          **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-2003                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#include "MSX.h"
#include "Help.h"

#ifdef SDL
#include "SDL.h"
#endif

#ifdef MSDOS
#include "LibMSDOS.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static const char *Options[]=
{ 
  "verbose","hperiod","vperiod","uperiod","pal","ntsc","help",
  "printer","serial","diska","diskb","tape","font","logsnd","state",
  "ram","vram","rom","auto","noauto","msx1","msx2","msx2+","joy",
  "home","trap","sound","nosound","shm","noshm","saver","nosaver",
  "scale","sync","nosync","static","nostatic","vsync","240","200",
#ifdef SDL
  "msxmusic","msxaudio","buffer","lowres","filter","stereo",
#endif
  0
};

extern char *Title;      /* Program title                       */
extern int   UseSound;   /* Sound mode (#ifdef SOUND)           */
extern int   UseSHM;     /* Use SHM X extension (#ifdef MITSHM) */
extern int   UseZoom;    /* Zoom factor (#ifdef UNIX)           */
extern int   SaveCPU;    /* Pause when inactive (#ifdef UNIX)   */
extern int   UseStatic;  /* Use static colors (MSDOS & UNIX)    */
extern int   SyncScreen; /* Sync screen updates (#ifdef MSDOS)  */
extern int   FullScreen; /* Use 256x240 screen (#ifdef MSDOS)   */
extern int   SyncFreq;   /* Sync screen updates (#ifdef UNIX)   */
extern int   SndBufSize; /* Size of audio buffer (#ifdef SDL)   */
extern int   ARGC;       /* argc/argv from main (#ifdef UNIX)   */
extern char **ARGV;

/** Zero-terminated arrays of disk names for each drive ******/
extern char *Disks[2][MAXDISKS+1];

/** main() ***************************************************/
/** This is a main() function used in Unix and MSDOS ports. **/
/** It parses command line arguments, sets emulation        **/
/** parameters, and passes control to the emulation itself. **/
/*************************************************************/

int main(int argc,char *argv[])
{
  int CartCount,TypeCount;
  int JoyCount,DiskCount[2];
  int N,J;

  const char* argv1[] = {"fmsx", "-msx2+", "-ram","16", "-vram", "16",  "-diska", "1278127358_ko_all.dsk"};
  const char* argv2[] = { "fmsx", "-msx2+", "-ram",  "16", "-vram", "16","Knightmare_SCC-I.rom" };
  argc = sizeof(argv2) / sizeof(const char*);
  argv = argv2;

#ifdef DEBUG
  CPU.Trap  = 0xFFFF;
  CPU.Trace = 0;
#endif

#ifdef UNIX
  ARGC      = argc;
  ARGV      = argv;
#endif


#ifdef SDL
  Verbose=1;
#endif

  /* Clear everything */
  CartCount=TypeCount=JoyCount=0;
  DiskCount[0]=DiskCount[1]=0;

  /* Default disk images */
  Disks[0][1]=Disks[1][1]=0;
  Disks[0][0]=DiskA;
  Disks[1][0]=DiskB;

  for(N=1;N<argc;N++)
    if(*argv[N]!='-')
      switch(CartCount++)
      {
        case 0: CartA=argv[N];break;
        case 1: CartB=argv[N];break;
        default: printf("%s: Excessive filename '%s'\n",argv[0],argv[N]);
      }
    else
    {    
      for(J=0;Options[J];J++)
        if(!strcmp(argv[N]+1,Options[J])) break;

      switch(J)
      {
//#ifndef SDL
        case 0:  N++;
                 if(N<argc) Verbose=atoi(argv[N]);
                 else printf("%s: No verbose level supplied\n",argv[0]);
                 break;
        case 1:  N++;
                 if(N>=argc)
                   printf("%s: No HBlank period supplied\n",argv[0]);
                 else
                 {
                   J=atoi(argv[N]);
                   if((J>=30)&&(J<=3000)) HPeriod=J;
                 }
                 break;
        case 2:  N++;
                 if(N>=argc)
                   printf("%s: No VBlank period supplied\n",argv[0]);
                 else
                 {
                   J=atoi(argv[N]);
                   if((J>=1000)&&(J<=100000)) VPeriod=J;
                   if((J>=256)&&(J<=999)) VPeriod=HPeriod*J;
                 }
                 break;
        case 3:  N++;
                 if(N>=argc)
                   printf("%s: No screen update period supplied\n",argv[0]);
                 else
                 {
                   J=atoi(argv[N]);
                   if((J>=1)&&(J<=20)) UPeriod=J; 
                 }
                 break;
        case 4:  VPeriod=VPERIOD_PAL/6;
                 HPeriod=HPERIOD/6;
                 break;
        case 5:  VPeriod=VPERIOD_NTSC/6;
                 HPeriod=HPERIOD/6;
                 break;
	case 6:  //printf
                 //("%s by Marat Fayzullin    (C)FMS 1994-2003\n",Title);
                 //for(J=0;HelpText[J];J++) puts(HelpText[J]);
                 return(0);
        case 7:  N++;
                 if(N<argc) PrnName=argv[N];
                 else printf("%s: No file for printer redirection\n",argv[0]);
                 break;
        case 8:  N++;
                 if(N<argc) ComName=argv[N];
                 else printf("%s: No file for serial redirection\n",argv[0]);
                 break;
//#endif
        case 9:  N++;
                 if(N>=argc)
                   printf("%s: No file for drive A\n",argv[0]);
                 else
                   if(DiskCount[0]>=MAXDISKS)
                     printf("%s: Too many disks for drive A\n",argv[0]);
                   else
                     Disks[0][DiskCount[0]++]=argv[N];
                 break;
        case 10: N++;
                 if(N>=argc) 
                   printf("%s: No file for drive B\n",argv[0]);
                 else
                   if(DiskCount[1]>=MAXDISKS) 
                     printf("%s: Too many disks for drive B\n",argv[0]);
                   else
                     Disks[1][DiskCount[1]++]=argv[N];
                 break;
        case 11: N++;  
                 if(N<argc) CasName=argv[N];
                 else printf("%s: No file for the tape\n",argv[0]);
                 break;
#ifndef SDL
        case 12: N++;
                 if(N<argc) FontName=argv[N];  
                 else printf("%s: No font name supplied\n",argv[0]);
                 break;
        case 13: N++;
                 if(N<argc) SndName=argv[N];
                 else printf("%s: No file for soundtrack logging\n",argv[0]);
                 break;
#endif
        case 14: N++;
                 if(N<argc) StateName=argv[N];
                 else printf("%s: No file to save emulation state\n",argv[0]);
                 break;
        case 15: N++;
                 if(N>=argc)
                   printf("%s: No number of RAM pages supplied\n",argv[0]);
                 else
                   RAMPages=atoi(argv[N]);
                 break;
        case 16: N++;
                 if(N>=argc)
                   printf("%s: No number of VRAM pages supplied\n",argv[0]);
                 else
                   VRAMPages=atoi(argv[N]);
                 break;
        case 17: N++;  
                 if(N>=argc)
                   printf("%s: No ROM mapper type supplied\n",argv[0]);
                 else
                   switch(TypeCount++)
                   {
                     case 0: ROMTypeA=ROMTypeB=atoi(argv[N]);break;
                     case 1: ROMTypeB=atoi(argv[N]);break;
                     default: printf("%s: Excessive -rom option\n",argv[0]);
                   }
                 break;
        case 18: AutoFire=1;break;
        case 19: AutoFire=0;break;
        case 20: MSXVersion=0;break;
        case 21: MSXVersion=1;break;
        case 22: MSXVersion=2;break;
#ifndef SDL
        case 23: N++;
                 if(N>=argc)
                   printf("%s: No joystick type supplied\n",argv[0]);
                 else
                   switch(JoyCount++)
                   {
                     case 0: JoyTypeA=atoi(argv[N]);break;
                     case 1: JoyTypeB=atoi(argv[N]);break;
                     default: printf("%s: Excessive -joy option\n",argv[0]);
                   }
                 break;
        case 24: N++;
                 if(N>=argc)
                   printf("%s: No home directory name supplied\n",argv[0]);
                 else
                   ProgDir=argv[N];
                 break;
#ifdef DEBUG
        case 25: N++;
                 if(N>=argc)
                   printf("%s: No trap address supplied\n",argv[0]);
                 else
                   if(!strcmp(argv[N],"now")) CPU.Trace=1;
                   else sscanf(argv[N],"%hX",&(CPU.Trap));
                 break;
#endif
#ifdef SOUND
        case 26: N++;
                 if(N>=argc) { UseSound=1;N--; }
                 else if(sscanf(argv[N],"%d",&UseSound)!=1)
                      { UseSound=1;N--; }
                 break;
        case 27: UseSound=0;break;
#endif
#ifdef UNIX
#ifdef MITSHM
        case 28: UseSHM=1;break;
        case 29: UseSHM=0;break;
#endif
        case 30: SaveCPU=1;break;
        case 31: SaveCPU=0;break;
        case 32: N++;
                 if(N<argc) UseZoom=atoi(argv[N]);
                 else printf("%s: No scaling factor supplied\n",argv[0]);
                 break;
        case 33: N++;
                 if(N<argc) SyncFreq=atoi(argv[N]);
                 else printf("%s: No sync frequency supplied\n",argv[0]);
                 break;
        case 34: SyncFreq=0;break;
        case 35: UseStatic=1;break;
        case 36: UseStatic=0;break;
#endif
#ifdef MSDOS
        case 33: N++;
                 if(N<argc) SyncFreq=atoi(argv[N]);
                 else printf("%s: No sync frequency supplied\n",argv[0]);
                 break;
        case 34: SyncFreq=0;break;
        case 35: UseStatic=1;break;
        case 36: UseStatic=0;break;
        case 37: SyncFreq=-1;break;
        case 38: FullScreen=1;break;
        case 39: FullScreen=0;break;
#endif
#endif /* SDL */
#ifdef SDL
        case 40: Use2413=1;break;
        case 41: Use8950=1;break;
        case 42: N++;
                 if(N<argc) SndBufSize=atoi(argv[N]);
                 else printf("%s: No buffer size supplied\n",argv[0]);
                 break;
#ifdef NARROW
        case 43: SetLowres();break;
#endif
        case 44: N++;
                 if(N<argc) UseFilter=atoi(argv[N]);
                 if(UseFilter>5 || UseFilter<0) {
                   printf("%s: Invalid filter mode supplied\n",argv[0]);
                   UseFilter=0;
                 }
                 break;
        case 45: UseStereo=1;break;
#endif /* SDL */
        default: printf("%s: Wrong option '%s'\n",argv[0],argv[N]);
      }
    }

  /* Terminate disk lists and set initial disk names */
  if(DiskCount[0]) { Disks[0][DiskCount[0]]=0;DiskA=Disks[0][0]; }
  if(DiskCount[1]) { Disks[1][DiskCount[1]]=0;DiskB=Disks[1][0]; }

  /* Start fMSX! */
  if(!InitMachine()) return(1);
  StartMSX();
  TrashMSX();
  TrashMachine();
  return(0);
}
