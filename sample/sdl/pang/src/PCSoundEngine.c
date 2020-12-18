#ifdef PC_VERSION
#include <stdio.h>
#include <stdlib.h>
#ifdef NO_SOUND
#else
#include <SDL.h>
#include <SDL_mixer.h>
#endif

#ifdef NO_SOUND
#else
Mix_Chunk *sound0;
Mix_Chunk *sound1;
Mix_Chunk *sound2;
Mix_Chunk *sound3;
Mix_Chunk *sound4;
Mix_Chunk *sound5;
Mix_Chunk *sound6;
Mix_Chunk *sound7;
Mix_Chunk *sound8;
Mix_Chunk *sound9;
Mix_Chunk *sound10;

int soundUsed0 = 0;
int soundUsed1 = 0;
int soundUsed2 = 0;
int soundUsed3 = 0;
int soundUsed4 = 0;
int soundUsed5 = 0;
int soundUsed6 = 0;
int soundUsed7 = 0;
int soundUsed8 = 0;
int soundUsed9 = 0;
int soundUsed10 = 0;
#endif

void initSoundEngine()
{
#ifdef NO_SOUND
#else
     int tmp=0;
 int audio_rate = 44100;
 Uint16 audio_format = AUDIO_S16;
 int audio_channels = 2;
 int audio_buffers = 4096;

#ifdef GP2X_VERSION
 audio_rate = 44100;
 audio_buffers = 128;
#endif
 

 if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
 	printf("Unable to initialize audio: %s\n", Mix_GetError());
  	exit(1);
  }
#endif
  }

void loadSound(char *filenamePC,char *filenameDreamcast,int soundNumber)
{
#ifdef NO_SOUND
#else
     if (soundNumber==0)  sound0 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==1)  sound1 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==2)  sound2 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==3)  sound3 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==4)  sound4 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==5)  sound5 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==6)  sound6 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==7)  sound7 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==8)  sound8 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==9)  sound9 = Mix_LoadWAV(filenamePC);
     else if (soundNumber==10)  sound10 = Mix_LoadWAV(filenamePC);
#endif
}


void playSound(int soundNumber,int numberLoop)
{
#ifdef NO_SOUND
#else
 int channel=0;

 if ( (soundNumber==0) && (soundUsed0==0) ) {channel = Mix_PlayChannel(0, sound0, numberLoop);soundUsed0=1;}
 else if((soundNumber==1) && (soundUsed1==0)) {channel = Mix_PlayChannel(1, sound1, numberLoop);soundUsed1=1;}
 else if((soundNumber==2) && (soundUsed2==0)) {channel = Mix_PlayChannel(2, sound2, numberLoop);soundUsed2=1;}
 else if((soundNumber==3) && (soundUsed3==0)) {channel = Mix_PlayChannel(3, sound3, numberLoop);soundUsed3=1;}
 else if((soundNumber==4) && (soundUsed4==0)) {channel = Mix_PlayChannel(4, sound4, numberLoop);soundUsed4=1;}
 else if((soundNumber==5) && (soundUsed5==0)) {channel = Mix_PlayChannel(5, sound5, numberLoop);soundUsed5=1;}
 else if((soundNumber==6) && (soundUsed6==0)) {channel = Mix_PlayChannel(6, sound6, numberLoop);soundUsed6=1;}
 else if((soundNumber==7) && (soundUsed7==0)) {channel = Mix_PlayChannel(7, sound7, numberLoop);soundUsed7=1;}
 else if((soundNumber==8) && (soundUsed8==0)) {channel = Mix_PlayChannel(8, sound8, numberLoop);soundUsed8=1;}
 else if((soundNumber==9) && (soundUsed9==0)) {channel = Mix_PlayChannel(9, sound9, numberLoop);soundUsed9=1;}
 else if((soundNumber==10) && (soundUsed10==0)) {channel = Mix_PlayChannel(10, sound10, numberLoop);soundUsed10=1;}
 
 if(channel == -1) {
 	printf("Unable to play WAV file: %s\n", Mix_GetError());
}
#endif
}

/*
  0 - pas clear?  1 - clear?*/
