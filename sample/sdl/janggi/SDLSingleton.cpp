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

bool SDLSingleton::InitSystem()
{
	if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0){
		return false;
	}

	if (SDL_CreateWindowAndRenderer(640, 480, 0, &m_pWindow, &m_pRenderer) < 0)
	{
		return false;
	}

	SDL_GetWindowSize(m_pWindow, &m_winWidth, &m_winHeight);	

	m_pGameTexture = SDL_CreateTexture(m_pRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		m_winWidth, m_winHeight);

	if (m_pGameTexture == 0)
	{
		return false;
	}

	return true;
}