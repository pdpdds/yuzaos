#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "proggytiny.h"
#include "stb_decompress.h"

#define TICK_INTERVAL    30

static Uint32 next_time;

Uint32 time_left(void)
{
	Uint32 now;

	now = SDL_GetTicks();
	if (next_time <= now)
		return 0;
	else
		return next_time - now;
}

int main(int argc, char** argv)
{
	SDL_Window *pWindow;
	SDL_Renderer *pRenderer;
	
	int width = 320;
	int height = 200;

	if (TTF_Init() == -1) 
	{
		return 0;
	}

#ifdef _WIN32
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
#endif
	if (SDL_CreateWindowAndRenderer(width, height, 0, &pWindow, &pRenderer) < 0)
	{
		printf("SDL_CreateWindowAndRenderer Error\n");
		return 0;
	}

	SDL_Texture* screenTexture  = SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
		
    const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
    SDL_RWops* font = GetFontDataFromMemoryCompressedBase85TTF(ttf_compressed_base85);
    
	TTF_Font* ttfFont = TTF_OpenFontRW(font, 1, 30);

	SDL_Color textColor = { 0, 0, 0 };
	SDL_Surface* textSurface = TTF_RenderText_Blended(ttfFont, "TTF Test", textColor);
	SDL_Texture* mTexture = SDL_CreateTextureFromSurface(pRenderer, textSurface);


	int mWidth = textSurface->w;
	int mHeight = textSurface->h;

	// render text
	SDL_Rect renderQuad = { 10, 10, mWidth, mHeight };
	SDL_Rect renderQuad2 = { 10, 100, mWidth, mHeight };
	

	bool running = true;

	next_time = SDL_GetTicks() + TICK_INTERVAL;
	int lastTickCount = SDL_GetTicks();
	int curTickCount = SDL_GetTicks();
	int k = 0;
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
		}

		SDL_RenderClear(pRenderer);
		SDL_SetRenderDrawColor(pRenderer, 255, 0, 255, 255);
		SDL_RenderCopy(pRenderer, mTexture, NULL, &renderQuad);
		SDL_RenderCopy(pRenderer, mTexture, NULL, &renderQuad2);
		
		SDL_RenderPresent(pRenderer);

		SDL_Delay(time_left());
		next_time += TICK_INTERVAL;
	}

    TTF_CloseFont(ttfFont);
    SDL_FreeRW(font);

	SDL_DestroyRenderer(pRenderer);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();

	return 0;
}