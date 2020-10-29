#include "math.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include "sound.h"

#include "CTile.h"
#include "CObject.h"
#include "CGame.h"

extern int ENEMY_SPEED;
extern int ENEMY_HSPEED;
extern int PLAYING_WINDOW;


CEnemyRacerCarObject::CEnemyRacerCarObject(int nx,int ny,CTile *t,int start_delay,CGame *g) : CEnemyCarObject(nx,ny,t,start_delay,g)
{
	advanced=false;
} /* CEnemyRacerCarObject::CEnemyRacerCarObject */ 


CEnemyRacerCarObject::~CEnemyRacerCarObject(void)
{
} /* CEnemyRacerCarObject::~CEnemyRacerCarObject */ 



bool CEnemyRacerCarObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	bool retval=true;

	retval=CEnemyCarObject::cycle(keyboard,old_keyboard);

	if (!advanced && game->min_distance_to_players(y)<48 && y_speed!=0) {
		advanced=true;
		if (game->S_caradvance!=0) Sound_play(game->S_caradvance);
	} /* if */ 

	return retval;
} /* CEnemyRacerCarObject::cycle */ 

