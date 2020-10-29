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


CEnemySlidderCarObject::CEnemySlidderCarObject(int nx,int ny,CTile *t,int start_delay,CGame *g) : CEnemyCarObject(nx,ny,t,start_delay,g)
{
} /* CEnemySlidderCarObject::CEnemySlidderCarObject */ 


CEnemySlidderCarObject::~CEnemySlidderCarObject(void)
{
} /* CEnemySlidderCarObject::~CEnemySlidderCarObject */ 



bool CEnemySlidderCarObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	bool retval=true;

	retval=CEnemyCarObject::cycle(keyboard,old_keyboard);

	if (state==1 && game->min_distance_to_players(y)<192) {
		CObject *closest_player=game->find_closest_player(x,y);
		if (x>closest_player->get_x()) {
			if (following_right_border) distance_to_border+=40;
								   else distance_to_border-=40;
		} else {
			if (following_right_border) distance_to_border-=40;
								   else distance_to_border+=40;
		} /* if */ 
		if (distance_to_border<0) distance_to_border+=80;
		state=2;
	} /* if */ 

	return retval;
} /* CEnemySlidderCarObject::cycle */ 
