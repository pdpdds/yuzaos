#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "sound.h"

#include "CTile.h"
#include "CObject.h"
#include "CGame.h"
#include "CRoadFighter.h"
#include "auxiliar.h"

#include "debug.h"
#include <systemcall_impl.h>

extern int MAX_SPEED;
extern int MAX_FUEL;
extern int SCREEN_X;
extern int SCREEN_Y;

CRoadFighter::CRoadFighter(void)
{
	int i;

	output_debug_message("CRoadFighter: in constructor...\n");

	state=PRESENTATION_STATE;
	state_timmer=0;
	current_level=0;
	high_score=0;
	game=0;

	presentation_state=0;
	presentation_timmer=0;
	konami_state=0;
	konami_timmer=0;
	interlevel_state=0;
	interlevel_timmer=0;
	gameover_state=0;
	gameover_timmer=0;
	endsequence_state=0;
	endsequence_timmer=0;
	scoreboard_fade_timmer=0;

	menu_tittle_text=0;
	menu_options_text=0;
	menu_redefining_key=-1;

	load_configuration();

	game_mode=0;

	scoreboard_x=-1;
	desired_scoreboard_x=SCREEN_X;

	for(i=0;i<SDLK_LAST;i++) old_keyboard[i]=0;

	font1=TTF_OpenFont("fonts/comicbd.ttf", 16);
	font2big=TTF_OpenFont("fonts/tanglewo.ttf", FONT_SIZE);
	font2medium=TTF_OpenFont("fonts/tanglewo.ttf", int(FONT_SIZE*0.8));
	font2small=TTF_OpenFont("fonts/tanglewo.ttf", int(FONT_SIZE*0.65));

	retroremakes_sfc = IMG_Load("graphics/retroremakes.bmp");
	disclaimer_sfc=IMG_Load("graphics/disclaimer.jpg");
	
	konami1_sfc=IMG_Load("graphics/konami1.jpg");
	konami2_sfc=IMG_Load("graphics/konami2.jpg");
	tittle_sfc=IMG_Load("graphics/title.jpg");
	arrow_sfc=IMG_Load("graphics/arrow.bmp");
	scoreboard_sfc=0;
	gamemap_sfc=IMG_Load("graphics/gamemap.bmp");
	minicar1_sfc=IMG_Load("graphics/minicar1.bmp");
	minicar2_sfc=IMG_Load("graphics/minicar2.bmp");
	gameover_sfc=IMG_Load("graphics/gameover.jpg");
	scoreboard_sfc=IMG_Load("graphics/scoreboard.bmp");
	scoreboard2_sfc=0;
	scoreboardleft_sfc=IMG_Load("graphics/scoreboard_left.bmp");

	{
		SDL_Color c;
		char *text="        "
				   "Game created for the RETROREMAKES REMAKE COMPETITION"
				   "        "
				   "PROGRAMMING: Santi Ontañón (Brain)    "
				   "GRAPHICS: Miikka Poikela (McBain)    "
				   "MUSIC/SFX: Jorrith Schaap (Jorito)    "
				   "BETA TESTING: Jason Eames (JEames), Miikka Poikela (McBain), Jorrith Schaap (Jorito), Santi Ontañón (Brain)"
				   "        ";
		c.r=c.g=c.b=128;

		credits_sfc=TTF_RenderText_Blended(font1,text,c);
		credits2_sfc=TTF_RenderText_Blended(font1,text,c);
	}

	levelintro_sfc=0;
	minicar1_tile=new CTile(0,0,minicar1_sfc->w/2,minicar1_sfc->h,minicar1_sfc,false);
	minicar2_tile=new CTile(0,0,minicar2_sfc->w/2,minicar2_sfc->h,minicar2_sfc,false);
	
	{
		SDL_Surface *sfc;

		sfc = SDL_DisplayFormat(konami1_sfc);
		SDL_FreeSurface(konami1_sfc);
		konami1_sfc=sfc;

		sfc = SDL_DisplayFormat(konami2_sfc);
		SDL_FreeSurface(konami2_sfc);
		konami2_sfc=sfc;

		sfc = SDL_DisplayFormat(tittle_sfc);
		SDL_FreeSurface(tittle_sfc);
		tittle_sfc=sfc;

		/* ... */ 
	}
	
	S_menu_move=Sound_create_sound("sound/menu_move");

	S_menu_select=Sound_create_sound("sound/menu_select");

	S_menu_in=Sound_create_sound("sound/logo_in");
	S_menu_out=Sound_create_sound("sound/logo_out");

	replay_fp=0;
	
	output_debug_message("CRoadFighter: constructor done.\n");

} /* CRoadFighter::CRoadFighter */ 


