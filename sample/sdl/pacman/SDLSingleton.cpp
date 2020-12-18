#include "SDLSingleton.h"


SDLSingleton* SDLSingleton::m_pInstance = 0;

SDLSingleton::SDLSingleton()
{
	m_pInstance = 0;
}


SDLSingleton::~SDLSingleton()
{
	SDL_DestroyTexture(m_pGameTexture);
	SDL_DestroyRenderer(m_pRenderer);
	SDL_DestroyWindow(m_pWindow);
	SDL_Quit();
}

bool SDLSingleton::InitSystem(int width, int height)
{
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0){
		return 0;
	}


	if (SDL_CreateWindowAndRenderer(width, height, 0, &m_pWindow, &m_pRenderer) < 0)
	{
		return false;
	}

	SDL_GetWindowSize(m_pWindow, &m_winWidth, &m_winHeight);

	// if all this hex scares you, check out SDL_PixelFormatEnumToMasks()!
	m_pGameScreen = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
		0,
		0,
		0,
		0);

	if (m_pGameScreen == 0)
	{
		return false;
	}

	m_pGameTexture = SDL_CreateTexture(m_pRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width, height);

	if (m_pGameTexture == 0)
	{
		return false;
	}

	return true;
}

void SDLSingleton::DoRender()
{
	SDL_RenderClear(m_pRenderer);
	SDL_UpdateTexture(m_pGameTexture, NULL, m_pGameScreen->pixels, m_pGameScreen->pitch);
	SDL_Rect srcRect;
	SDL_Rect destRect;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = destRect.w = WINDOW_WIDTH;
	srcRect.h = destRect.h = WINDOW_HEIGHT;
	destRect.x = 0;
	destRect.y = 0;

	destRect.x = 0;
	destRect.y = 0;

	destRect.w = SDL_RATIO_X(destRect.w);
	destRect.h = SDL_RATIO_Y(destRect.h);

	//SDL_SetTextureAlphaMod(m_pGameTexture, 255);
	SDL_RenderCopyEx(SDLSingleton::GetInstance()->GetRenderer(), m_pGameTexture, &srcRect, &destRect, 0, 0, SDL_FLIP_NONE);

	SDL_RenderPresent(m_pRenderer);
}