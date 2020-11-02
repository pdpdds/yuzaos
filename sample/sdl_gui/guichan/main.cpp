#include <stdio.h>
#include <SDL.h>
#include <systemcall_impl.h>

#define TICK_INTERVAL    30
#include <iostream>

#include "GUIManager.h"
#include "WidgetsContainer.h"

int main(int argc, char** argv)
{
	SDL_Window* pWindow;
	SDL_Renderer* pRenderer;

	int width = 640;
	int height = 480;

	if (SDL_CreateWindowAndRenderer(width, height, 0, &pWindow, &pRenderer) < 0)
	{
		printf("SDL_CreateWindowAndRenderer Error\n");
		return 0;
	}

	GUIManager* pGUIManager = new GUIManager(pWindow, pRenderer);

	if (false == pGUIManager->InitSystem())
	{
		std::cout << "GUIManager::InitSystem Error: " << std::endl;
		return 0;
	}

	WidgetsContainer* pWidgetsContainer = new WidgetsContainer(pGUIManager);

	if (false == pWidgetsContainer->BuildWidgets())
	{
		std::cout << "GUIManager::BuildWidget Error: " << std::endl;
		return 0;
	}

	bool running = true;

	while (running)
	{
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
			else if (event.type == SDL_QUIT)
			{
				running = false;
			}

			pGUIManager->PushInput(event);
		}

		pGUIManager->ProcessLogic();
		pGUIManager->ProcessDraw();

		SDL_RenderClear(pRenderer);
		pGUIManager->DoRender();
		SDL_RenderPresent(pRenderer);
	}

	pGUIManager->Finally();

	SDL_DestroyRenderer(pRenderer);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();

	return 0;
}