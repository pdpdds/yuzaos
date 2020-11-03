/** EMULib Emulation Library *********************************/
/**                                                         **/
/**                         SndUnix.c                       **/
/**                                                         **/
/** This file contains standard sound generation routines   **/
/** for Unix using /dev/dsp and /dev/audio. See SndUnixT.c  **/
/** for the threaded routines that require PThreads.        **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996-2003                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifdef UNIX
#ifndef SDL

#include "Sound.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifdef SUN_AUDIO  

#include <sys/audioio.h>
#include <sys/conf.h>
#include <stropts.h>

#define AUDIO_CONV(A) (ULAW[0xFF&(128+(A))]) 

static const unsigned char ULAW[256] =
{
    31,   31,   31,   32,   32,   32,   32,   33,
    33,   33,   33,   34,   34,   34,   34,   35,
    35,   35,   35,   36,   36,   36,   36,   37,
    37,   37,   37,   38,   38,   38,   38,   39,
    39,   39,   39,   40,   40,   40,   40,   41,
    41,   41,   41,   42,   42,   42,   42,   43,
    43,   43,   43,   44,   44,   44,   44,   45,
    45,   45,   45,   46,   46,   46,   46,   47,
    47,   47,   47,   48,   48,   49,   49,   50,
    50,   51,   51,   52,   52,   53,   53,   54,
    54,   55,   55,   56,   56,   57,   57,   58,
    58,   59,   59,   60,   60,   61,   61,   62,
    62,   63,   63,   64,   65,   66,   67,   68,
    69,   70,   71,   72,   73,   74,   75,   76,
    77,   78,   79,   81,   83,   85,   87,   89,
    91,   93,   95,   99,  103,  107,  111,  119,
   255,  247,  239,  235,  231,  227,  223,  221,
   219,  217,  215,  213,  211,  209,  207,  206,
   205,  204,  203,  202,  201,  200,  199,  198,
   219,  217,  215,  213,  211,  209,  207,  206,
   205,  204,  203,  202,  201,  200,  199,  198,
   197,  196,  195,  194,  193,  192,  191,  191,
   190,  190,  189,  189,  188,  188,  187,  187,
   186,  186,  185,  185,  184,  184,  183,  183,
   182,  182,  181,  181,  180,  180,  179,  179,
   178,  178,  177,  177,  176,  176,  175,  175,
   175,  175,  174,  174,  174,  174,  173,  173,
   173,  173,  172,  172,  172,  172,  171,  171,
   171,  171,  170,  170,  170,  170,  169,  169,
   169,  169,  168,  168,  168,  168,  167,  167,
   167,  167,  166,  166,  166,  166,  165,  165,
   165,  165,  164,  164,  164,  164,  163,  163
};

#else /* SUN_AUDIO */

#ifdef __FreeBSD__
#include <machine/soundcard.h>
#endif
 
#ifdef __NetBSD__
#include <soundcard.h>
#endif
 
#ifdef __linux__
#include <sys/soundcard.h>
#endif
    
#define AUDIO_CONV(A) (128+(A))

#endif /* SUN_AUDIO */

static int PipeFD[2];
static int PeerPID      = 0;
static int SoundFD      = -1;
static int SoundRate    = 0;
static int MasterVolume = 200;
static int MasterSwitch = (1<<SND_CHANNELS)-1;
static int LoopFreq     = 25;
static int NoiseGen     = 1;
static int Verbose      = 1;

static struct   
{
  int Type;                       /* Channel type (SND_*)             */
  int Freq;                       /* Channel frequency (Hz)           */
  int Volume;                     /* Channel volume (0..255)          */

  signed char Data[SND_SAMPLESIZE]; /* Wave data (-128..127 each)     */
  int Length;                     /* Wave length in Data              */
  int Rate;                       /* Wave playback rate (or 0Hz)      */
  int Pos;                        /* Wave current position in Data    */

  int Count;                      /* Phase counter                    */
} CH[SND_CHANNELS];

static void UnixSetWave(int Channel,const signed char *Data,int Length,int Rate);
static void UnixSetSound(int Channel,int NewType);
static void UnixDrum(int Type,int Force);
static void UnixSetChannels(int Volume,int Switch);
static void UnixSound(int Channel,int NewFreq,int NewVolume);

static int  OpenSoundDevice(int Rate);
static void SoundSignal(int SIG);
static void DSPLoop(void);

