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


CFuelObject::CFuelObject(int nx,int ny,CTile *t,CGame *g) : CObject(nx,ny,t,CONSTITUTION_FUEL,g)
{
	y_speed=0;
	y_precision=0;

	state=0;

	x_speed=0;
	x_precision=0;
} /* CFuelObject::CFuelObject */ 


CFuelObject::~CFuelObject(void)
{
} /* CFuelObject::~CFuelObject */ 



bool CFuelObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	y_precision+=y_speed;
	while(y_precision>(1<<8)) {
		y++;
		y_precision-=1<<8;
	} /* while */ 
	while(y_precision<((-1)<<8)) {
		y--;
		y_precision+=1<<8;
	} /* while */ 

	x_precision+=x_speed;
	while(x_precision>(1<<8)) {
		x++;
		x_precision-=1<<8;
	} /* while */ 
	while(x_precision<((-1)<<8)) {
		x--;
		x_precision+=1<<8;
	} /* while */ 

	y_speed=-ENEMY_SPEED;

	y_speed=-ENEMY_SPEED;
	x_speed=0;
	if (game->object_collision(16,0,this,CONSTITUTION_SOLID)!=0) {
		x_speed=-ENEMY_HSPEED;
	} else {
		if (game->object_collision(-16,0,this,CONSTITUTION_SOLID)!=0) {
			x_speed=ENEMY_HSPEED;
		} /* if */ 
	} /* if */ 

	if (game->min_distance_to_players(y)>PLAYING_WINDOW) return false;
	
	return true;
} /* CFuelObject::cycle */ 

