#include "math.h"
#include "stdlib.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#include "sound.h"

#include "CTile.h"
#include "CObject.h"
#include "CGame.h"

extern int MIN_SPEED;
extern int MAX_SPEED;
extern int MAX_ACCEL_RATE;
extern int BRAKE_RATE;
extern int BRAKE_RATE_NO_FUEL;
extern int MAX_HSPEED;
extern int BOUNCE_HSPEED;
extern int MAX_FUEL;
extern int FUEL_RECHARGE;
extern int FUEL_LOSS;

extern int ENEMY_SPEED;

const int explosion_tiles=1;


CPlayerCarObject::CPlayerCarObject(int nx,int ny,List<CTile> *l,int first_tile,int last_tile,int lk,int rk,int fk,int sc,int init_delay,CGame *g)
{
	int i;
	ntiles=(last_tile-first_tile)+1;
	tiles=new CTile *[(last_tile-first_tile)+1];

	for(i=0;i<(last_tile-first_tile)+1;i++) tiles[i]=l->operator [](i+first_tile);

	game=g;
	tile=car_tile(0);
	old_angle=rotating_angle=0;

	x=nx;
	y=ny;
	constitution=CONSTITUTION_PLAYER|CONSTITUTION_CAR;

	y_speed=0;
	y_precision=0;

	x_speed=0;
	x_precision=0;

	fuel=int(MAX_FUEL*0.95);
	score=sc;

	left_key=lk;
	right_key=rk;
	fire_key=fk;

	state=0;
	state_timmer=init_delay;
	blinking_time=init_delay;

	bonus=0;
	next_bonus=300;
	bonus_timmer=0;

	last_collision=0;

	sound_timmer=0;

	enginesound_channel=-1;
	if (game->S_carengine != 0) {
		S_carengine_working.allocated=1;
		S_carengine_working.abuf=new Uint8[game->S_carengine->alen];
		S_carengine_working.alen=game->S_carengine->alen;
		S_carengine_working.volume=MIX_MAX_VOLUME;
	}

	skidsound_channel=-1;
	if (game->S_carengine != 0) {
		S_carskid_working.allocated=1;
		S_carskid_working.abuf=new Uint8[game->S_carskid->alen];
		S_carskid_working.alen=game->S_carskid->alen;
		S_carskid_working.volume=MIX_MAX_VOLUME;
	}

	goal_reached=false;
} /* CPlayerCarObject::CPlayerCarObject */ 


CPlayerCarObject::~CPlayerCarObject(void)
{
} /* CPlayerCarObject::~CPlayerCarObject */ 



