#include "GUIManager.h"
#include "IWidgetContainer.h"

GUIManager::GUIManager(SDL_Window *pWindow, SDL_Renderer *pRenderer)
: m_pWindow(pWindow)
, m_pRenderer(pRenderer)
{

}


GUIManager::~GUIManager()
{
}

bool GUIManager::InitSystem()
{
	SDL_GetWindowSize(m_pWindow, &m_winWidth, &m_winHeight);

	// if all this hex scares you, check out SDL_PixelFormatEnumToMasks()!
	m_pGUIScreen = SDL_CreateRGBSurface(0, m_winWidth, m_winHeight, 32,
		0,
		0,
		0,
		0);

	if (m_pGUIScreen == 0)
	{
		std::cout << "SDL_CreateRGBSurface Error: " << SDL_GetError() << std::endl;
		return false;
	}

	m_pGUITexture = SDL_CreateTexture(m_pRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		m_winWidth, m_winHeight);

	if (m_pGUITexture == 0)
	{
		std::cout << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
		return false;
	}

	m_pGcnImageLoader = new gcn::SDLImageLoader(m_pRenderer);
	gcn::Image::setImageLoader(m_pGcnImageLoader);
	m_pGcnGraphics = new gcn::SDLGraphics();
	m_pGcnGraphics->setTarget(m_pGUIScreen);
	m_pGcnInput = new gcn::SDLInput();

	m_pGcn = new gcn::Gui();
	m_pGcn->setGraphics(m_pGcnGraphics);
	m_pGcn->setInput(m_pGcnInput);

	return true;
}

bool GUIManager::Finally()
{
	SDL_DestroyTexture(m_pGUITexture);
	SDL_FreeSurface(m_pGUIScreen);

	delete m_pGcn;

	delete m_pGcnImageLoader;
	delete m_pGcnInput;
	delete m_pGcnGraphics;

	return true;
}

bool GUIManager::PushInput(SDL_Event event)
{
	m_pGcnInput->pushInput(event);
	return true;
}

bool GUIManager::ProcessLogic()
{
	m_pGcn->logic();
	return true;
}

bool GUIManager::ProcessDraw()
{
	m_pGcn->draw();
	return true;
}

bool GUIManager::DoRender()
{
	SDL_UpdateTexture(m_pGUITexture, NULL, m_pGUIScreen->pixels, m_pGUIScreen->pitch);
	SDL_RenderCopy(m_pRenderer, m_pGUITexture, NULL, NULL);

	return true;
}