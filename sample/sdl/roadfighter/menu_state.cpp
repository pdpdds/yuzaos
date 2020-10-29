#ifdef _WIN32
//#include <windows.h>
//#include <windowsx.h>
#else
#include <sys/time.h>
#include <time.h>
#define strupr(x) (x)
// TODO: really find a replacement for strupr
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

#include "sound.h"

#include "sge/sge.h"
#include "CTile.h"
#include "CObject.h"
#include "CGame.h"
#include "CRoadFighter.h"
#include "auxiliar.h"

const int EFFECT_LENGTH=25;
const int TEXT_EFFECT_LENGTH=16;

extern int start_level;

int CRoadFighter::menu_cycle(void)
{
	if (state_timmer==0) {

		menu_effect=(rand()%3)+1;	/* 1,2 ?3 */ 
		menu_state=0;
		menu_timmer=0;
		menu_current_menu=0;
		menu_redefining_key=-1;
		if (S_menu_in!=0) Sound_play(S_menu_in);
		Sound_create_music("sound/menu_theme",-1);
		menu_credits_timmer=0;
	} /* if */ 

	switch(menu_state) {
	/* Appearing effect: */ 
	case 0:
		menu_timmer++;
		//Sound_music_volume((menu_timmer*MIX_MAX_VOLUME)/EFFECT_LENGTH);
		if (menu_timmer>=EFFECT_LENGTH) {
			//Sound_music_volume(MIX_MAX_VOLUME);
			menu_state=1;
			menu_timmer=0;
		} /* if */ 
		break;

	/* Appearing text: */ 
	case 1:
		menu_timmer++;
		if (menu_timmer>=TEXT_EFFECT_LENGTH) {
			menu_state=2;
			menu_timmer=0;
			menu_item=0;
		} /* if */ 
		break;

	/* Browsing menu: */ 
	case 2:
		if (menu_redefining_key!=-1) {
			int i;
			bool found=false;

			for(i=0;!found && i<SDLK_LAST;i++) {
				if (keyboard[i] && !old_keyboard[i]) {
					if ((SDLKey)i!=SDLK_ESCAPE &&
						(SDLKey)i!=SDLK_F12) {
						switch(menu_redefining_key) {
						case 0:if (i!=left_key && i!=fire_key) right_key=(SDLKey)i;
							   break;
						case 1:if (i!=right_key && i!=fire_key) left_key=(SDLKey)i;
							   break;
						case 2:if (i!=right_key && i!=left_key) fire_key=(SDLKey)i;
							   break;

						case 3:if (i!=left2_key && i!=fire2_key) right2_key=(SDLKey)i;
							   break;
						case 4:if (i!=right2_key && i!=fire2_key) left2_key=(SDLKey)i;
							   break;
						case 5:if (i!=right2_key && i!=left2_key) fire2_key=(SDLKey)i;
							   break;
						} /* switch */ 
					} /* if */ 
					found=true;
					menu_redefining_key=-1;
				} /* if */ 
			} /* for */ 
		} else {
			if (keyboard[SDLK_DOWN] && !old_keyboard[SDLK_DOWN]) {
				Sound_play(S_menu_move);
				menu_item++;
			} /* if */ 
			if (keyboard[SDLK_UP] && !old_keyboard[SDLK_UP]) {
				Sound_play(S_menu_move);
				menu_item--;
			} /* if */ 
			if (menu_item<0) menu_item=0;
			if (menu_item>menu_nitems-1) menu_item=menu_nitems-1;

			if ((keyboard[fire_key] && !old_keyboard[fire_key]) ||
				(keyboard[SDLK_SPACE] && !old_keyboard[SDLK_SPACE]) ||
				(keyboard[right_key] && !old_keyboard[right_key]) ||
				(keyboard[left_key] && !old_keyboard[left_key]) ||
				(keyboard[SDLK_ESCAPE] && !old_keyboard[SDLK_ESCAPE])) {
				if ((keyboard[left_key] && !old_keyboard[left_key]) ||
					(keyboard[SDLK_ESCAPE] && !old_keyboard[SDLK_ESCAPE])) {
					menu_item=menu_nitems-1;
				} /* if */ 

				Sound_play(S_menu_select);
				switch(menu_current_menu) {
				case 0:
					menu_state=3;
					menu_timmer=TEXT_EFFECT_LENGTH;
					break;
				case 1:
					if (menu_item==2) {
						if (game_remake_extras) game_remake_extras=false;
										   else game_remake_extras=true;
					} /* if */ 
					if (menu_item==3) {
						default_configuration();
					} /* if */ 

					if (menu_item==0 || menu_item==1 || menu_item==4) {
						menu_state=3;
						menu_timmer=TEXT_EFFECT_LENGTH;
					} /* if */ 
					break;
				case 2:
					if (menu_item==0) menu_redefining_key=0;
					if (menu_item==1) menu_redefining_key=1;
					if (menu_item==2) menu_redefining_key=2;

					if (menu_item==3) {
						menu_state=3;
						menu_redefining_key=-1;
						menu_timmer=TEXT_EFFECT_LENGTH;
					} /* if */ 
					break;
				case 3:
					if (menu_item==0) menu_redefining_key=3;
					if (menu_item==1) menu_redefining_key=4;
					if (menu_item==2) menu_redefining_key=5;

					if (menu_item==3) {
						menu_state=3;
						menu_redefining_key=-1;
						menu_timmer=TEXT_EFFECT_LENGTH;
					} /* if */ 
					break;
				case 4:
					menu_state=3;
					menu_timmer=TEXT_EFFECT_LENGTH;
					break;
				case 5:
					menu_state=3;
					menu_timmer=TEXT_EFFECT_LENGTH;
					break;
				} /* switch */ 

			} /* if */ 
		} /* if */ 
		break;

	/* Disappearing text: */ 
	case 3:
		menu_timmer--;
		if (menu_timmer<0) {
			switch(menu_current_menu) {
			case 0:
				if (menu_item==0) {
					menu_state=1;
					menu_timmer=0;
					menu_current_menu=4;
				} /* if */ 
				if (menu_item==1) {
					menu_state=1;
					menu_timmer=0;
					menu_current_menu=5;
				} /* if */ 
				if (menu_item==2) {
					menu_state=1;
					menu_timmer=0;
					menu_current_menu=1;
				} /* if */ 
				if (menu_item==3) {
					menu_state=4;
					if (S_menu_out!=0) Sound_play(S_menu_out);
					menu_timmer=EFFECT_LENGTH;
				} /* if */ 
				break;
			case 1:
				if (menu_item==0) {
					menu_state=1;
					menu_timmer=0;
					menu_redefining_key=-1;
					menu_current_menu=2;
				} /* if */ 
				if (menu_item==1) {
					menu_state=1;
					menu_timmer=0;
					menu_redefining_key=-1;
					menu_current_menu=3;
				} /* if */ 
				if (menu_item==4) {
					menu_state=1;
					menu_timmer=0;
					menu_current_menu=0;
					save_configuration();
				} /* if */ 
				break;
			case 2:
			case 3:
				if (menu_item==3) {
					menu_state=1;
					menu_timmer=0;
					menu_current_menu=1;
				} /* if */ 
				break;
			case 4:
			case 5:
				if (menu_item==3) {
					menu_state=1;
					menu_timmer=0;
					menu_current_menu=0;
				} else {
					menu_state=4;
					if (S_menu_out!=0) Sound_play(S_menu_out);
					menu_timmer=EFFECT_LENGTH;
				} /* if */ 
				break;
			} /* switch */ 
		} /* if */ 
		break;

	/* Disappearing menu: */ 
	case 4:
		menu_timmer--;

		if (menu_timmer>=0) Sound_music_volume((menu_timmer*MIX_MAX_VOLUME)/EFFECT_LENGTH);
		if (menu_timmer<0) {

			switch(menu_current_menu) {
			case 0:
				if (menu_item==3) {
					Sound_release_music();
					return QUIT_STATE;
				} /* if */ 
				break;
			case 4:
				if (menu_item==0) {
					game_mode=0;
					n_players=1;
					current_level=start_level;
					scoreboard_x=-1;
					interlevel_state=0;
					interlevel_timmer=0;
					Sound_release_music();
					return INTERLEVEL_STATE;
				} /* if */ 
				if (menu_item==1) {
					game_mode=1;
					n_players=1;
					current_level=start_level;
					scoreboard_x=-1;
					interlevel_state=0;
					interlevel_timmer=0;
					Sound_release_music();
					return INTERLEVEL_STATE;
				} /* if */ 
				if (menu_item==2) {
					game_mode=2;
					n_players=1;
					current_level=start_level;
					scoreboard_x=-1;
					interlevel_state=0;
					interlevel_timmer=0;
					Sound_release_music();
					return INTERLEVEL_STATE;
				} /* if */ 
				break;
			case 5:
				if (menu_item==0) {
					game_mode=0;
					n_players=2;
					current_level=start_level;
					scoreboard_x=-1;
					interlevel_state=0;
					interlevel_timmer=0;
					n_players=2;
					Sound_release_music();
					return INTERLEVEL_STATE;
				} /* if */ 
				if (menu_item==1) {
					game_mode=1;
					n_players=2;
					current_level=start_level;
					scoreboard_x=-1;
					interlevel_state=0;
					interlevel_timmer=0;
					Sound_release_music();
					return INTERLEVEL_STATE;
				} /* if */ 
				if (menu_item==2) {
					game_mode=2;
					n_players=2;
					current_level=start_level;
					scoreboard_x=-1;
					interlevel_state=0;
					interlevel_timmer=0;
					Sound_release_music();
					return INTERLEVEL_STATE;
				} /* if */ 
				break;
			} /* switch */ 
		} /* if */ 
		break;
	} /* break */ 

	if (menu_state==1 || menu_state==2 || menu_state==3) {
		if (menu_current_menu==0) {
			delete menu_tittle_text;
			menu_tittle_text=new char[strlen("PLAY SELECT:")+1];
			strcpy(menu_tittle_text,"PLAY SELECT:");
			delete menu_options_text;
			menu_options_text=new char[strlen("ONE PLAYER\nTWO PLAYERS\nOPTIONS\nQUIT\n")+1];
			strcpy(menu_options_text,"ONE PLAYER\nTWO PLAYERS\nOPTIONS\nQUIT\n");
			menu_nitems=4;
		} /* if */ 

		if (menu_current_menu==1) {
			char tmp[256];
			delete menu_tittle_text;
			menu_tittle_text=new char[strlen("OPTIONS:")+1];
			strcpy(menu_tittle_text,"OPTIONS:");
			delete menu_options_text;
			sprintf(tmp,"PLAYER 1 KEYS\nPLAYER 2 KEYS\nEXTRAS: %s\nDEFAULT\nBACK\n",(game_remake_extras ? "ON":"OFF"));
			menu_options_text=new char[strlen(tmp)+1];
			strcpy(menu_options_text,tmp);
			menu_nitems=5;
		} /* if */ 

		if (menu_current_menu==2) {
			char tmp[256];
			delete menu_tittle_text;
			menu_tittle_text=new char[strlen("PLAYER 1:")+1];
			strcpy(menu_tittle_text,"PLAYER 1:");
			delete menu_options_text;
			sprintf(tmp,"RIGHT : %s\nLEFT : %s\nFIRE : %s\nBACK\n",
					(menu_redefining_key==0 ? "":strupr(SDL_GetKeyName((SDLKey)right_key))),
					(menu_redefining_key==1 ? "":strupr(SDL_GetKeyName((SDLKey)left_key))),
					(menu_redefining_key==2 ? "":strupr(SDL_GetKeyName((SDLKey)fire_key))));
			menu_options_text=new char[strlen(tmp)+1];
			strcpy(menu_options_text,tmp);
			menu_nitems=4;
		} /* if */ 
		if (menu_current_menu==3) {
			char tmp[256];
			delete menu_tittle_text;
			menu_tittle_text=new char[strlen("PLAYER 2:")+1];
			strcpy(menu_tittle_text,"PLAYER 2:");
			delete menu_options_text;
			sprintf(tmp,"RIGHT : %s\nLEFT : %s\nFIRE : %s\nBACK\n",
					(menu_redefining_key==3 ? "":strupr(SDL_GetKeyName((SDLKey)right2_key))),
					(menu_redefining_key==4 ? "":strupr(SDL_GetKeyName((SDLKey)left2_key))),
					(menu_redefining_key==5 ? "":strupr(SDL_GetKeyName((SDLKey)fire2_key))));
			menu_options_text=new char[strlen(tmp)+1];
			strcpy(menu_options_text,tmp);
			menu_nitems=4;
		} /* if */ 
		if (menu_current_menu==4) {
			char tmp[256];
			delete menu_tittle_text;
			menu_tittle_text=new char[strlen("ONE PLAYER:")+1];
			strcpy(menu_tittle_text,"ONE PLAYER:");
			delete menu_options_text;
			sprintf(tmp,"LEVEL A\nLEVEL B\nLEVEL C\nBACK\n");
			menu_options_text=new char[strlen(tmp)+1];
			strcpy(menu_options_text,tmp);
			menu_nitems=4;
		} /* if */ 
		if (menu_current_menu==5) {
			char tmp[256];
			delete menu_tittle_text;
			menu_tittle_text=new char[strlen("TWO PLAYERS:")+1];
			strcpy(menu_tittle_text,"TWO PLAYERS:");
			delete menu_options_text;
			sprintf(tmp,"LEVEL A\nLEVEL B\nLEVEL C\nBACK\n");
			menu_options_text=new char[strlen(tmp)+1];
			strcpy(menu_options_text,tmp);
			menu_nitems=4;
		} /* if */ 
	} /* if */ 

	return MENU_STATE;
} /* CRoadFighter::menu_cycle */ 