bool CPlayerCarObject::cycle(unsigned char *keyboard,unsigned char *old_keyboard)
{
	float f=float(-y_speed)/MAX_SPEED;

	if (bonus_timmer>0) bonus_timmer--;

	if (last_collision) {
		if (state!=4) {
			/* Collision with a car: */ 
			if (last_collision->get_x()<x) state=5;
									  else state=6;
			state_timmer=0;
			last_collision=0;
		} else {
			last_collision=0;
		} /* if */ 
	} /* if */ 

	switch(state) {
	case 0:
		/* STARTING: */ 
		rotating_angle=0;
		x_speed=0;
		y_speed=0;
		state_timmer--;
		if (state_timmer==0) state=1;

		break;

	case 1:
		/* DRIVING */ 
		rotating_angle=0;
		tile=car_tile(0);
		if (y>-32) {
			score+=(-y_speed)>>10;

			if (goal_reached && y_speed<ENEMY_SPEED) {
				y_speed+=MAX_ACCEL_RATE;
			} else {

				if (fuel>0) {
					/* Accelerate/Brake: */ 
					if (keyboard[fire_key]) {
						float f=float(-y_speed)/MAX_SPEED;
						y_speed+=int((1.0-f)*MAX_ACCEL_RATE);
					} else {
						y_speed+=BRAKE_RATE;
					} /* if */ 
				} else {
					y_speed+=BRAKE_RATE_NO_FUEL;
				} /* if */ 
			} /* if */ 


			/* Move left/right: */ 
			if (f<0.1) f=f*2;
				  else f=(((f-0.1F)/0.9F)*0.8F)+0.2F;

			if (keyboard[left_key]) {
				if (!keyboard[right_key] ||
					!old_keyboard[left_key] ||
					(x_speed<0 && !old_keyboard[right_key])) {
					x_speed=-int(MAX_HSPEED*f);
				} /* if */ 
			} /* if */ 

			if (keyboard[right_key]) {
				if (!keyboard[left_key] ||
					!old_keyboard[right_key] ||
					(x_speed>0 && !old_keyboard[left_key])) {
					x_speed=int(MAX_HSPEED*f);
				} /* if */ 
			} /* if */ 

			if (!keyboard[right_key] && !keyboard[left_key]) x_speed=0;

		} else {
			y_speed+=BRAKE_RATE_NO_FUEL;
		} /* if */ 

		break;

	case 2:
		/* BOUNCING RIGHT: */ 
		 rotating_angle=0;
		tile=car_tile(0);
		if (y>0) {
			score+=(-y_speed)>>10;

			if (fuel>0) {
				/* Accelerate/Brake: */ 
				if (keyboard[fire_key]) {
					float f=float(-y_speed)/MAX_SPEED;
					y_speed+=int((1.0-f)*MAX_ACCEL_RATE);
				} else {
					y_speed+=BRAKE_RATE;
				} /* if */ 
			} else {
				y_speed+=BRAKE_RATE_NO_FUEL;
			} /* if */ 

		} else {
			y_speed+=BRAKE_RATE_NO_FUEL;
		} /* if */ 

		x_speed=BOUNCE_HSPEED;
		state_timmer++;
		if (state_timmer>=10) state=1;
		break;

	case 3:
		/* BOUNCING LEFT: */ 
		rotating_angle=0;
		tile=car_tile(0);
		if (y>0) {
			score+=(-y_speed)>>10;

			if (fuel>0) {
				/* Accelerate/Brake: */ 
				if (keyboard[fire_key]) {
					float f=float(-y_speed)/MAX_SPEED;
					y_speed+=int((1.0-f)*MAX_ACCEL_RATE);
				} else {
					y_speed+=BRAKE_RATE;
				} /* if */ 
			} else {
				y_speed+=BRAKE_RATE_NO_FUEL;
			} /* if */ 

		} else {
			y_speed+=BRAKE_RATE_NO_FUEL;
		} /* if */ 

		x_speed=-BOUNCE_HSPEED;
		state_timmer++;
		if (state_timmer>=10) state=1;
		break;

	case 4:
		if (state_timmer==0) {
			CObject *o=new CExplosionObject(x-16,y-32,&game->explosion_tiles,0,11,game);
			game->objects.Add(o);
		} /* if */ 
		rotating_angle=0;
		next_bonus=300;
		x_speed=0;
		y_speed=0;
		state_timmer++;
		tile=ntiles-1;		
		if (state_timmer>=75) {
			blinking_time=32;
			tile=car_tile(0);
			state=1;
			/* Look for the center of the road: */ 
			if (!game->object_collision(8,0,this,CONSTITUTION_SOLID)!=0) {
				int i,j;
				bool found=false;

				for(i=4;!found && draw_x+i<game->get_dx();i+=4) {
					if (game->object_collision(i,0,this,CONSTITUTION_SOLID)!=0) {
						j=x+i;
						found=true;
					} /* if */ 
				} /* for */ 
				if (found) {
					draw_x=x=((draw_x+4)+(j-4))/2;
				} else {
					draw_x=x=(game->get_dx()/2)-16;
				} /* if */ 

			} else {
				if (!game->object_collision(-8,0,this,CONSTITUTION_SOLID)!=0) {
					int i,j;
					bool found=false;

					for(i=-4;!found && draw_x+i>0;i-=4) {
						if (game->object_collision(i,0,this,CONSTITUTION_SOLID)!=0) {
							j=x+i;
							found=true;
						} /* if */ 
					} /* for */ 
					if (found) {
						draw_x=x=((draw_x-4)+(j+4))/2;
					} else {
						draw_x=x=(game->get_dx()/2)-16;
					} /* if */ 
				} else {
					draw_x=x=(game->get_dx()/2)-8;
				} /* if */ 
			} /* if */ 
		} /* if */ 
		break;

	case 5:
		/* SLIDDING RIGHT: */ 
		x_speed=MAX_HSPEED;
		if (state_timmer<16) {
			if (keyboard[right_key] && !old_keyboard[right_key]) state=1;
			rotating_angle=-45;
		} else {
			if ((-y_speed)<=0.75F*MAX_SPEED) state=1;
									    else rotating_angle-=10;
		} /* if */ 
		tile=car_tile(rotating_angle);
		state_timmer++;
		break;

	case 6:
		/* SLIDDING LEFT: */ 
		x_speed=-MAX_HSPEED;
		if (state_timmer<16) {
			if (keyboard[left_key] && !old_keyboard[left_key]) state=1;
			rotating_angle=45;
		} else {
			if ((-y_speed)<=0.75F*MAX_SPEED) state=1;
										else rotating_angle+=10;
		} /* if */ 
		tile=car_tile(rotating_angle);
		state_timmer++;
		break;

	} /* switch */ 

	sound_timmer++;
	if (fuel<int(0.15F*MAX_FUEL) && fuel>0 &&  
		(sound_timmer%45)==0) {
		if (game->S_fuelempty!=0) Sound_play(game->S_fuelempty);
	} /* if */ 

	if (state!=4 && y>0) {
		if (fuel>0 && game->S_carengine!=0) {
			if ((sound_timmer&0x07)==0) {

				float f;
				unsigned int i,j,k;
				Sint16 *ip,*ip2;
/*
				// 2 octave range: 0.25 - 1.0
				f=(3.0F*(float(-y_speed)/MAX_SPEED)+1.0F)/4.0F;
				if (f<0.25F) f=0.25F;
				if (f>1.0F) f=1.0F;
*/
/*
				// 1 octave range: 0.5 - 1.0
				f=((float(-y_speed)/MAX_SPEED)+1.0F)/2.0F;
				if (f<0.5F) f=0.5F;
				if (f>1.0F) f=1.0F;
*/


				//  3 tones lower, and end 4 above:	0.8408964 - 1.2599210
				f=0.8408964F + (1.259921F-0.8408964F)*(float(-y_speed)/MAX_SPEED);
				if (f<0.8408964F) f=0.8408964F;
				if (f>1.2599210F) f=1.2599210F;


				ip=(Sint16 *)S_carengine_working.abuf;
				ip2=(Sint16 *)game->S_carengine->abuf;
				for(i=0,j=0;i<game->S_carengine->alen && 
							((unsigned int)(j*f))*4<game->S_carengine->alen;i+=4,j++) {
					k=(unsigned int)(j*f);
					if (game->focusing_objects.Length()==1) {
						ip[j*2]=ip2[k*2];		/* L */ 
						ip[j*2+1]=ip2[k*2+1];	/* R */ 
					} else {
						if (game->focusing_objects[0]==this) {
							ip[j*2]=0;							/* L */ 
							ip[j*2+1]=(ip2[k*2+1]+ip2[k*2])>>1;	/* R */ 
						} else {
							ip[j*2]=(ip2[k*2+1]+ip2[k*2])>>1;		/* L */ 
							ip[j*2+1]=0;							/* R */ 
						} /* if */ 
					} /* if */ 
				} /* if */ 

				if (enginesound_channel==-1) {
					enginesound_channel=Mix_PlayChannel(-1,&S_carengine_working,0);
				} else {
					Mix_HaltChannel(enginesound_channel);
					Mix_PlayChannel(enginesound_channel,&S_carengine_working,0);
				} /* if */ 

				if (state==5 || state==6) {
					float f;
					unsigned int i,j,k;
					Sint16 *ip,*ip2;

					if (state_timmer<16) {
						f=1.0;
					} else {
						f=1.5;
					} /* if */ 

					ip=(Sint16 *)S_carskid_working.abuf;
					ip2=(Sint16 *)game->S_carskid->abuf;
					for(i=0,j=0;i<game->S_carskid->alen && 
								((unsigned int)(j*f))*4<game->S_carskid->alen;i+=4,j++) {
						k=(unsigned int)(j*f);
						ip[j*2]=ip2[k*2];		/* L */ 
						ip[j*2+1]=ip2[k*2+1];	/* R */ 
					} /* if */ 

					if (skidsound_channel==-1) {
						skidsound_channel=Mix_PlayChannel(-1,&S_carskid_working,0);
					} else {
						Mix_HaltChannel(skidsound_channel);
						Mix_PlayChannel(skidsound_channel,&S_carskid_working,0);
					} /* if */ 
				} else {
					if (skidsound_channel!=-1) {
						Mix_HaltChannel(skidsound_channel);
						skidsound_channel=-1;
					} /* if */ 
				} /* if */ 
			} /* if */ 
		} /* if */ 
	} else {
		if (enginesound_channel!=-1) {
			Mix_HaltChannel(enginesound_channel);
			enginesound_channel=-1;
		} /* if */ 
		if (skidsound_channel!=-1) {
			Mix_HaltChannel(skidsound_channel);
			skidsound_channel=-1;
		} /* if */ 
	} /* if */ 

	if (state!=4 && state!=0) {
		/* COMMON TO ALL STATES: */ 
		CObject *o=0;

		if (goal_reached && fuel<=0) fuel=1;

		/* Lose fuel: */ 
		if (fuel>0 && !goal_reached) {
			fuel--;
			if (fuel>=MAX_FUEL) fuel=MAX_FUEL;
			if (fuel<0) fuel=0;
		} /* if */ 

		/* Take fuel: */ 
		o=game->object_collision(0,0,this,CONSTITUTION_FUEL);
		if (o!=0) {
			fuel+=FUEL_RECHARGE;
			if (fuel>=MAX_FUEL) fuel=MAX_FUEL;
			game->todelete.Add(o);
			bonus=next_bonus;
			score+=next_bonus;
			bonus_timmer=64;
			if (next_bonus==800) next_bonus=1000;
			if (next_bonus==500) next_bonus=800;
			if (next_bonus==300) next_bonus=500;
			if (game->S_takefuel!=0) Sound_play(game->S_takefuel);
		} /* if */ 

		/* Wall collision: */ 
		o=game->object_collision(0,0,this,CONSTITUTION_SOLID);
		if (o!=0) {
			/* CRASH AGAINST A ROAD WALL: */ 
			if ((-y_speed)>0.75F*MAX_SPEED ||
				o->constitution_test(CONSTITUTION_CAR)) {
				if (fuel>FUEL_LOSS*2) fuel-=FUEL_LOSS;
				state=4;
				state_timmer=0;
				y_speed=0;
				x_speed=0;
				if (game->S_crash!=0) Sound_play(game->S_crash);
			} else {
				if (game->object_collision(8,0,this,CONSTITUTION_SOLID)!=0) {
					state_timmer=0;
					state=3;
					if (game->S_collision!=0) Sound_play(game->S_collision);
				} else {
					if (game->object_collision(-8,0,this,CONSTITUTION_SOLID)!=0) {
						state_timmer=0;
						state=2;
						if (game->S_collision!=0) Sound_play(game->S_collision);
					} else {
						/* CRASH ???????? */ 
						state=4;
						state_timmer=0;
						y_speed=0;
						x_speed=0;
						if (game->S_crash!=0) Sound_play(game->S_crash);
					} /* if */ 
				} /* if */ 
			} /* if */ 
		} else {
			/* Car collision: */ 
			o=game->object_collision(0,0,this,CONSTITUTION_CAR);
			if (o!=0) {
				CCarObject *co=(CCarObject *)o;

				if (last_collision==0 && game->S_collision!=0) Sound_play(game->S_collision);

				co->car_collision(this);
				car_collision(co);
			} else {
				o=game->object_collision(0,0,this,CONSTITUTION_OIL);
				if (o!=0 && (-y_speed)>0.75*MAX_SPEED) {
					last_collision=this;
				} else {
					o=game->object_collision(0,0,this,CONSTITUTION_WATER);
					if (o!=0) {
						if ((-y_speed)>0.75*MAX_SPEED) {
							if (game->S_water!=0) Sound_play(game->S_water);
							y_speed=-int(0.5*MAX_SPEED);
						} /* if */ 
					} /* if */ 
				} /* if */ 
			} /* if */ 
		} /* if */ 
	} /* if */ 

	if (blinking_time>0) blinking_time--;

	if (y_speed>-MIN_SPEED) y_speed=-MIN_SPEED;
	if (y_speed<-MAX_SPEED) y_speed=-MAX_SPEED;

	/* TYRE MARKS: */ 
	if (game->game_remake_extras) {
		if ((state==5 || state==6) && tile<8) {
			int a,b,c,d;
			int e,f,g,h;

			tyre_coordinates(rotating_angle,&a,&b,&c,&d);
			tyre_coordinates(old_angle,&e,&f,&g,&h);

			/* Create new mark: */ 
			CTyreMark *r;
			
			r=new CTyreMark();
			r->x=x+e;
			r->y=y+f;
			r->x2=compute_next_x()+a;
			r->y2=compute_next_y()+b;
			if (r->y!=r->y2) game->tyre_marks.Add(r);
						else delete r;

			r=new CTyreMark();
			r->x=x+g;
			r->y=y+h;
			r->x2=compute_next_x()+c;
			r->y2=compute_next_y()+d;
			if (r->y!=r->y2) game->tyre_marks.Add(r);
						else delete r;

		} /* if */ 
	} /* if */ 

	CCarObject::cycle(keyboard,old_keyboard);

	if (y<-33) y=-33;

	old_angle=rotating_angle;

	return true;
} /* CPlayerCarObject::cycle */ 


