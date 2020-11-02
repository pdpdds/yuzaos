#ifndef __GUI_MANAGER_H__
#define __GUI_MANAGER_H__

#include "guichan.hpp"
#include <guichan/sdl.hpp>

class GUIManager
{
public:
	GUIManager(SDL_Window *pWindow, SDL_Renderer *pRenderer);
	virtual ~GUIManager();

	bool InitSystem();
	bool Finally();

	bool PushInput(SDL_Event event);
	bool ProcessLogic();
	bool ProcessDraw();
	bool DoRender();

public:
	gcn::Gui* m_pGcn;
	int m_winWidth, m_winHeight;

private:
	SDL_Window* m_pWindow;
	SDL_Renderer* m_pRenderer;

	SDL_Surface* m_pGUIScreen;
	SDL_Texture* m_pGUITexture;

	
	gcn::SDLGraphics* m_pGcnGraphics;
	gcn::SDLInput* m_pGcnInput;
	gcn::SDLImageLoader* m_pGcnImageLoader;

};

#endif