CRoadFighter::~CRoadFighter(void)
{
	output_debug_message("CRoadFighter: in destructor...\n");

	SDL_FreeSurface(retroremakes_sfc);
	SDL_FreeSurface(disclaimer_sfc);
	SDL_FreeSurface(konami1_sfc);
	SDL_FreeSurface(konami2_sfc);
	SDL_FreeSurface(tittle_sfc);
	SDL_FreeSurface(arrow_sfc);
	SDL_FreeSurface(scoreboard_sfc);
	SDL_FreeSurface(scoreboard2_sfc);
	SDL_FreeSurface(scoreboardleft_sfc);
	SDL_FreeSurface(gamemap_sfc);
	SDL_FreeSurface(minicar1_sfc);
	SDL_FreeSurface(minicar2_sfc);
	SDL_FreeSurface(gameover_sfc);
	SDL_FreeSurface(credits_sfc);
	SDL_FreeSurface(credits2_sfc);
	if (levelintro_sfc!=0) SDL_FreeSurface(levelintro_sfc);

	delete minicar1_tile;
	delete minicar2_tile;

	TTF_CloseFont(font1);
	TTF_CloseFont(font2big);
	TTF_CloseFont(font2medium);
	TTF_CloseFont(font2small);

	delete game;
	game=0;

	Sound_delete_sound(S_menu_move);
	Sound_delete_sound(S_menu_select);
	Sound_delete_sound(S_menu_in);
	Sound_delete_sound(S_menu_out);

	if (replay_fp!=0) fclose(replay_fp);
	replay_fp=0;

	output_debug_message("CRoadFighter: destructor done.\n");

} /* CRoadFighter::~CRoadFighter */ 



bool CRoadFighter::cycle(void)
{
	int i,old_state=state;
	bool retval=true;
	keyboard = (unsigned char *)SDL_GetKeyState(NULL);;

	switch(state) {
	case PRESENTATION_STATE:state=presentation_cycle();
					break;
	case KONAMI_STATE:state=konami_cycle();
					  break;
	case MENU_STATE:state=menu_cycle();
					break;
	case DEMO_STATE:state=KONAMI_STATE;
					break;
	case PLAYING_STATE:state=playing_cycle();
					break;
	case INTERLEVEL_STATE:state=interlevel_cycle();
					break;
	case GAMEOVER_STATE:state=gameover_cycle();
					break;
	case QUIT_STATE:retval=false;
					break;
	} /* switch */ 

	if (state!=old_state) state_timmer=0;
				 	 else state_timmer++;

	for(i=0;i<SDLK_LAST;i++) old_keyboard[i]=keyboard[i];
	
	return retval;
} /* CRoadFighter::cycle */ 


void CRoadFighter::draw(SDL_Surface *screen)
{
	if (state_timmer==0) return;

	switch(state) {
	case PRESENTATION_STATE:presentation_draw(screen);
					break;
	case KONAMI_STATE:konami_draw(screen);
					  break;
	case MENU_STATE:menu_draw(screen);
					break;
	case DEMO_STATE:
					break;
	case PLAYING_STATE:playing_draw(screen);
					break;
	case INTERLEVEL_STATE:interlevel_draw(screen);
					break;
	case GAMEOVER_STATE:gameover_draw(screen);
					break;
	} /* switch */ 
} /* CRoadFighter::draw */ 