int clearSound(int soundNumber)
{
#ifdef NO_SOUND
#else
     if (soundNumber==0)
{
          if (Mix_Playing(0) != 0) {return 0;} else {Mix_HaltChannel(0);/*Mix_FreeChunk(sound0);*/soundUsed0=0;return 1;}
     }
     if (soundNumber==1)
{
          if (Mix_Playing(1) != 0) {return 0;} else {Mix_HaltChannel(1);/*Mix_FreeChunk(sound1);*/soundUsed1=0;return 1;}
     }
     if (soundNumber==2)
{
          if (Mix_Playing(2) != 0) {return 0;} else {Mix_HaltChannel(2);/*Mix_FreeChunk(sound2);*/soundUsed2=0;return 1;}
     }
     if (soundNumber==3)
{
          if (Mix_Playing(3) != 0) {return 0;} else {Mix_HaltChannel(3);/*Mix_FreeChunk(sound3);*/soundUsed3=0;return 1;}
     }
     if (soundNumber==4)
{
          if (Mix_Playing(4) != 0) {return 0;} else {Mix_HaltChannel(4);/*Mix_FreeChunk(sound4);*/soundUsed4=0;return 1;}
     }
     if (soundNumber==5)
{
          if (Mix_Playing(5) != 0) {return 0;} else {Mix_HaltChannel(5);/*Mix_FreeChunk(sound5);*/soundUsed5=0;return 1;}
     }
     if (soundNumber==6)
{
          if (Mix_Playing(6) != 0) {return 0;} else {Mix_HaltChannel(6);/*Mix_FreeChunk(sound6);*/soundUsed6=0;return 1;}
     }
     if (soundNumber==7)
{
          if (Mix_Playing(7) != 0) {return 0;} else {Mix_HaltChannel(7);/*Mix_FreeChunk(sound7);*/soundUsed7=0;return 1;}
     }
     if (soundNumber==8)
{
          if (Mix_Playing(8) != 0) {return 0;} else {Mix_HaltChannel(8);/*Mix_FreeChunk(sound8);*/soundUsed8=0;return 1;}
     }
     if (soundNumber==9)
{
          if (Mix_Playing(9) != 0) {return 0;} else {Mix_HaltChannel(9);/*Mix_FreeChunk(sound9);*/soundUsed9=0;return 1;}
     }
     if (soundNumber==10)
{
          if (Mix_Playing(10) != 0) {return 0;} else {Mix_HaltChannel(10);/*Mix_FreeChunk(sound10);*/soundUsed10=0;return 1;}
     }
#endif
 }
 
void forceClearSound (int soundNumber)
{
#ifdef NO_SOUND
#else
     if (soundNumber==0) {Mix_HaltChannel(0);/*Mix_FreeChunk(sound0);*/soundUsed0=0;}
     else if (soundNumber==1) {Mix_HaltChannel(1);/*Mix_FreeChunk(sound1);*/soundUsed1=0;}
     else if (soundNumber==2) {Mix_HaltChannel(2);/*Mix_FreeChunk(sound2);*/soundUsed2=0;}
     else if (soundNumber==3) {Mix_HaltChannel(3);/*Mix_FreeChunk(sound3);*/soundUsed3=0;}
     else if (soundNumber==4) {Mix_HaltChannel(4);/*Mix_FreeChunk(sound4);*/soundUsed4=0;}
     else if (soundNumber==5) {Mix_HaltChannel(5);/*Mix_FreeChunk(sound5);*/soundUsed5=0;}
     else if (soundNumber==6) {Mix_HaltChannel(6);/*Mix_FreeChunk(sound6);*/soundUsed6=0;}
     else if (soundNumber==7) {Mix_HaltChannel(7);/*Mix_FreeChunk(sound7);*/soundUsed7=0;}
     else if (soundNumber==8) {Mix_HaltChannel(8);/*Mix_FreeChunk(sound8);*/soundUsed8=0;}
     else if (soundNumber==9) {Mix_HaltChannel(9);/*Mix_FreeChunk(sound9);*/soundUsed9=0;}
     else if (soundNumber==10) {Mix_HaltChannel(10);/*Mix_FreeChunk(sound10);*/soundUsed10=0;}
 #endif
}

void quitSoundEngine()
{
#ifdef NO_SOUND
#else
     Mix_CloseAudio();
#endif
}

#endif
