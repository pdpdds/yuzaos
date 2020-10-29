#include "assert.h"

#include "stdio.h"
#include "string.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "sge/sge.h"

#include "CTile.h"
#include "math.h"

#include "auxiliar.h"


CTile::CTile(void)
{
	r.x=0;
	r.y=0;
	r.h=0;
	r.w=0;
	orig=0;
	mask_visualization=0;
	mask_collision=0;
	collision_data=0;
} /* CTile::CTile */ 


CTile::CTile(int x,int y,int dx,int dy,SDL_Surface *o,bool collision)
{

	r.x=x;
	r.y=y;
	r.h=dy;
	r.w=dx;

	orig=SDL_CreateRGBSurface(0,dx,dy,32,RMASK,GMASK,BMASK,AMASK);
	SDL_SetAlpha(orig,0,SDL_ALPHA_OPAQUE);
	SDL_BlitSurface(o,&r,orig,0);

	SDL_SetAlpha(orig,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
	surface_mask_from_bitmap(orig,o,r.x+r.w,r.y);
	mask_visualization=0;

	if (collision) {
		SDL_Rect r2;

		r2.x=r.x+(r.w*2);
		r2.y=r.y;
		r2.w=r.w;
		r2.h=r.h;
		mask_collision=SDL_CreateRGBSurface(0,r.w,r.h,32,RMASK,GMASK,BMASK,AMASK);
		SDL_BlitSurface(o,&r2,mask_collision,0);
		surface_bw(mask_collision,128);
		collision_data=sge_make_cmap(mask_collision);
	} else {
		mask_collision=0;
		collision_data=0;
	} /* if */ 
} /* CTile::CTile */ 


CTile::~CTile(void)
{
	free();
} /* CTile::CTile */ 


void CTile::draw(int x,int y,SDL_Surface *dest)
{
	SDL_Rect d;

	if (orig!=0) {
		d.x=x;
		d.y=y;
		d.w=r.w;
		d.h=r.h;
		SDL_BlitSurface(orig,0,dest,&d);
//		SDL_BlitSurface(mask_collision,0,dest,&d);
	} /* if */ 
} /* CTile::draw */ 


void CTile::draw_collision_mask(int x,int y,SDL_Surface *dest)
{
	SDL_Rect d;

	if (orig!=0) {
		if (mask_collision!=0) {
			d.x=x;
			d.y=y;
			d.w=r.w;
			d.h=r.h;
			SDL_BlitSurface(mask_collision,0,dest,&d);
		} /* if */ 
	} /* if */ 
} /* CTile::draw_collision_mask */ 


void CTile::draw_shaded(int x,int y,SDL_Surface *dest,int factor,int red,int green,int blue,int alpha)
{
	SDL_Rect d;

	if (orig!=0) {
		SDL_Surface *tmp;
		d.x=0;
		d.y=0;
		d.w=r.w;
		d.h=r.h;
		tmp=SDL_DisplayFormatAlpha(orig);
		surface_shader(tmp,float(factor)/100.0F,red,green,blue,alpha);
		d.x=x;
		d.y=y;
		d.w=r.w;
		d.h=r.h;
		SDL_BlitSurface(tmp,0,dest,&d);
		SDL_FreeSurface(tmp);
	} /* if */ 
} /* CTile::draw_shaded */ 


void CTile::draw_bicolor(int x,int y,SDL_Surface *dest,int factor,int r1,int g1,int b1,int a1,int r2,int g2,int b2,int a2)
{
	SDL_Rect d;

	if (orig!=0) {
		SDL_Surface *tmp;
		d.x=0;
		d.y=0;
		d.w=r.w;
		d.h=r.h;
		tmp=SDL_DisplayFormatAlpha(orig);
		surface_bicolor(tmp,float(factor)/100.0F,r1,g1,b1,a1,r2,g2,b2,a2);
		d.x=x;
		d.y=y;
		d.w=r.w;
		d.h=r.h;
		SDL_BlitSurface(tmp,0,dest,&d);
		SDL_FreeSurface(tmp);
	} /* if */ 
} /* CTile::draw_bicolor */ 


void CTile::draw_scaled(int x,int y,SDL_Surface *dest,float scale)
{
	if (orig!=0) sge_transform(orig,dest,0,scale,scale,0,0,x,y,0);

/*
	if (orig!=0) {
		SDL_Rect d;
		SDL_Surface *res=zoomSurface(orig, scale, scale, 0);
		
		d.x=x;
		d.y=y;
		d.w=res->w;
		d.h=res->h;
		SDL_BlitSurface(res,0,dest,&d);

		SDL_FreeSurface(res);
	}
*/ 
} /* CTile::draw_scaled */ 


void CTile::draw_mask(int x,int y,SDL_Surface *dest)
{
	SDL_Rect d;

	if (orig!=0) {
		if (mask_visualization==0) {
			int i,j;

			mask_visualization=SDL_CreateRGBSurface(SDL_HWSURFACE,r.w,r.h,32,0,0,0,0);
			for(i=0;i<r.w;i++) {
				for(j=0;j<r.h;j++) {
					Uint32 color;
                    Uint8 r,g,b,a;
                    
					SDL_LockSurface(orig);
                    color=getpixel(orig,i,j);
					SDL_UnlockSurface(orig);
                    SDL_GetRGBA(color,orig->format,&r,&g,&b,&a);

                    color=SDL_MapRGBA(mask_visualization->format,a,a,a,0);
					SDL_LockSurface(mask_visualization);
                    putpixel(mask_visualization,i,j,color);		
					SDL_UnlockSurface(mask_visualization);
				} /* for */ 
			} /* for */ 
		} /* if */ 

		d.x=x;
		d.y=y;
		d.w=r.w;
		d.h=r.h;
		SDL_BlitSurface(mask_visualization,0,dest,&d);
	} /* if */ 
} /* CTile::draw_mask */ 


void CTile::clear(void)
{
	orig=0;
	mask_visualization=0;
	mask_collision=0;
	collision_data=0;
} /* CTile::clear */ 


void CTile::free(void)
{
	if (orig!=0) SDL_FreeSurface(orig);
	orig=0;
	if (mask_visualization!=0) SDL_FreeSurface(mask_visualization);
	mask_visualization=0;

	if (mask_collision!=0) SDL_FreeSurface(mask_collision);
	mask_collision=0;

	if (collision_data!=0) sge_destroy_cmap(collision_data);
	collision_data=0;
} /* CTile::free */ 


void CTile::instance(CTile *t)
{
	r=t->r;
	orig=t->orig;
	mask_visualization=t->mask_visualization;
	mask_collision=t->mask_collision;
	collision_data=t->collision_data;

} /* CTile::instace */ 





TILE_SOURCE::TILE_SOURCE(void)
{
	fname=0;
	sfc=0;
} /* TILE_SOURCE::TILE_SOURCE */ 


TILE_SOURCE::TILE_SOURCE(char *filename)
{
	SDL_Surface *tmp_sfc;

	fname=new char[strlen(filename)+1];
	strcpy(fname,filename);
	tmp_sfc=IMG_Load(fname);

	sfc = SDL_CreateRGBSurface(SDL_HWSURFACE,tmp_sfc->w,tmp_sfc->h,32,0,0,0,AMASK);
	SDL_SetAlpha(sfc,0,SDL_ALPHA_OPAQUE);
	SDL_BlitSurface(tmp_sfc,0,sfc,0);
	SDL_FreeSurface(tmp_sfc);
} /* TILE_SOURCE::TILE_SOURCE */ 


TILE_SOURCE::~TILE_SOURCE(void)
{
	delete fname;
	fname=0;
	SDL_FreeSurface(sfc);
} /* TILE_SOURCE::~TILE_SOURCE */ 


bool TILE_SOURCE::save(FILE *fp)
{
	fprintf(fp,"%s\n",fname);

	return true;
} /* TILE_SOURCE::save */ 


bool TILE_SOURCE::load(FILE *fp)
{
	char tmp[256];
	SDL_Surface *tmp_sfc;

	if (1!=fscanf(fp,"%s",tmp)) return false;

	if (fname!=0) delete fname;
	fname=new char[strlen(tmp)+1];
	strcpy(fname,tmp);
	tmp_sfc=IMG_Load(fname);

	sfc = SDL_CreateRGBSurface(SDL_HWSURFACE,tmp_sfc->w,tmp_sfc->h,32,0,0,0,AMASK);
	SDL_SetAlpha(sfc,0,SDL_ALPHA_OPAQUE);
	SDL_BlitSurface(tmp_sfc,0,sfc,0);
	SDL_FreeSurface(tmp_sfc);

	return true;
} /* TILE_SOURCE::load */ 


bool TILE_SOURCE::cmp(char *n)
{
	if (strcmp(n,fname)==0) return true;

	return false;
} /* TILE_SOURCE::cmp */ 