void CRoadFighter::scoreboard_draw(int x,int y,SDL_Surface *screen)
{
	SDL_Rect r;

	if (scoreboard_sfc==0) return;

	/* Draw Scoreboard: */ 
	r.x=x;
	r.y=y;
	r.w=scoreboard_sfc->w;
	r.h=scoreboard_sfc->h;
	SDL_BlitSurface(scoreboard_sfc,0,screen,&r);
	if (r.x+r.w<screen->w) {
		SDL_Rect r2;
		int i;

		r2.x=scoreboard_sfc->w-1;
		r2.y=0;
		r2.w=1;
		r2.h=scoreboard_sfc->h;
		for(i=r.x+r.w;i<screen->w;i++) {
			r.x=i;
			r.w=1;
			SDL_BlitSurface(scoreboard_sfc,&r2,screen,&r);
		} /* for */ 
	} /* if */ 

	/* Draw the Speed: */ 
	if (game!=0) {
		List<int> l;
		int *speed;
		int i,j,tmp,tmp2;

		game->get_speeds(&l);
		l.Rewind();
		j=l.Length()-1;
		while(l.Iterate(speed)) {
			tmp=int(112*(float(-*(speed))/MAX_SPEED));
			tmp2=(33-l.Length())/l.Length();

			for(i=367;i>367-tmp;i-=2) {
				r.x=(x+28)+(tmp2+1)*j;
				r.y=i;
				r.w=tmp2;
				r.h=1;
				SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));
			} /* for */ 
			j--;
		} /* while */ 
	}

	/* Draw the fuel: */ 
	if (game!=0) {
		List<int> l;
		int *fuel;
		int i,j,tmp,tmp2;

		game->get_fuels(&l);
		l.Rewind();
		j=l.Length()-1;
		while(l.Iterate(fuel)) {
			tmp=int(112*(float(*fuel)/MAX_FUEL));
			tmp2=(33-l.Length())/l.Length();

			for(i=367;i>367-tmp;i-=2) {
				r.x=(x+76)+(tmp2+1)*j;
				r.y=i;
				r.w=tmp2;
				r.h=1;
				SDL_FillRect(screen,&r,SDL_MapRGB(screen->format,255,255,255));
			} /* for */ 
			j--;
		} /* while */ 
	} /* if */ 

	/* Draw the mini-map: */ 
	{
		SDL_Rect r;
		float f;

		r.x=scoreboard_x+41;
		r.y=97;
		r.w=55;
		r.h=143;
		SDL_BlitSurface(scoreboard2_sfc,0,screen,&r);
		f=scoreboard_fade_timmer/25.0F;
		if (f<0) f=0;
		if (f>1.0) f=1.0;

		if (f!=1.0) surface_fader(screen,f,f,f,&r);
	}

	/* Draw the mini-car: */ 
	if (game!=0 && (state_timmer>>3)&0x01!=0) {
		List<float> l;
		float *pos;
		int car_x,car_y,j;
		
		game->get_positions(&l);
		l.Rewind();
 
		car_x=x+68+l.Length()*minicar1_tile->get_dx()/2-minicar1_tile->get_dx();
		j=0;
		while(l.Iterate(pos)) {
			car_y=int(108+*pos*(121-minicar1_tile->get_dy()));
			if (j==0) minicar1_tile->draw(car_x,car_y,screen);
			if (j==1) minicar2_tile->draw(car_x,car_y,screen);
			car_x-=minicar1_tile->get_dx();
			j++;
		} /* while */ 
	} /* if */ 

	/* Draw the score: */ 
	if (game!=0) {
		/* High-score: */ 
		SDL_Color c;
		SDL_Surface *sfc;
		char tmp[80];
		List<int> l;
		int *itmp,j,score_x;

		sprintf(tmp,"%i",high_score);
		c.r=c.b=0;
		c.g=255;
		sfc=TTF_RenderText_Blended(font1,tmp,c);
		r.x=x+103-sfc->w;
		r.y=21;
		r.w=sfc->w;
		r.h=sfc->h;
		SDL_BlitSurface(sfc,0,screen,&r);
		SDL_FreeSurface(sfc);

		/* Scores: */ 
		game->get_scores(&l);
		l.Rewind();
		j=0;
		score_x=103;
		while(l.Iterate(itmp)) {
			if (((state_timmer>>5)%l.Length())==j) {
				sprintf(tmp,"%i",*itmp);
				c.r=c.b=c.g=0;
				if (j==0) c.r=255;
				if (j==1) {
					c.r=255;
					c.g=255;
				} /* if */ 
				sfc=TTF_RenderText_Blended(font1,tmp,c);
				r.x=x+score_x-sfc->w;
				r.y=64;
				r.w=sfc->w;
				r.h=sfc->h;
				SDL_BlitSurface(sfc,0,screen,&r);
				SDL_FreeSurface(sfc);
			} /* if */ 
			j++;
		} /* while */ 
	} /* if */ 

	/* Draw the left scoreboard: */ 
	if (n_players==1) {
		float f;
		SDL_Rect r;

		f=float(screen->w-scoreboard_x)/float(screen->w-desired_scoreboard_x);
		r.x=(-scoreboardleft_sfc->w)+int((desired_scoreboard_x-352)*f);
		r.y=0;
		r.w=scoreboardleft_sfc->w;
		r.h=scoreboardleft_sfc->h;

		SDL_BlitSurface(scoreboardleft_sfc,0,screen,&r);
	} /* if */ 

} /* CRoadFighter::scoreboard_draw */ 
