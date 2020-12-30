#pragma once
#include <SDL.h>
class SDLSingleton
{
public:
	SDLSingleton();
	virtual ~SDLSingleton();

	SDL_Window* GetWindow(){ return m_pWindow; }	
	SDL_Renderer* GetRenderer(){ return m_pRenderer; }

	static SDLSingleton* GetInstance()
	{
		if (m_pInstance == 0)
		{
			m_pInstance = new SDLSingleton();
		}
		return m_pInstance;
	}

	bool InitSystem();

	int m_winWidth, m_winHeight;


private:
	SDL_Window* m_pWindow;
	static SDLSingleton* m_pInstance;

	SDL_Renderer* m_pRenderer;
	
	SDL_Texture* m_pGameTexture;
};

