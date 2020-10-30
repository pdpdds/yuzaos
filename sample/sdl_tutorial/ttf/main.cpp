#include <stdio.h>
#include <MainEntry.h>
#include <SDL.h>
#include <systemcall_impl.h>
#include <SDL_ttf.h>

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
	TTF_Font* gFont;
	
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
		
#if 1
	gFont = TTF_OpenFont("Inconsolata-Regular.ttf", 30);
#else	
	gFont = TTF_OpenFont("beskin.ttf", 30);
#endif
#ifdef _WIN32
	TTF_SetFontHinting(gFont, TTF_HINTING_MONO);
#endif

#if 1
	SDL_Color textColor = { 0, 0, 0 };
	SDL_Surface* textSurface = TTF_RenderText_Blended(gFont, "TTF Test", textColor);
	SDL_Texture* mTexture = SDL_CreateTextureFromSurface(pRenderer, textSurface);
#else
	FILE* fp = fopen("utf8_text.txt", "rb");
	char utf8_string[256] = { 0, };
	fread(utf8_string, 255, 1, fp);
	fclose(fp);

	SDL_Color textColor = { 0, 0, 0 };
	SDL_Surface* textSurface = TTF_RenderUTF8_Blended(gFont, utf8_string, textColor);
	//SDL_Surface* textSurface2 = TTF_RenderText_Blended(gFont, "TTF font bug is panic!!", textColor);
	SDL_Texture* mTexture = SDL_CreateTextureFromSurface(pRenderer, textSurface);
#endif

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

	SDL_DestroyRenderer(pRenderer);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();

	return 0;
}