/** StopSound() **********************************************/
/** Temporarily suspend sound.                              **/
/*************************************************************/
void StopSound(void) { if(SoundRate) kill(PeerPID,SIGUSR1); }

/** ResumeSound() ********************************************/
/** Resume sound after StopSound().                         **/
/*************************************************************/
void ResumeSound(void) { if(SoundRate) kill(PeerPID,SIGUSR2); }

/** OpenSoundDevice() ****************************************/
/** Open /dev/dsp with a given level of sound quality.      **/
/** Returns 0 if failed or sound quality (Mode).            **/
/*************************************************************/
static int OpenSoundDevice(int Rate)
{
#ifdef SUN_AUDIO

  if(Verbose) printf("  Opening /dev/audio...");
  if((SoundFD=open("/dev/audio",O_WRONLY | O_NONBLOCK))<0)
  {
    if(Verbose) puts("FAILED");
    return(0);
  }

  /*
  ** Sun's specific initialization should be here...
  ** We assume, that it's set to 8000Hz u-law mono right now.
  */    

#else /* SUN_AUDIO */

  int I,J,K;

  /* At first, we need to open /dev/dsp: */
  if(Verbose) printf("  Opening /dev/dsp...");
  I=((SoundFD=open("/dev/dsp",O_WRONLY))<0);

  /* Set 8-bit sound */
  if(!I)
  { 
    if(Verbose) printf("OK\n  Setting mode: 8bit...");
    J=AFMT_U8;
    I=(ioctl(SoundFD,SNDCTL_DSP_SETFMT,&J)<0);
  }

  /* Set mono sound */
  if(!I)
  {
    if(Verbose) printf("mono...");
    J=0;
    I=(ioctl(SoundFD,SNDCTL_DSP_STEREO,&J)<0);
  }

  /* Set sampling rate */
  if(!I)
  {
    if(Verbose) printf("OK\n  Setting sampling rate: %dHz...",Rate);
    I=(ioctl(SoundFD,SNDCTL_DSP_SPEED,&Rate)<0);
    if(Verbose) printf("(got %dHz)...",Rate);
  } 

  /* Here we set the number of buffers to use */
  if(!I)
  { 
    if(Verbose)
      printf
      (
        "OK\n  Adjusting buffers: %d buffers %d bytes each...",
        SND_BUFFERS,1<<SND_BITS
      );

    /* Set buffer length and number of buffers */
    J=K=SND_BITS|(SND_BUFFERS<<16);
    I=(ioctl(SoundFD,SNDCTL_DSP_SETFRAGMENT,&J)<0);
    /* Buffer length as n, not 2^n! */
    if((J&0xFFFF)<=16) J=(J&0xFFFF0000)|(1<<(J&0xFFFF));
    K=(1<<SND_BITS)|(SND_BUFFERS<<16);

    /* If something went wrong... */
    if(J!=K)
    {
      if((J>>16)<SND_BUFFERS)       I=-1;
      if((J&0xFFFF)!=(1<<SND_BITS)) I=-1;
    }
  } 

  /* If something failed, fall out */
  if(I) { if(Verbose) puts("FAILED");return(0); }
    
#endif /* SUN_AUDIO */
    
  if(Verbose) puts("OK");
  return(Rate);
}

/** SoundSignal() ********************************************/
/** Signal handler for the sound server.                    **/
/*************************************************************/
static void SoundSignal(int SIG)
{
  static volatile int Suspended=0;

  switch(SIG)
  {
    case SIGUSR1:
      /* Suspend execution, until SIGUSR2 catched */
      if(SoundFD>=0)
      {
#ifndef SUN_AUDIO
        ioctl(SoundFD,SNDCTL_DSP_RESET);
#endif
        close(SoundFD);
        SoundFD=-1;
      }
      for(Suspended=1;Suspended;pause());
      SoundRate=OpenSoundDevice(SoundRate);
      break;
    case SIGUSR2:
      Suspended=0;
      break;
  }

  signal(SIG,SoundSignal);
}

