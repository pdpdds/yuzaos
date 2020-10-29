#ifdef _WIN32
//#include <windows.h>
//#include <windowsx.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

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

#include "debug.h"
#include <winapi.h>
#include <systemcall_impl.h>
/*						GLOBAL VARIABLES INITIALIZATION:							*/ 

const int REDRAWING_PERIOD=27;	/* This is for 35fps */ 
// const int REDRAWING_PERIOD=40;	/* This is for 25fps */ 

int SCREEN_X=512;
int SCREEN_Y=384;
const int COLOUR_DEPTH=32;
const char *application_name="Road Fighter v1.0";

#ifdef _WIN32
bool fullscreen=false;
#else
bool fullscreen=false;
#endif

int init_time=0;
SDL_Surface *screen_sfc;
CRoadFighter *game=0;
int screen_flags= /* SDL_GLSDL | SDL_DOUBLEBUF | */ 
				  /* SDL_HWSURFACE | SDL_DOUBLEBUF | */ 
				  0;

int start_level=1;


/*						AUXILIAR FUNCTION DEFINITION:							*/ 

SDL_Surface* initializeSDL(int moreflags)
{
	char VideoName[256];
	SDL_Surface *screen;

	//int flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWPALETTE;
	int flags = screen_flags|moreflags;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL_INIT_VIDEO | SDL_INIT_AUDIO\n");
		return 0;
	}
	output_debug_message("Initializing SDL video subsystem.\n");
	if ((SDL_Init(SDL_INIT_VIDEO)) == -1)
	{
	  output_debug_message("Couldn't initialize video subsystem: %s\n", SDL_GetError());
	  exit(-1);
	}
	SDL_VideoDriverName (VideoName, sizeof (VideoName));
    output_debug_message("SDL driver used: %s\n", VideoName);
    // Set the environment variable SDL_VIDEODRIVER to override
    // For Linux: x11 (default), dga, fbcon, directfb, svgalib,
    //            ggi, aalib
    // For Windows: directx (default), windib
	output_debug_message("SDL video subsystem initialized.\n");

	atexit(SDL_Quit);
	//SDL_WM_SetCaption(application_name, 0);
	//if (fullscreen) SDL_ShowCursor(SDL_DISABLE);
	Sound_initialization();
	
	//pause(1000);
	Sleep(1000);
	
	screen = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, COLOUR_DEPTH, flags);

	if (screen == NULL) {
		output_debug_message("Couldn't set %ix%ix%i", SCREEN_X, SCREEN_Y, COLOUR_DEPTH);
	    if (fullscreen) output_debug_message(",fullscreen,");
	    output_debug_message(" video mode: %s\n",SDL_GetError ());
	    exit(-1);
	} else {
	    output_debug_message("Set the video resolution to: %ix%ix%i",
				 SDL_GetVideoSurface()->w, SDL_GetVideoSurface()->h,
				 SDL_GetVideoSurface()->format->BitsPerPixel);
	    if (fullscreen) output_debug_message(",fullscreen");
	    output_debug_message("\n");
    } /* if */ 

	TTF_Init();

	SDL_EnableUNICODE(1);
	
	return screen;
} /* initializeSDL */ 


void finalizeSDL()
{
	TTF_Quit();
//	Sound_release();
	SDL_Quit();
} /* finalizeSDL */ 