void CPlayerCarObject::draw(int sx,int sy,SDL_Surface *screen)
{
	draw_x=x;
	draw_y=y;

	if (tile>=0 && tile<ntiles) {
		if (blinking_time>0) {
			int f=int(sin(double(blinking_time)/2)*50+50);
			tiles[tile]->draw_shaded(x-sx,y-sy,screen,f,-1,-1,-1,0);
		} else {
			tiles[tile]->draw(x-sx,y-sy,screen);
		} /* if */ 
	} /* if */ 

	if (fuel<=0) {
		game->extra_tiles[1]->draw((x-sx)+16-game->extra_tiles[1]->get_dx()/2,
								   (y-sy)-game->extra_tiles[1]->get_dy(),
								   screen);
	} else {
		if (bonus>0 && bonus_timmer>0) {
			int j=2;
			if (bonus==500) j=3;
			if (bonus==800) j=4;
			if (bonus==1000) j=5;
			game->extra_tiles[j]->draw((x-sx)+16-game->extra_tiles[j]->get_dx()/2,
								   (y-sy)-game->extra_tiles[j]->get_dy(),
								   screen);
		} /* if */ 
	} /* if */ 

} /* CPlayerCarObject::draw */ 

int CPlayerCarObject::car_tile(int angle)
{
	long nt=ntiles-explosion_tiles;

	while(angle<0) angle+=360;
	while(angle>=360) angle-=360;

	return (angle*nt)/360;
} /* CPlayerCarObject::car_tile */ 