/** DSPLoop() ************************************************/
/** Main loop of the sound server.                          **/
/*************************************************************/
static void DSPLoop(void)
{
  int Wave[SND_BUFSIZE];
  unsigned char Buf[SND_BUFSIZE],R;
  register int J,I,K,L,M,N,L1,L2,A1,A2,V;
  int FreqCount;

  /* Reset some variables to keep compiler happy */
  L=N=A2=0;

  /* Reset all channels */
  for(J=0;J<SND_CHANNELS;J++)
  {
    CH[J].Type   = SND_MELODIC;
    CH[J].Count  = 0;
    CH[J].Volume = 0;
    CH[J].Freq   = 0;
  }  

  /* Clear synthesis buffer */
  for(J=0;J<SND_BUFSIZE;J++) Wave[J]=0;

  FreqCount=SoundRate/SND_BUFSIZE;
  for(;;FreqCount-=LoopFreq)
  {
    /* Terminate if parent process dies */
    if(PeerPID!=getppid())
    {
      fprintf(stderr,"SOUND: Parent died\n");
      exit(1);
    }

    /* Read data from the pipe into sound registers */
    while((J=read(PipeFD[0],Buf,4)))
    {
      if(J<0)
      {
        if(errno==EWOULDBLOCK) break;
        else
        {
          fprintf(stderr,"SOUND: Can't read from pipe\n");
          exit(1);
        }
      }

      if(J==4)
        switch(R=Buf[0])
        {
          case 0xFF: MasterVolume=Buf[1];
                     MasterSwitch=Buf[2]+Buf[3]*256;
                     break;
          case 0xFE: R=Buf[1];
                     J=Buf[2]+Buf[3]*256+2;
                     /* If everything is correct... */
                     if((R<SND_CHANNELS)&&(J<SND_SAMPLESIZE))
                       if(read(PipeFD[0],Buf,2)==2)
                       {
                         J-=2;
                         CH[R].Type   = SND_WAVE;
                         CH[R].Rate   = Buf[0]+Buf[1]*256;
                         CH[R].Length = J;
                         CH[R].Count  = 0;
                         CH[R].Pos    = 0;
                         I=read(PipeFD[0],CH[R].Data,J);
                         if(I>0) J-=I;
                       }
                     /* Drain remaining bytes from the pipe */
                     for(;J>0;J--)
                       if(read(PipeFD[0],Buf,1)!=1)
                       {
                         fprintf(stderr,"SOUND: Can't drain pipe\n");
                         exit(1);
                       }
                     break;
          case 0xFC: R=Buf[1];
                     if(R<SND_CHANNELS) CH[R].Type=Buf[2];
                     break;
          default:   if(R<SND_CHANNELS)
                     {
                       CH[R].Volume = Buf[1];
                       CH[R].Freq   = Buf[2]+Buf[3]*256;
                     }
                     break;
        }
    }

    /* Waveform generator */
    for(J=0,M=MasterSwitch;M&&(J<SND_CHANNELS);J++,M>>=1)
      if(CH[J].Freq&&(V=CH[J].Volume)&&(M&1))
        switch(CH[J].Type)
        {
          case SND_NOISE: /* White Noise */
            /* For high frequencies, recompute volume */
            if(CH[J].Freq<=SoundRate) K=0x10000*CH[J].Freq/SoundRate;
            else { V=V*SoundRate/CH[J].Freq;K=0x10000; }
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++)
            {
              L1+=K;
              if(L1&0xFFFF0000)
              {
                L1&=0xFFFF;
                if((NoiseGen<<=1)&0x80000000) NoiseGen^=0x08000001;  
              }
              Wave[I]+=NoiseGen&1? V:-V;
            }
            CH[J].Count=L1;
            break;
              
          case SND_WAVE: /* Custom Waveform */
            /* Waveform data must have correct length! */
            if(CH[J].Length<=0) break;
            /* Start counting */
            K  = CH[J].Rate>0? (SoundRate<<15)/CH[J].Freq/CH[J].Rate
                             : (SoundRate<<15)/CH[J].Freq/CH[J].Length;
            L1 = CH[J].Pos%CH[J].Length;
            L2 = CH[J].Count;
            A1 = CH[J].Data[L1]*V;
            /* If expecting interpolation... */
            if(L2<K)
            {
              /* Compute interpolation parameters */
              A2 = CH[J].Data[(L1+1)%CH[J].Length]*V;
              L  = (L2>>15)+1;
              N  = ((K-(L2&0x7FFF))>>15)+1;
            } 
            /* Add waveform to the buffer */  
            for(I=0;I<SND_BUFSIZE;I++)
              if(L2<K)
              {
                /* Interpolate linearly */
                Wave[I]+=A1+L*(A2-A1)/N;
                /* Next waveform step */
                L2+=0x8000;  
                /* Next interpolation step */
                L++;
              }
              else
              {
                L1 = (L1+L2/K)%CH[J].Length;
                L2 = (L2%K)+0x8000;
                A1 = CH[J].Data[L1]*V;
                Wave[I]+=A1;
                /* If expecting interpolation... */
                if(L2<K)
                {
                  /* Compute interpolation parameters */
                  A2 = CH[J].Data[(L1+1)%CH[J].Length]*V;
                  L  = 1;
                  N  = ((K-L2)>>15)+1;  
                }
              }
            /* End counting */
            CH[J].Pos   = L1;
            CH[J].Count = L2;
            break;
                
          case SND_MELODIC:  /* Melodic Sound   */
          case SND_TRIANGLE: /* Triangular Wave */
          default:           /* Default Sound   */
            /* Triangular wave has twice less volume */
            if(CH[J].Type==SND_TRIANGLE) V=(V+1)>>1;
            /* Do not allow frequencies that are too high */
            if(CH[J].Freq>=SoundRate/3) break;
            K=0x10000*CH[J].Freq/SoundRate;
            L1=CH[J].Count;
            V<<=7;
            for(I=0;I<SND_BUFSIZE;I++)
            {
              L2=L1+K;
              Wave[I]+=L1&0x8000? (L2&0x8000? V:0):(L2&0x8000? 0:-V);
              L1=L2;
            }
            CH[J].Count=L1;  
            break;
        }

    /* Mix and convert waveforms */   
    for(J=0;J<SND_BUFSIZE;J++)
    {
      I=(Wave[J]*MasterVolume)>>16;
      I=I<-128? -128:I>127? 127:I;
      Buf[J]=AUDIO_CONV(I);
      Wave[J]=0;
    }

    if(SoundFD<0) sleep(1);
    else
    {
#ifdef SUN_AUDIO
      /* Flush output first, don't care about return status. After this
      ** write next buffer of audio data. This method produces a horrible 
      ** click on each buffer :( Any ideas, how to fix this?
      */
      ioctl(SoundFD,AUDIO_DRAIN);
      write(SoundFD,Buf,SND_BUFSIZE);
#else
      /* We'll block here until next DMA buffer becomes free. It happens 
      ** once per SND_BUFSIZE/SoundRate seconds.
      */
      write(SoundFD,Buf,SND_BUFSIZE);
#endif
    }
  }
}

