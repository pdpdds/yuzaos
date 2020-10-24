#ifndef CGAMECORE_H
#define CGAMECORE_H

struct SDL_Texture;
struct SDL_Renderer;

#define SOCOBAN_MAX_WIDTH 20
#define SOCOBAN_MAX_HEIGHT 20

#define GAME_WIDTH  400
#define GAME_HEIGHT 400

#define SDL_RATIO_X(width) width * ((float)CGameCore::GetInstance()->getGameWidth() / (float)GAME_WIDTH)
#define SDL_RATIO_Y(height) height * ((float)CGameCore::GetInstance()->getGameHeight() / (float)GAME_HEIGHT)

#define SDL_REVERSE_RATIO_X(width) width * ((float)GAME_WIDTH / (float)CGameCore::GetInstance()->getGameWidth())
#define SDL_REVERSE_RATIO_Y(height) height * ((float)GAME_HEIGHT / (float)CGameCore::GetInstance()->getGameHeight())

enum  enumSpaceProperty
{		
	ENUM_SPACE_EMPTY,
	ENUM_SPACE_BLOCK,	
	ENUM_SPACE_BOX_POINT,
	ENUM_SPACE_BOX,
	ENUM_SPACE_ENEMY,
};

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
	void DrawGround();
	void DrawWall();
	void DrawBox();
	void DrawPlayer();

	int getGameWidth() const { return m_gameWidth; }
	int getGameHeight() const { return m_gameHeight; }


protected:

private:
	static CGameCore* m_pGameCore;
	static bool m_bPaused;

	static bool m_bLevelReset;
	static int iStageNum;

	int array_value[SOCOBAN_MAX_WIDTH][SOCOBAN_MAX_HEIGHT];
	int array_value1[SOCOBAN_MAX_WIDTH][SOCOBAN_MAX_HEIGHT];
	int array_value2[SOCOBAN_MAX_WIDTH][SOCOBAN_MAX_HEIGHT];
	int hero_x;
	int hero_y;
	
	int iTotalBoxNum;
	int iCurrentExactBoxCount;

	int m_gameWidth;
	int m_gameHeight;	
};

#endif