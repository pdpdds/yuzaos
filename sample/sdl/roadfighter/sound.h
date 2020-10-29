#ifndef __BRAIN_SDL_SOUND
#define __BRAIN_SDL_SOUND

typedef Mix_Chunk * SOUNDT;

bool Sound_initialization(void);
int Sound_initialization(int nc,int nrc);
void Sound_release(void);

bool Sound_file_test(char *f1);

SOUNDT Sound_create_sound(char *file);
void Sound_delete_sound(SOUNDT s);
int Sound_play(SOUNDT s);
int Sound_play(SOUNDT s,int volume);
int Sound_play_continuous(SOUNDT s);
int Sound_play_continuous(SOUNDT s,int volume);
void Sound_play_ch(SOUNDT s,int channel);
void Sound_play_ch(SOUNDT s,int channel,int volume);

void Sound_create_music(char *f1,int times);
void Sound_release_music(void);
void Sound_pause_music(void);
void Sound_unpause_music(void);

void Sound_music_volume(int volume);

/* These functions are AGRESIVE! (i.e. they actually STOP SDL_mixer and restart it) */
void Stop_playback(void);
void Resume_playback(void);
int Resume_playback(int nc,int nr);

#endif

