#ifdef __cplusplus
extern "C" {
#endif
#include "SDL.h"
#ifdef __cplusplus
}
#endif

#include "SDLGui.h"
#include <sprintf.h>
#include <string.h>
#include  <stdio.h>

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0){
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return 0;
	}

	SDL_Window *pWindow;
	SDL_Renderer *pRenderer;
	if (SDL_CreateWindowAndRenderer(640, 480, 0, &pWindow, &pRenderer) < 0)

	{
		printf("SDL_CreateWindowAndRenderer Error: %s\n",SDL_GetError());
		return 0;
	}

	// if all this hex scares you, check out SDL_PixelFormatEnumToMasks()!
	screen = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 480, 32,
		0,
		0,
		0,
		0);

	if (screen == 0)
	{
		printf("SDL_CreateRGBSurface Error: %s\n", SDL_GetError());
		return 0;
	}

	SDL_Texture* pGUITexture = SDL_CreateTexture(pRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		640, 480);

	if (pGUITexture == 0)
	{
		printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
		return 0;
	}

	if(TTF_Init() == -1)
	{
		printf("TTF Error: %s\n", SDL_GetError());
		return 0;
	}

	GUILoadFont("font.ttf");

	char text[256] = { 0, };
	GUIListBox serverListBox;
	GUIButton getServers; 
	Uint32 lastRetrieve = 0;
	GUIButton connectToServer; 
	connectToServer.SetDisabled(true);
	GUILabel titleLabel; 
	sprintf(text, "G v%s Protocol version %s", "1.0", "2.0");
	titleLabel.Render(text, 0x0000FFFF, 20);
	GUITextBox customServer; customServer.SetText("localhost:666");
	GUIButton customServerConnect;

	bool running = true;
	Uint32 start = SDL_GetTicks();

	int screenW = 640;
	int screenH = 480;
	titleLabel.Register();
	titleLabel.SetPos(5, 5, screenW - 5, 25);
	serverListBox.Register();
	serverListBox.SetPos(5, 30, screenW - 5, screenH - 55);
	getServers.Register();
	getServers.SetPos(5, screenH - 50, screenW / 2 - 5, screenH - 30);
	getServers.SetText("Retrieve server list");
	connectToServer.Register();
	connectToServer.SetPos(screenW / 2 + 5, screenH - 50, screenW - 5, screenH - 30);
	connectToServer.SetText("Connect to selected server");
	customServer.Register();
	customServer.SetPos(5, screenH - 25, screenW / 2 - 5, screenH - 5);
	customServerConnect.Register();
	customServerConnect.SetPos(screenW / 2 + 5, screenH - 25, screenW - 5, screenH - 5);
	customServerConnect.SetText("Direct connect");

	for (int i = 0; i < 30; i++)
	{
		serverListBox.AddItem("Chat Test");
	}
	

	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (GUIEvent(&event))continue;
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

		}

		Uint32 currentTime = SDL_GetTicks();
		Uint32 diff = currentTime - start;

		
		boxColor(screen,0,0,screen->w,screen->h,0xF0F0F0FF);
		GUIDraw(screen);
		SDL_RenderClear(pRenderer);
		SDL_UpdateTexture(pGUITexture, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(pRenderer, pGUITexture, NULL, NULL);
		SDL_RenderPresent(pRenderer);
	}

	
	SDL_DestroyRenderer(pRenderer);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();

	return 0;
}

void Popup(const char *text, Uint32 color, Uint32 time){
	SDLTimer timer; SDL_Event SDLEvent;
	bool run = true; Uint16 textLen = strlen(text);
	SDLSetVideo(800, 200, false);
	SDL_ShowCursor(SDL_ENABLE);
	timer.Reset();
	while (run){
		while (SDL_PollEvent(&SDLEvent)){
			switch (SDLEvent.type){
			case SDL_QUIT: run = false; break;
			case SDL_KEYDOWN:
				if (SDLEvent.key.keysym.sym == SDLK_ESCAPE)run = false;
				break;
			}
		}
		timer.Frame(15);
		if (time > 0)if (timer.total > time)run = false;
		boxColor(screen, 0, 0, screen->w, screen->h, 0xF0F0F0FF);
		stringColor(screen, (screen->w / 2) - (textLen * 4), screen->h / 2 - 4, text, color);
		//SDL_Flip(screen);
	}
}