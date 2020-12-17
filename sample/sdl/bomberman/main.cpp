// ConanGame.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "SDL.h"

#include "Game.h"
#include <iostream>

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char* argv[])
{
	Uint32 frameStart, frameTime;

	std::cout << "game init attempt...\n";

#ifndef WIN32 
	if (TheGame::Instance()->init("BomberMan", 0, 0, 0, 0, false))
#else
	if (TheGame::Instance()->init("BomberMan", 100, 100, DESIGNED_SCREEN_SIZE_X, DESIGNED_SCREEN_SIZE_Y, false))
#endif
	{
		std::cout << "game init success!\n";
		while (TheGame::Instance()->running())
		{
			frameStart = SDL_GetTicks();

			TheGame::Instance()->handleEvents();
			TheGame::Instance()->Update();
			TheGame::Instance()->Render();

			frameTime = SDL_GetTicks() - frameStart;

			if (frameTime < DELAY_TIME)
			{
				SDL_Delay((int)(DELAY_TIME - frameTime));
			}
		}
	}
	else
	{
		std::cout << "game init failure - " << SDL_GetError() << "\n";
		return -1;
	}

	std::cout << "game closing...\n";
	TheGame::Instance()->Clean();

	return 0;
}