/** InitSound() **********************************************/
/** Initialize DSP. Returns Rate on success, 0 otherwise.   **/
/** Mode is 0 to skip initialization (will be silent).      **/
/*************************************************************/
int InitSound(int Rate,int Verb)
{
  /* If sound was initialized, kill it */
  TrashSound();

  /* Silence requested */
  if(Rate<=0) return(0);

  /* Synthesis rate should be at least 8kHz */
  if(Rate<8192) Rate=44100;

  SoundRate = 0;
  SoundFD   = -1;
  PeerPID   = 0;
  Verbose   = Verb;

  /* Set driver functions */
  SndDriver.SetSound    = UnixSetSound;
  SndDriver.Drum        = UnixDrum;
  SndDriver.SetChannels = UnixSetChannels;
  SndDriver.Sound       = UnixSound;
  SndDriver.SetWave     = UnixSetWave;

  /* Open sound device */
  if(Verbose) puts("Starting sound server:");
  if(!(Rate=OpenSoundDevice(Rate))) return(0);

  if(Verbose) printf("  Opening pipe...");
  if(pipe(PipeFD)<0) { if(Verbose) puts("FAILED");return(0); }

  if(Verbose) { printf("OK\n  Forking...");fflush(stdout); }
  switch(PeerPID=fork())
  {
    case -1: if(Verbose) puts("FAILED");
             return(0);
    case 0:  close(PipeFD[1]);
             fcntl(PipeFD[0],F_SETFL,O_NONBLOCK);
             Verbose&=0x80;
             PeerPID=getppid();
             SoundRate=Rate;
             signal(SIGUSR1,SoundSignal);
             signal(SIGUSR2,SoundSignal);
             DSPLoop();
             exit(0);
    default: if(Verbose) puts("OK");
             close(PipeFD[0]);
             fcntl(PipeFD[1],F_SETFL,O_NONBLOCK);
             close(SoundFD);
             break;
  }

  SetChannels(192,0xFF);
  return(SoundRate=Rate);
}

