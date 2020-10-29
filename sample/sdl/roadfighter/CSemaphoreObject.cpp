#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include "sound.h"

#include "CTile.h"
#include "CObject.h"
#include "CGame.h"

CSemaphoreObject::CSemaphoreObject(int nx,int ny,CTile *t1,CTile *t2,CTile *t3,CTile *t4,CTile *t5,CGame *g)
{
	state=0;

	ntiles=5;
	tiles=new CTile *[5];
	tiles[0]=t1;
	tiles[1]=t2;
	tiles[2]=t3;
	tiles[3]=t4;
	tiles[4]=t5;

	game=g;
	tile=0;

	x=nx;
	y=ny;
	constitution=CONSTITUTION_NONE;

	timmer=0;

} /* CSemaphoreObject::CSemaphoreObject */ 


CSemaphoreObject::~CSemaphoreObject(void)
{
} /* CSemaphoreObject::~CSemaphoreObject */ 



bool CSemaphoreObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	timmer++;

	if (timmer>=SEMAPHORE_TIME*1) tile=1;
	if (timmer>=SEMAPHORE_TIME*2) tile=0;
	if (timmer>=SEMAPHORE_TIME*3) tile=2;
	if (timmer>=SEMAPHORE_TIME*4) tile=0;
	if (timmer>=SEMAPHORE_TIME*5) tile=3;
	if (timmer>=SEMAPHORE_TIME*6) tile=0;
	if (timmer>=SEMAPHORE_TIME*7) tile=4;

	/* sound: */ 
	if (timmer==SEMAPHORE_TIME*1) if (game->S_redlight!=0) Sound_play(game->S_redlight);
	if (timmer==SEMAPHORE_TIME*3) if (game->S_redlight!=0) Sound_play(game->S_redlight);
	if (timmer==SEMAPHORE_TIME*5) if (game->S_redlight!=0) Sound_play(game->S_redlight);
//	if (timmer==SEMAPHORE_TIME*7) if (game->S_greenlight!=0) Sound_play(game->S_greenlight);

	return true;
} /* CSemaphoreObject::cycle */ 
