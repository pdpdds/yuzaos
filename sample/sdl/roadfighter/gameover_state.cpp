#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

#include "sound.h"

#include "CTile.h"
#include "CObject.h"
#include "CGame.h"
#include "CRoadFighter.h"
#include "auxiliar.h"

int gameover_time=25;

int CRoadFighter::gameover_cycle(void)
{
	if (state_timmer==0) {
		gameover_state=0;
		gameover_timmer=0;
		Sound_music_volume(MIX_MAX_VOLUME);
		Sound_create_music("sound/gameover",0);
	} /* if */ 

	if (gameover_state==1) {
		int volume=(gameover_timmer*MIX_MAX_VOLUME)/gameover_time;
		if (volume<0) volume=0;
		if (volume>MIX_MAX_VOLUME) volume=MIX_MAX_VOLUME;
		Sound_music_volume(volume);
	}

	if (gameover_state==0) gameover_timmer++;
	if (gameover_state==1) gameover_timmer--;

	if (gameover_state==0 &&
		(state_timmer>=gameover_time*6 ||
		 (state_timmer>=gameover_time &&
		 ((keyboard[fire_key] && !old_keyboard[fire_key]) ||
		  (keyboard[SDLK_ESCAPE] && !old_keyboard[SDLK_ESCAPE]))))) {
		gameover_state=1;
		if (gameover_timmer>=gameover_time) gameover_timmer=gameover_time;
	} /* if */ 

	if (gameover_state==1 && gameover_timmer<=0) {
		Sound_release_music();
		Sound_music_volume(MIX_MAX_VOLUME);
		return KONAMI_STATE;
	} /* if */ 
	return GAMEOVER_STATE;
} /* CRoadFighter::gameover_cycle */ 



void CRoadFighter::gameover_draw(SDL_Surface *screen)
{
	SDL_Rect r;

	SDL_FillRect(screen,0,0);

	/* Draw Scoreboard: */ 
	if (gameover_state==1) {
		if (scoreboard_fade_timmer>0) scoreboard_fade_timmer--;
		if (scoreboard_x<screen->w) scoreboard_x+=int((screen->w-desired_scoreboard_x)/gameover_time);
		if (scoreboard_x>screen->w) scoreboard_x=screen->w;
	} /* if */ 
	scoreboard_draw(scoreboard_x,0,screen);


	/* Draw text: */ 
	{
		SDL_Surface *text_sfc;
		float f=1.0;

		text_sfc=SDL_DisplayFormat(gameover_sfc);
		f=float(gameover_timmer)/gameover_time;	
		if (f>=1.0) f=1.0;
		if (f<1.0) surface_fader(text_sfc,f,f,f,0);
		r.x=(desired_scoreboard_x)/2-text_sfc->w/2;
		r.y=(screen->h/2)-text_sfc->h/2;
		r.w=text_sfc->w;
		r.h=text_sfc->h;
		SDL_BlitSurface(text_sfc,0,screen,&r);
		SDL_FreeSurface(text_sfc);
	}

} /* CRoadFighter::gameover_draw */ 