void CPlayerCarObject::tyre_coordinates(int angle,int *x1,int *y1,int *x2,int *y2)
{
	int x1v[8]={6,16,23,27,  21,16,8,5};
	int y1v[8]={7,3,8,16,    25,25,22,12};
	int x2v[8]={21,27,23,16, 6,5,8,16};
	int y2v[8]={7,12,22,25,  25,16,8,3};
	long nt=ntiles-explosion_tiles;
	float tmp;
	int tile,tile2;

	while(angle<0) angle+=360;
	while(angle>=360) angle-=360;

	tmp=float(float(angle)*float(nt))/360.0F;
	tile=int(floor(tmp));
	tmp=tmp-tile;
	tile2=(tile+1)%8;

	*x1=int(x1v[tile2]*tmp+x1v[tile]*(1.0-tmp));
	*y1=int(y1v[tile2]*tmp+y1v[tile]*(1.0-tmp));
	*x2=int(x2v[tile2]*tmp+x2v[tile]*(1.0-tmp));
	*y2=int(y2v[tile2]*tmp+y2v[tile]*(1.0-tmp));
} /* CPlayerCarObject::tyre_coordinates */ 


void CPlayerCarObject::reach_goal(void)
{
	goal_reached=true;
} /* CPlayerCarObject::reach_goal */ 