#ifdef _WIN32
#undef main
int main(int argc, char** argv)
{
	{
		int tmp;
		if (argc == 2 &&
			1 == sscanf(argv[1], "%i", &tmp)) {
			start_level = tmp;
			if (start_level<1 || start_level>6) start_level = 1;
		} /* if */
	}

#else
int main(int argc, char** argv)
{
	setupTickCount();

	{
		int tmp;
		if (argc==2 &&
			1==sscanf(argv[1],"%i",&tmp)) {
			start_level=tmp;
			if (start_level<1 || start_level>6) start_level=1;
		} /* if */ 
	}
#endif

	int time,act_time;
	SDL_Event event;
    bool quit = false;

	
	time=init_time=GetTickCount();
	screen_sfc = initializeSDL((fullscreen ? SDL_FULLSCREEN : 0));
	if (screen_sfc == 0)
	{
		
		printf("SDL Init Fail\n");
		return 0;
	}
	
	game=new CRoadFighter();

	while (!quit) {
		while( SDL_PollEvent( &event ) ) {
            switch( event.type ) {
                /* Keyboard event */ 
                case SDL_KEYDOWN:
					// quit
					if (event.key.keysym.sym==SDLK_F12) {
						quit = true;
					}
					if (event.key.keysym.sym==SDLK_F4) {
						SDLMod modifiers;
						modifiers=SDL_GetModState();
						if ((modifiers&KMOD_ALT)!=0) {
							quit=true;
						}
					}
#ifdef __APPLE__
                    // different quit shortcut on OSX: apple+Q
                    if (event.key.keysym.sym == SDLK_q) {
                        SDLMod modifiers;
                        modifiers = SDL_GetModState();
                        if ((modifiers&KMOD_META) != 0) {
                            quit = true;
                        }
                    }
#endif
					// fullscreen
/*
FIXME: the code below is a big copy/paste; it should be in a separate function in stead
*/

#ifdef __APPLE__
					if (event.key.keysym.sym == SDLK_f) {
						SDLMod modifiers;

						modifiers=SDL_GetModState();

						if ((modifiers&KMOD_META) != 0) {
							Stop_playback();
							/* Toogle FULLSCREEN mode: */ 
							if (fullscreen) fullscreen=false;
									   else fullscreen=true;
							SDL_QuitSubSystem(SDL_INIT_VIDEO);
							SDL_InitSubSystem(SDL_INIT_VIDEO);
							if (SDL_WasInit(SDL_INIT_VIDEO)) {
								screen_sfc = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, COLOUR_DEPTH, 
															  (fullscreen ? SDL_FULLSCREEN : 0) | screen_flags);
								SDL_WM_SetCaption(application_name, 0);
								SDL_ShowCursor(SDL_DISABLE);
							} else {
								quit = true;
							} /* if */ 
							Resume_playback();
						} /* if */ 
					} /* if */ 
#endif
					
					if (event.key.keysym.sym==SDLK_RETURN) {
						SDLMod modifiers;

						modifiers=SDL_GetModState();

						if ((modifiers&KMOD_ALT)!=0) {
							Stop_playback();
							/* Toogle FULLSCREEN mode: */ 
							if (fullscreen) fullscreen=false;
									   else fullscreen=true;
							SDL_QuitSubSystem(SDL_INIT_VIDEO);
							SDL_InitSubSystem(SDL_INIT_VIDEO);
							if (SDL_WasInit(SDL_INIT_VIDEO)) {
								screen_sfc = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, COLOUR_DEPTH, 
															  (fullscreen ? SDL_FULLSCREEN : 0) | screen_flags);
								SDL_WM_SetCaption(application_name, 0);
								SDL_ShowCursor(SDL_DISABLE);
							} else {
								quit = true;
							} /* if */ 
							Resume_playback();
						} /* if */ 
					} /* if */ 
                    break;

                /* SDL_QUIT event (window close) */ 
                case SDL_QUIT:
                    quit = true;
                    break;
            } /* switch */ 
        } /* while */ 

		act_time=GetTickCount();
		if (act_time-time>=REDRAWING_PERIOD) {
			if ((act_time-init_time)>=1000) init_time=act_time;

			time+=REDRAWING_PERIOD;
			if ((act_time-time)>2*REDRAWING_PERIOD) time=act_time;
		
			if (!game->cycle()) quit=true;	
			SDL_SetClipRect(screen_sfc, 0);
			game->draw(screen_sfc);
			SDL_Flip(screen_sfc);
		} /* if */ 
		SDL_Delay(1);
	}

	delete game;

	finalizeSDL();

	return 0;
} /* main */ 


