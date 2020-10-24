
#include "SDL.h"
#include <stdlib.h>
#include "SDLSingleton.h"
#include "CGameCore.h"
#include <assert.h>

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char** argv)

{
	if (SDLSingleton::GetInstance()->InitSystem() == false)
		assert(0);
	
	CGameCore::GetInstance()->Initialize();

	int success = SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
		"Simple MessageBox",
		"12 ¬ã¬Ú¬Þ¬Ó¬à¬Ý¬à¬Ó.",
		NULL);

	bool running = true;

	Uint32 frameStart, frameTime;

	while (running)
	{
		frameStart = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					running = false;				
				}				
			}
			else if (event.type == SDL_KEYUP)
			{				
				CGameCore::GetInstance()->ProcessInput(event.key.keysym.scancode);
			}
			
			else if (event.type == SDL_QUIT)
			{
				running = false;
			}
			else if (event.type == SDL_FINGERUP)
			{							
				
				float fingerX = event.tfinger.x;
				float fingerY = event.tfinger.y;

				CGameCore::GetInstance()->ProcessInputWithTouch(fingerX, fingerY);
			}					
		}

		CGameCore::GetInstance()->ProcessGame();

		SDL_RenderClear(SDLSingleton::GetInstance()->GetRenderer());
		CGameCore::GetInstance()->Render();
		SDL_RenderPresent(SDLSingleton::GetInstance()->GetRenderer());

		frameTime = SDL_GetTicks() - frameStart;

		if (frameTime < DELAY_TIME)
		{
			SDL_Delay((int)(DELAY_TIME - frameTime));
		}
	}

	

	return 0;
}

