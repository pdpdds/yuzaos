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
#include "filehandling.h"

void CRoadFighter::load_configuration(void)
{
	int a,b,c;
	FILE *fp;

	fp=f1open("RoadFighter.cfg", "r", USERDATA);
	if (fp==0) {
		default_configuration();
		save_configuration();
		return;
	} /* if */ 
	if (3!=fscanf(fp,"%i %i %i",&a,&b,&c)) {
		fclose(fp);
		default_configuration();
		save_configuration();
		return;
	} /* if */ 
	left_key=a;
	right_key=b;
	fire_key=c;
	if (3!=fscanf(fp,"%i %i %i",&a,&b,&c)) {
		fclose(fp);
		default_configuration();
		save_configuration();
		return;
	} /* if */ 
	left2_key=a;
	right2_key=b;
	fire2_key=c;
	if (1!=fscanf(fp,"%i",&a)) {
		fclose(fp);
		default_configuration();
		save_configuration();
		return;
	} /* if */ 
	if (a==1) game_remake_extras=true;
		 else game_remake_extras=false;

} /* CRoadFighter::load_configuration */ 


void CRoadFighter::save_configuration(void)
{
	FILE *fp;

	fp=f1open("RoadFighter.cfg", "w", USERDATA);
	if (fp==0) return;
	fprintf(fp,"%i %i %i\n",left_key,right_key,fire_key);
	fprintf(fp,"%i %i %i\n",left2_key,right2_key,fire2_key);
	if (game_remake_extras) fprintf(fp,"1\n");
					   else fprintf(fp,"0\n");
	fclose(fp);
} /* CRoadFighter::save_configuration */ 


void CRoadFighter::default_configuration(void)
{
	left_key=SDLK_LEFT;
	right_key=SDLK_RIGHT;
	fire_key=SDLK_SPACE;

	left2_key=SDLK_a;
	right2_key=SDLK_d;
	fire2_key=SDLK_LSHIFT;

	game_remake_extras=true;
} /* CRoadFighter::default_configuration */ 