void CRoadFighter::menu_draw(SDL_Surface *screen)
{
	float y_position=0.55F;

	switch(menu_state) {
	/* Appearing/Disappearing effect: */ 
	case 0:
	case 4:
		switch(menu_effect) {
		/* Fade: */ 
		case 0:
			{
				float f=float(menu_timmer)*(1.0F/float(EFFECT_LENGTH));
				if (f<0.0) f=0.0;
				if (f>=1.0) f=1.0;
				SDL_FillRect(screen,0,0);
				SDL_BlitSurface(tittle_sfc,0,screen,0);
				if (f<1.0) surface_fader(screen,f,f,f,0);
			}

			break;

		/* Fall: */ 
		case 1:
			{
				int y=int(screen->h*float(menu_timmer)*(1.0F/float(EFFECT_LENGTH)));
				SDL_Rect r;

				if (y<0) y=0;
				if (y>screen->h) y=screen->h;
				y-=screen->h;
				r.x=0;
				r.y=y;
				r.w=screen->w;
				r.h=screen->h;

				SDL_FillRect(screen,0,0);
				SDL_BlitSurface(tittle_sfc,0,screen,&r);
			}
			break;

		/* Zoom: */ 

		case 2:
			{
				float f=float(menu_timmer)*(1.0F/float(EFFECT_LENGTH));
				if (f<0.0) f=0.0;
				if (f>=1.0) f=1.0;
				SDL_FillRect(screen,0,0);
				sge_transform(tittle_sfc,screen,0,f,f,tittle_sfc->w/2,tittle_sfc->h/2,tittle_sfc->w/2,tittle_sfc->h/2,0);
			}
			break;

		/* Roto-zoom: */ 
		case 3:
			{
				float f=float(menu_timmer)*(1.0F/float(EFFECT_LENGTH));
				float a;
				if (f<0.0) f=0.0;
				if (f>=1.0) f=1.0;
				a=(1.0F-f)*260.0F;
				SDL_FillRect(screen,0,0);
				sge_transform(tittle_sfc,screen,a,f,f,tittle_sfc->w/2,tittle_sfc->h/2,tittle_sfc->w/2,tittle_sfc->h/2,0);
			}
			break;
		} /* switch */ 
		break;

	/* Appearing/Disappearing text: */ 
	case 1:
	case 3:
		{
			SDL_Rect r,r2;
			SDL_Color c,c1;
			SDL_Surface *menu_sfc,*options_sfc;

			/* Draw tittle: */ 
			SDL_FillRect(screen,0,0);
			SDL_BlitSurface(tittle_sfc,0,screen,0);

			c.r=c.g=c.b=255;
			c1.r=c1.g=c1.b=224;
			menu_sfc=TTF_RenderText_Blended(font2big,menu_tittle_text,c);
			if (menu_nitems>5) {
				options_sfc=multiline_text_surface2(menu_options_text,0,font2small,c1,c1,-1,0);
			} else {
				if (menu_nitems>4) {
					options_sfc=multiline_text_surface2(menu_options_text,2,font2medium,c1,c1,-1,0);
				} else {
					options_sfc=multiline_text_surface2(menu_options_text,4,font2big,c1,c1,-1,0);
				} /* if */ 
			} /* if */ 

			/* Draw bar: */ 
			{
				int bar_length=int(menu_sfc->w*1.1);

				r.x=screen->w/2-bar_length/2;
				r.y=int(screen->h*y_position);
				r.w=bar_length;
				r.h=4;

				SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,64,64,64));

				r.x=(screen->w/2-bar_length/2)+1;
				r.y=int(screen->h*y_position)+1;
				r.w=bar_length-2;
				r.h=2;

				SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));
			}

			/* Draw text: */ 
			{
				int tmp;
				float f=float(menu_timmer)*(1.0F/float(TEXT_EFFECT_LENGTH));
				if (f<0.0) f=0.0;
				if (f>=1.0) f=1.0;

				tmp=int(f*menu_sfc->h);

				r.x=0;
				r.y=0;
				r.w=menu_sfc->w;
				r.h=tmp;
				r2.x=screen->w/2-menu_sfc->w/2;
				r2.y=int(screen->h*y_position-(tmp));
				r2.w=menu_sfc->w;
				r2.h=tmp;
				SDL_BlitSurface(menu_sfc,&r,screen,&r2);

				tmp=int(f*options_sfc->h);

				r.x=0;
				r.y=options_sfc->h-tmp;
				r.w=options_sfc->w;
				r.h=tmp;
				r2.x=screen->w/2-options_sfc->w/2;
				r2.y=int(screen->h*y_position+8);
				r2.w=options_sfc->w;
				r2.h=tmp;
				SDL_BlitSurface(options_sfc,&r,screen,&r2);
			}

			SDL_FreeSurface(menu_sfc);
			SDL_FreeSurface(options_sfc);
		}

		break;

	/* Browsing menu: */ 
	case 2:
		{
			SDL_Rect r;
			SDL_Color c,c1;
			SDL_Surface *menu_sfc,*options_sfc;
			int y_inc,y_start;
			float glow;

			/* Draw tittle: */ 
			SDL_FillRect(screen,0,0);
			SDL_BlitSurface(tittle_sfc,0,screen,0);

			c.r=c.g=c.b=255;
			c1.r=c1.g=c1.b=224;
			menu_sfc=TTF_RenderText_Blended(font2big,menu_tittle_text,c);

			glow=(float(fabs(sin(state_timmer/10.0F)))/2.0F)+0.125F;

			if (menu_nitems>5) {
				options_sfc=multiline_text_surface2(menu_options_text,0,font2small,c1,c,menu_item,glow);
				y_inc=int((FONT_SIZE)*0.65F);
				y_start=9;
			} else {
				if (menu_nitems>4) {
					options_sfc=multiline_text_surface2(menu_options_text,2,font2medium,c1,c,menu_item,glow);
					y_inc=int(2+(FONT_SIZE)*0.8F);
					y_start=12;
				} else {
					options_sfc=multiline_text_surface2(menu_options_text,4,font2big,c1,c,menu_item,glow);
					y_inc=(4+FONT_SIZE);
					y_start=15;
				} /* if */ 
			} /* if */ 

			/* Draw bar: */ 
			{
				int bar_length=int(menu_sfc->w*1.1);

				r.x=screen->w/2-bar_length/2;
				r.y=int(screen->h*y_position);
				r.w=bar_length;
				r.h=4;

				SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,64,64,64));

				r.x=(screen->w/2-bar_length/2)+1;
				r.y=int(screen->h*y_position)+1;
				r.w=bar_length-2;
				r.h=2;

				SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));
			}

			/* Draw text: */ 
			{
				r.x=screen->w/2-menu_sfc->w/2;
				r.y=int(screen->h*y_position-(menu_sfc->h));
				r.w=menu_sfc->w;
				r.h=menu_sfc->h;
				SDL_BlitSurface(menu_sfc,0,screen,&r);

				r.x=screen->w/2-options_sfc->w/2;
				r.y=int(screen->h*y_position+8);
				r.w=options_sfc->w;
				r.h=options_sfc->h;
				SDL_BlitSurface(options_sfc,0,screen,&r);
			}

			/* Draw Arrow: */ 
