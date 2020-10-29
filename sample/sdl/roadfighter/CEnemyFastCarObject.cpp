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


int FENEMY_SPEED=(24<<8);
int FENEMY_HSPEED=32*24;

CEnemyFastCarObject::CEnemyFastCarObject(int nx,int ny,CTile *t,int start_delay,CGame *g) : CEnemyCarObject(nx,ny,t,start_delay,g)
{
} /* CEnemyFastCarObject::CEnemyFastCarObject */ 


CEnemyFastCarObject::~CEnemyFastCarObject(void)
{
} /* CEnemyFastCarObject::~CEnemyFastCarObject */ 



bool CEnemyFastCarObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	CCarObject::cycle(keyboard,old_keyboard);

	if (state_timmer>0) state_timmer--;
	if (state==0 && state_timmer==0) state=1;

	if (last_collision!=0) {
		if (last_collision->get_x()<x) slide_direction=1;
								  else slide_direction=-1;
		slide_timmer=8;

		last_collision=0;
	} /* if */ 

	if (state!=0) {
		y_speed=-FENEMY_SPEED;
		x_speed=0;
		if (game->object_collision(8,-32,this,CONSTITUTION_SOLID)!=0) {
			x_speed=-FENEMY_HSPEED;
			slide_timmer=0;
		} else {
			if (game->object_collision(-8,-32,this,CONSTITUTION_SOLID)!=0) {
				x_speed=FENEMY_HSPEED;
				slide_timmer=0;
			} /* if */ 
		} /* if */ 

		if (slide_timmer>0) {
			slide_timmer--;
			if (slide_direction==1) x_speed=FENEMY_HSPEED;
			if (slide_direction==-1) x_speed=-FENEMY_HSPEED;
		} /* if */ 

		if (x_speed==0) {
			if (game->object_collision(16,-64,this,CONSTITUTION_CAR)!=0 ||
				game->object_collision(16,-32,this,CONSTITUTION_CAR)!=0 ||
				game->object_collision(16,0,this,CONSTITUTION_CAR)!=0) {
				x_speed=-FENEMY_HSPEED;
				slide_timmer=0;
			} else {
				if (game->object_collision(-16,-64,this,CONSTITUTION_CAR)!=0 ||
					game->object_collision(-16,-32,this,CONSTITUTION_CAR)!=0 ||
					game->object_collision(-16,0,this,CONSTITUTION_CAR)!=0) {
					x_speed=FENEMY_HSPEED;
					slide_timmer=0;
				} /* if */ 
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
				if (distance<=distance_to_border-8) x_speed=-FENEMY_HSPEED;
				if (distance>=distance_to_border+8) x_speed=FENEMY_HSPEED;
			} else {
				if (distance<=distance_to_border-8) x_speed=FENEMY_HSPEED;
				if (distance>=distance_to_border+8) x_speed=-FENEMY_HSPEED;
			} /* if */ 
		} /* if */ 

		{
			CObject *o=0;

			/* Car collision: */ 
			o=game->object_collision(0,0,this,CONSTITUTION_CAR);
			if (o!=0) {
				CCarObject *co=(CCarObject *)o;

				co->car_collision(this);
				car_collision(co);
			} /* if */ 
		}
	} else {
		y_speed=0;
	} /* if */ 

	if (game->min_distance_to_players(y)>PLAYING_WINDOW) return false;
	
	return true;
} /* CEnemyFastCarObject::cycle */ 

