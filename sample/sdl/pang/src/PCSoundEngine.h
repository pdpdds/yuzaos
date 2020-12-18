#ifdef PC_VERSION

#include <SDL.h>

void initSoundEngine();
void loadSound(char *filenameDreamcast,char *filenamePC,int soundNumber);
void playSound(int soundNumber,int number);
void cvlearSound(int soundNumber);
void forceClearSound (int soundNumber);
void quitSoundEngine();
int clearSound(int soundNumber);

#endif
