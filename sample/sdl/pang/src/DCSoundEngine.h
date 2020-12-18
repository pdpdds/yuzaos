#ifdef DREAMCAST_VERSION

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

extern Mix_Chunk *sound[10];

void initSoundEngine();
void loadSound(char *filenameDreamcast,char *filenamePC,int soundNumber);
void playSound(int soundNumber,int number);
void cvlearSound(int soundNumber);
void forceClearSound (int soundNumber);
void quitSoundEngine();

#endif
