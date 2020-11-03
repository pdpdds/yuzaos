//		MUSIC.H Generalized sound driver - header
//		Copyright 1991 Epic MegaGames, Written by Tim Sweeney

#include <stdint.h>

#define num_samps 50
#define num_text  40
#define num_macs  128
#define maxsndlen 4096

extern int32_t vocposn [num_samps];
extern int16_t voclen [num_samps];
extern int16_t vocrate [num_samps];
extern char *soundmac [num_macs];
extern int32_t textposn [num_text];
extern int16_t textlen [num_text];

extern int16_t *freq;
extern int16_t *dur;

extern int16_t vocflag;
extern int16_t musicflag;
extern char *textmsg;
extern int16_t textmsglen;

extern uint16_t xmusicrate;
extern uint16_t xclockrate;
extern uint16_t xclockval;

extern char *xvocptr;
extern int16_t xvoclen;

extern int16_t soundf;
void snd_init (char *newvoclib);
void snd_play (int16_t priority,int16_t num);
void snd_do (void);
void snd_exit (void);
void text_get (int16_t n);

void music_init (void);
void sb_playtune (char *fname);
void sb_shutup (void);
void sb_update (void);
int16_t sb_playing (void);