//			if ((state_timmer%12)<6) {
				r.x=screen->w/2-options_sfc->w/2-arrow_sfc->w-10;
				r.y=int(screen->h*y_position+y_start)+menu_item*y_inc;
				r.w=arrow_sfc->w;
				r.h=arrow_sfc->h;
				SDL_BlitSurface(arrow_sfc,0,screen,&r);
//			}

			SDL_FreeSurface(menu_sfc);
			SDL_FreeSurface(options_sfc);
		}
		break;
	} /* switch */ 

	/* Credits: */ 
	{
		int start_x,start_x2;
		SDL_Rect r1,r2;
		float f=float(menu_timmer)*(1.0F/float(EFFECT_LENGTH));
		if (f<0.0) f=0.0;
		if (f>=1.0) f=1.0;

		SDL_BlitSurface(credits_sfc,0,credits2_sfc,0);
		if (menu_state==0 || menu_state==4) surface_fader(credits2_sfc,f,f,f,0);

		menu_credits_timmer+=2;

		if (menu_credits_timmer>screen->w) {
			if (menu_credits_timmer-screen->w>credits_sfc->w) {
				menu_credits_timmer=0;
				start_x=0;
			} else {
				start_x=menu_credits_timmer-screen->w;
			} /* if */ 
		} else {
			start_x=0;
		} /* if */ 
		start_x2=screen->w-menu_credits_timmer;
		if (start_x2<0) start_x2=0;
		r1.x=start_x;
		r1.y=0;
		r1.w=credits_sfc->w-start_x;
		r1.h=credits_sfc->h;
		r2.x=start_x2;
//		r2.y=174;
		r2.y=screen->h-credits_sfc->h;
		SDL_BlitSurface(credits2_sfc,&r1,screen,&r2);
	}

} /* CRoadFighter::menu_draw */ 
