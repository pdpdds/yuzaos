#include "math.h"
#include "stdlib.h"

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


CEnemyTruckObject::CEnemyTruckObject(int nx,int ny,CTile *t,int start_delay,CGame *g) : CCarObject(nx,ny,t,g)
{
	state_timmer=start_delay;

	following_right_border=true;
	distance_to_border=-1;

	constitution=CONSTITUTION_SOLID|CONSTITUTION_CAR;

	advanced=false;
} /* CEnemyTruckObject::CEnemyTruckObject */ 


CEnemyTruckObject::~CEnemyTruckObject(void)
{
} /* CEnemyTruckObject::~CEnemyTruckObject */ 

bool CEnemyTruckObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	CCarObject::cycle(keyboard,old_keyboard);

	if (state_timmer>0) state_timmer--;
	if (state==0 && state_timmer==0) state=1;

	if (state!=0) {
		y_speed=-ENEMY_SPEED;
		x_speed=0;
		if (game->object_collision(8,0,this,CONSTITUTION_SOLID)!=0) {
			x_speed=-ENEMY_HSPEED;
		} else {
			if (game->object_collision(-8,0,this,CONSTITUTION_SOLID)!=0) {
				x_speed=ENEMY_HSPEED;
			} /* if */ 
		} /* if */ 

		if (x_speed==0 && distance_to_border!=-1) {
			int i;
			int distance=-1;
			for(i=0;distance==-1 && i<=distance_to_border+8;i+=8) {
				if (following_right_border) {
					if (game->object_collision(i,0,this,CONSTITUTION_SOLID)!=0) distance=i-8;
				} else {
					if (game->object_collision(-i,0,this,CONSTITUTION_SOLID)!=0) distance=i-8;
				} /* if */ 
			} /* for */ 
			if (distance==-1) distance=distance_to_border+8;
			if (following_right_border) {
				if (distance<=distance_to_border-8) x_speed=-ENEMY_HSPEED;
				if (distance>=distance_to_border+8) x_speed=ENEMY_HSPEED;
			} else {
				if (distance<=distance_to_border-8) x_speed=ENEMY_HSPEED;
				if (distance>=distance_to_border+8) x_speed=-ENEMY_HSPEED;
			} /* if */ 
		} /* if */ 
	} else {
		y_speed=0;
	} /* if */ 

	if (game->min_distance_to_players(y)>PLAYING_WINDOW) return false;

	if (!advanced && game->min_distance_to_players(y)<48 && y_speed!=0) {
		advanced=true;
		if (game->S_caradvance!=0 && (rand()%3)==0) Sound_play(game->S_truck);
	} /* if */ 
	
	return true;
} /* CEnemyTruckObject::cycle */ 


