#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include "sound.h"

#include "CTile.h"
#include "CObject.h"
#include "CGame.h"

CExplosionObject::CExplosionObject(int nx,int ny,List<CTile> *l,int first_tile,int last_tile,CGame *g)
{
	int i;

	state=0;

	ntiles=(last_tile-first_tile)+1;
	tiles=new CTile *[(last_tile-first_tile)+1];

	for(i=0;i<(last_tile-first_tile)+1;i++) tiles[i]=l->operator [](i+first_tile);

	game=g;
	tile=0;

	x=nx;
	y=ny;
	draw_x=nx;
	draw_y=ny;
	constitution=CONSTITUTION_NONE;

	timmer=0;

} /* CExplosionObject::CExplosionObject */ 


CExplosionObject::~CExplosionObject(void)
{
} /* CExplosionObject::~CExplosionObject */ 



bool CExplosionObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	timmer++;

	tile=timmer/2;
	if (tile>=ntiles) return false;

	return true;
} /* CExplosionObject::cycle */ 