/** TrashSound() *********************************************/
/** Shut DSP down.                                          **/
/*************************************************************/
void TrashSound(void)
{
  int J;

  StopSound();
  if(PeerPID) { kill(PeerPID,SIGKILL);wait(&J); }
  if(SoundFD>=0) close(SoundFD);
  SoundRate=0;
}

/** UnixSound() **********************************************/
/** Generate sound of given frequency (Hz) and volume       **/
/** (0..255) via given channel.                             **/
/*************************************************************/
void UnixSound(int Channel,int NewFreq,int NewVolume)
{
  unsigned char Buf[4];

  if((Channel<0)||(Channel>=SND_CHANNELS)) return;
  if(!SoundRate||!(MasterSwitch&(1<<Channel))) return;
  if(!NewVolume||!NewFreq) { NewVolume=0;NewFreq=0; }

  if((CH[Channel].Volume!=NewVolume)||(CH[Channel].Freq!=NewFreq))
  {
    CH[Channel].Volume = NewVolume;
    CH[Channel].Freq   = NewFreq;

    Buf[0]=Channel;
    Buf[1]=NewVolume;
    Buf[2]=NewFreq&0xFF;
    Buf[3]=NewFreq>>8;
    write(PipeFD[1],Buf,4);
  }
}

/** UnixSetChannels() ****************************************/
/** Set master volume (0..255) and turn channels on/off.    **/
/** Each bit in Toggle corresponds to a channel (1=on).     **/
/*************************************************************/
void UnixSetChannels(int MVolume,int MSwitch)
{
  unsigned char Buf[4];
  int J;

  if(!SoundRate) return;

  /* Sending new MasterVolume/MasterSwitch */
  Buf[0]=0xFF;
  Buf[1]=MVolume;
  Buf[2]=MSwitch&0xFF;
  Buf[3]=MSwitch>>8;
  write(PipeFD[1],Buf,4);

  /* Switching channels on/off */
  for(J=0;J<SND_CHANNELS;J++)
    if((MSwitch^MasterSwitch)&(1<<J))
    {
      /* Modifying channel #J */
      Buf[0]=J;

      /* Set volume/frequency */
      if(!(MSwitch&(1<<J))) Buf[1]=Buf[2]=Buf[3]=0;
      else
      {
        Buf[1]=CH[J].Volume;
        Buf[2]=CH[J].Freq&0xFF;
        Buf[3]=CH[J].Freq>>8;
      }
 
      /* Write out */
      write(PipeFD[1],Buf,4);
    }

  /* Set new MasterSwitch value */
  MasterSwitch=MSwitch;
  MasterVolume=MVolume;
}

/** UnixSetSound() *******************************************/
/** Set sound type (SND_NOISE/SND_MELODIC) for a given      **/
/** channel.                                                **/
/*************************************************************/
void UnixSetSound(int Channel,int NewType)
{
  unsigned char Buf[4];

  if(!SoundRate) return;
  if((Channel<0)||(Channel>=SND_CHANNELS)) return;
  CH[Channel].Type=NewType;

  Buf[0]=0xFC;
  Buf[1]=Channel;
  Buf[2]=NewType&0xFF;
  Buf[3]=NewType>>8;
  write(PipeFD[1],Buf,4);
}

/** UnixSetWave() ********************************************/
/** Set waveform for a given channel. The channel will be   **/
/** marked with sound type SND_WAVE. Set Rate=0 if you want **/
/** waveform to be an instrument or set it to the waveform  **/
/** own playback rate.                                      **/
/*************************************************************/
void UnixSetWave(int Channel,const signed char *Data,int Length,int Rate)
{
  unsigned char Buf[6+SND_SAMPLESIZE];

  if(!SoundRate) return;
  if((Channel<0)||(Channel>=SND_CHANNELS)) return;
  if((Length<=0)||(Length>SND_SAMPLESIZE)) return;

  Buf[0]=0xFE;
  Buf[1]=Channel;
  Buf[2]=Length&0xFF;
  Buf[3]=Length>>8;
  Buf[4]=Rate&0xFF;
  Buf[5]=Rate>>8;
  memcpy(Buf+6,Data,Length);
  write(PipeFD[1],Buf,6+Length);
}

/** UnixDrum() ***********************************************/
/** Hit a drum of a given type with given force.            **/
/*************************************************************/
void UnixDrum(int Type,int Force)
{
  /* This function is currently empty */
}

#endif
#endif /* UNIX */
