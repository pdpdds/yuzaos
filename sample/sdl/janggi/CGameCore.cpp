#include "CGameCore.h"
#include "SDLSingleton.h"
#include <stdio.h>
#include <sprintf.h>
//#include <iostream>

CGameCore* CGameCore::m_pGameCore = 0;
bool CGameCore::m_bPaused = false;
bool CGameCore::m_bLevelReset = false;

char raw[255] = { '\0' };
SIDE currentTurn = SD_CHO;
SIDE computerSide = SD_HAN;
int playerSide = SD_CHO;



void initialSet(Search* search) {
	search->defaultSetting();

	/*search->addPiece(3, 0, 'a');
	search->addPiece(4, 0, 'j');
	search->addPiece(5, 0, 'a');
	search->addPiece(8, 0, 'c');
	search->addPiece(1, 1, 'C');
	search->addPiece(2, 2, 'm');
	search->addPiece(4, 2, 'p');
	search->addPiece(7, 3, 'b');
	search->addPiece(8, 3, 'b');

	search->addPiece(0, 4, 'c');
	search->addPiece(2, 4, 'b');
	search->addPiece(5, 4, 'M');
	search->addPiece(7, 5, 'M');
	search->addPiece(1, 6, 'B');
	search->addPiece(5, 6, 'm');
	search->addPiece(3, 7, 'P');
	search->addPiece(4, 7, 'P');

	search->addPiece(2, 8, 'p');
	search->addPiece(4, 8, 'J');
	search->addPiece(3, 9, 'A');
	search->addPiece(5, 9, 'A');
	search->addPiece(7, 9, 'C');*/
}

CGameCore::CGameCore(void)
{
	//search.initParams(3, 17, 6500, 15000);
	search.initParams(3, 17, 650, 1500);
	initialSet(&search);

	playerSide = SD_HAN;
	computerSide = SD_CHO;
	
}

CGameCore::~CGameCore(void)
{
}

bool CGameCore::Initialize()
{
	SDL_GetWindowSize(SDLSingleton::GetInstance()->GetWindow(), &m_gameWidth, &m_gameHeight);	
	m_mapTexture[PS_CHO_JANG] = LoadImage("Resource/PS_CHO_JANG.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_CHO_SA] = LoadImage("Resource/PS_CHO_SA.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_CHO_PO] = LoadImage("Resource/PS_CHO_PO.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_CHO_JOL] = LoadImage("Resource/PS_CHO_JOL.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_CHO_MA] = LoadImage("Resource/PS_CHO_MA.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_CHO_SANG] = LoadImage("Resource/PS_CHO_SANG.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_CHO_CHA] = LoadImage("Resource/PS_CHO_CHA.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_HAN_JANG] = LoadImage("Resource/PS_HAN_JANG.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_HAN_SA] = LoadImage("Resource/PS_HAN_SA.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_HAN_PO] = LoadImage("Resource/PS_HAN_PO.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_HAN_JOL] = LoadImage("Resource/PS_HAN_JOL.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_HAN_MA] = LoadImage("Resource/PS_HAN_MA.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_HAN_SANG] = LoadImage("Resource/PS_HAN_SANG.png", SDLSingleton::GetInstance()->GetRenderer());
	m_mapTexture[PS_HAN_CHA] = LoadImage("Resource/PS_HAN_CHA.png", SDLSingleton::GetInstance()->GetRenderer());

	m_boarder = LoadImage("Resource/boarder.png", SDLSingleton::GetInstance()->GetRenderer());
	return true;
}

SDL_Point getsize(SDL_Texture* texture) {
	SDL_Point size;
	SDL_QueryTexture(texture, NULL, NULL, &size.x, &size.y);
	return size;
}

SDL_Texture* CGameCore::LoadImage(char* szFileName, SDL_Renderer* pRenderer, bool colorKey)
{
	SDL_Surface* pTempSurface = IMG_Load(szFileName);

	if (pTempSurface == 0)
	{
		printf("%s\n", IMG_GetError());
		return 0;
	}

	if (colorKey == true)
		SDL_SetColorKey(pTempSurface, SDL_TRUE, 0);

	SDL_Texture* pTexture = SDL_CreateTextureFromSurface(pRenderer, pTempSurface);

	SDL_FreeSurface(pTempSurface);

	return pTexture;
}

bool CGameCore::InitStage()
{
	return true;
}


void CGameCore::UpdateGameStatus(){
	
}

void CGameCore::ProcessInputWithTouch(float x, float y)
{

	
}

void CGameCore::ProcessInput (int scanCode)
{
	
}

void makeMove(Search* search, MV mv) {
	int sqSrc = SRC(mv);
	int sqDst = DST(mv);

	int from_x = GET_X(sqSrc);
	int from_y = GET_Y(sqSrc);
	int to_x = GET_X(sqDst);
	int to_y = GET_Y(sqDst);

	int ret = search->makeMove(from_x, from_y, to_x, to_y);
	if (ret < 0) {
		assert(0);
	}
}

bool CGameCore::ProcessGame()
{
	if (m_bLevelReset == false)
		InitStage();

	//ProcessInput();

	int commv = search.getBestMove(currentTurn, 0);
	
	makeMove(&search, commv);
	currentTurn = (SD_HAN + 1) - currentTurn;
	
	Render();
	UpdateGameStatus();

	return true;
}

inline void DrawTexture(int x, int y, int width, int height, SDL_Texture *src)
{
	SDL_Rect srcRect;
	SDL_Rect destRect;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = destRect.w = width;
	srcRect.h = destRect.h = height;
	destRect.x = x;
	destRect.y = y;

	destRect.x = SDL_RATIO_X(x);
	destRect.y = SDL_RATIO_Y(y);

	destRect.x += SDL_RATIO_X(destRect.w) * 0.15;
	destRect.y += SDL_RATIO_Y(destRect.h) * 0.15;
	
	destRect.w = SDL_RATIO_X(destRect.w) * 0.7;
	destRect.h = SDL_RATIO_Y(destRect.h) * 0.7;

	SDL_SetTextureAlphaMod(src, 255);
	SDL_RenderCopyEx(SDLSingleton::GetInstance()->GetRenderer(), src, &srcRect, &destRect, 0, 0, SDL_FLIP_NONE);

}

void CGameCore::DrawPiece()
{
	for (int r = 0; r < DF_PANEL_HEIGHT; ++r) 
	{
		for (int c = 0; c < DF_PANEL_WIDTH; c++)
		{
			PIECE piece = search.GetPiece(c, r);
			
			auto iter = m_mapTexture.find(piece);

			if (iter != m_mapTexture.end())
			{
				SDL_Texture* pTexture = iter->second;
				SDL_Point size = getsize(pTexture);
				int anchor_x = 52;
				int anchor_y = 30;
				DrawTexture(anchor_x + c * 67 - size.x / 2, anchor_y + r * 46 - size.y / 2, size.x, size.y, pTexture);
				
			}
		}
	}

	

}

void CGameCore::DrawPlayer()
{
	//DrawTexture(hero_x*tileSize, hero_y*tileSize, tileSize, tileSize, hero);
}

void CGameCore::DrawBoader()
{
	SDL_RenderCopy(SDLSingleton::GetInstance()->GetRenderer(), m_boarder, 0, 0);
}

void CGameCore::Render()
{		
	DrawBoader();
	DrawPiece();
	DrawPlayer();
}

bool CGameCore::ProcessDirective(int iX, int iY)
{
	return true;
}