#ifndef CGAMECORE_H
#define CGAMECORE_H
#include "search.h"
#include <SDL_image.h>
#include <map>

struct SDL_Texture;
struct SDL_Renderer;


#define GAME_WIDTH  640
#define GAME_HEIGHT 480

#define SDL_RATIO_X(width) width * ((float)CGameCore::GetInstance()->getGameWidth() / (float)GAME_WIDTH)
#define SDL_RATIO_Y(height) height * ((float)CGameCore::GetInstance()->getGameHeight() / (float)GAME_HEIGHT)

#define SDL_REVERSE_RATIO_X(width) width * ((float)GAME_WIDTH / (float)CGameCore::GetInstance()->getGameWidth())
#define SDL_REVERSE_RATIO_Y(height) height * ((float)GAME_HEIGHT / (float)CGameCore::GetInstance()->getGameHeight())


class CGameCore
{	
public:	
	CGameCore(void);
	virtual ~CGameCore(void);

	bool Initialize();
	SDL_Texture* LoadImage(char* szFileName, SDL_Renderer* pRenderer, bool colorKey = false);

	static CGameCore* GetInstance()
	{
		if (0 == m_pGameCore)
		{
			m_pGameCore = new CGameCore();
		}
		return m_pGameCore;
	}

	bool InitStage();
	void UpdateGameStatus();
	void ProcessInput (int scanCode);
	void ProcessInputWithTouch(float x, float y);
	bool ProcessGame();
	void Render();
	bool ProcessDirective(int iX, int iY);

	bool GetPause()
	{
		return m_bPaused;
	}

	void DrawBoader();
	void DrawPiece();
	void DrawPlayer();

	int getGameWidth() const { return m_gameWidth; }
	int getGameHeight() const { return m_gameHeight; }


protected:

private:
	static CGameCore* m_pGameCore;
	static bool m_bPaused;
	static bool m_bLevelReset;


	int m_gameWidth;
	int m_gameHeight;

	std::map<int, SDL_Texture*> m_mapTexture;
	SDL_Texture* m_boarder;
	Search search;
};

#endif