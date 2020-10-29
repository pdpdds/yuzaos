#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include "CTile.h"
#include "CObject.h"

CObject::CObject(void)
{
	state=0;
	ntiles=0;
	tiles=0;

	game=0;
	tile=0;

	x=0;
	y=0;
	draw_x=0;
	draw_y=0;
	constitution=CONSTITUTION_NONE;
} /* CObject::CObject */ 


CObject::CObject(int nx,int ny,CTile *t,int cons,CGame *g)
{
	ntiles=1;
	tiles=new CTile *[1];
	tiles[0]=t;

	game=g;
	tile=0;

	x=nx;
	y=ny;
	draw_x=nx;
	draw_y=ny;
	constitution=cons;
} /* CObject::CObject */ 


CObject::~CObject(void)
{
	int i;

	for(i=0;i<ntiles;i++) tiles[i]=0;
	delete tiles;

} /* CObject::~CObject */ 



bool CObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	return true;
} /* CObject::cycle */ 


void CObject::draw(int sx,int sy,SDL_Surface *screen)
{
	draw_x=x;
	draw_y=y;
	if (tile>=0 && tile<ntiles) tiles[tile]->draw(x-sx,y-sy,screen);
} /* CObject::draw */ 


bool CObject::collision(int offsx,int offsy,CObject *o)
{
	CTile *t1=0,*t2=0;

	if (tile>=0 && tile<ntiles) t1=tiles[tile];
	if (o->tile>=0 && o->tile<o->ntiles) t2=o->tiles[o->tile];

	if (t1==0 || t2==0) return false;
	if (t1->collision_data==0 || t2->collision_data==0) return false;

	if (draw_y+t1->get_dy()<o->draw_y+offsy ||
		o->draw_y+offsy+t2->get_dy()<draw_y) return false;

	{
		int tmpx,tmpy;

		tmpx=int((o->draw_x+offsx)-draw_x);
		tmpy=int((o->draw_y+offsy)-draw_y);

		if (sge_cmcheck(t1->collision_data,0,0,t2->collision_data,tmpx,tmpy)) {
			tmpx=sge_get_cx();
			tmpy=sge_get_cy();
			return true;
		} /* if */ 
	}

	return false;
} /* CObject::collision */ 


int CObject::get_dx(void)
{
	if (tile>=0 && tile<ntiles) return tiles[tile]->get_dx();

	return 0;
} /* CObject::get_dx */ 


int CObject::get_dy(void)
{
	if (tile>=0 && tile<ntiles) return tiles[tile]->get_dy();

	return 0;
} /* CObject::get_dy */ 


bool CObject::constitution_test(int cons)
{
	if ((constitution&cons)!=0) return true;

	return false;
} /* CObject::constitution_test */ 
