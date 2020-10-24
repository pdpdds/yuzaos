#include "CGameCore.h"
#include "SDLSingleton.h"
#include <SDL_image.h>
#include <stdio.h>
#include <sprintf.h>
//#include <iostream>

CGameCore* CGameCore::m_pGameCore = 0;
bool CGameCore::m_bPaused = false;
bool CGameCore::m_bLevelReset = false;
int CGameCore::iStageNum = 1;

SDL_Texture* bg = 0;
SDL_Texture* background = 0;
SDL_Texture* box = 0;
SDL_Texture* hero = 0;
SDL_Texture* bloc = 0;
SDL_Texture* loc = 0;
SDL_Texture* pBoarder = 0;

int tileSize = 20;
int widthTileNum = 20;
int heightTileNum = 20;


CGameCore::CGameCore(void)
{
	hero_x=7;
	hero_y=11;
	
	iTotalBoxNum = 0;
	iCurrentExactBoxCount = 0;
	
}

CGameCore::~CGameCore(void)
{
}

bool CGameCore::Initialize()
{
	SDL_GetWindowSize(SDLSingleton::GetInstance()->GetWindow(), &m_gameWidth, &m_gameHeight);	

	bg = LoadImage("Resource/back1.png", SDLSingleton::GetInstance()->GetRenderer());
	background = LoadImage("Resource/bagkground.png", SDLSingleton::GetInstance()->GetRenderer());
	box = LoadImage("Resource/box.png", SDLSingleton::GetInstance()->GetRenderer());
	hero = LoadImage("Resource/hero.png", SDLSingleton::GetInstance()->GetRenderer(), true);
	
	bloc = LoadImage("Resource/block.png", SDLSingleton::GetInstance()->GetRenderer());
	loc = LoadImage("Resource/loc.png", SDLSingleton::GetInstance()->GetRenderer());
	pBoarder = LoadImage("Resource/rect.png", SDLSingleton::GetInstance()->GetRenderer());

	return true;
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
	SDL_RWops *fp;
	iCurrentExactBoxCount = 0;
	iTotalBoxNum = 0;
	char szFileName[256];

	int iIndex1 = 0;
	int iIndex2 =0;
	char cChar;

	if(false == m_bLevelReset)		
	{
		hero_x=7;hero_y=11;
		//hero_x=9;hero_y=8;
		//hero_x=9;hero_y=9;


	    sprintf(szFileName, "map%d.txt", iStageNum);		

		fp = SDL_RWFromFile(szFileName, "rb");

		if(fp==NULL)
		{
#ifdef _WIN32
			//MessageBox(m_hWnd,"맵이 존재하지 않습니다. 게임을 종료합니다.","맵이 없어요",MB_OK);
#endif
			return false;
		}
		else 
		{
			
			while (1 == SDL_RWread(fp, &cChar, sizeof(char), 1))
			{ 

				if (iIndex2 > SOCOBAN_MAX_HEIGHT - 1)
				{
					iIndex2 = -2;
					iIndex1++;
				}  

				int iSpaceProperty = (int)cChar;
				iSpaceProperty = iSpaceProperty - 48;

				switch (iSpaceProperty)
				{			
				case ENUM_SPACE_EMPTY:
					{
						array_value[iIndex1][iIndex2] = ENUM_SPACE_EMPTY;
					}
					break;
				case ENUM_SPACE_BLOCK:
					{
						array_value[iIndex1][iIndex2] = ENUM_SPACE_BLOCK;
					}
					break;
				case ENUM_SPACE_BOX_POINT:
					{
						array_value[iIndex1][iIndex2] = ENUM_SPACE_BOX_POINT;
					}
					break;
				case ENUM_SPACE_BOX: // 밀 수 있는 박스
					{
						array_value[iIndex1][iIndex2] = ENUM_SPACE_BOX;
					}
					break;
				case ENUM_SPACE_ENEMY:
					{
						array_value[iIndex1][iIndex2] = ENUM_SPACE_ENEMY;
					}
					break;
				}

				iIndex2++;
			}


		}

		SDL_RWclose(fp);

		for (iIndex1=0; iIndex1 < SOCOBAN_MAX_WIDTH; iIndex1++)
		{
			for (iIndex2 = 0; iIndex2 < SOCOBAN_MAX_HEIGHT; iIndex2++)
			{
				if(array_value[iIndex1][iIndex2] == ENUM_SPACE_BOX)			
				{
					array_value1[iIndex1][iIndex2] = array_value[iIndex1][iIndex2];		
					iTotalBoxNum++;
				}
				else 
				{
					array_value1[iIndex1][iIndex2] = ENUM_SPACE_EMPTY;
				}

				if(array_value[iIndex1][iIndex2] == ENUM_SPACE_BOX_POINT)	
				{
					array_value2[iIndex1][iIndex2] = array_value[iIndex1][iIndex2];
				}
				else
				{
					array_value2[iIndex1][iIndex2] = ENUM_SPACE_EMPTY;
				}

			}
		}

		m_bLevelReset = true;
	}

	return true;
}


void CGameCore::UpdateGameStatus(){
	int miro_mode=0,iCurrentExactBoxCount=0;
	int i,j,s=0;
	int direct=2;
	for(i=0;i<widthTileNum;i++)
		for(j=0;j<heightTileNum;j++)
			if(array_value[i][j]== ENUM_SPACE_BOX && array_value2[i][j] == ENUM_SPACE_BOX_POINT)
				iCurrentExactBoxCount++;

	if (iCurrentExactBoxCount == iTotalBoxNum)
	{ 
		//MessageBox(m_hWnd,"축하합니다. 다음 단계로 넘어갑니다.","성공!",MB_OK);	
		m_bLevelReset = 0;	
		iStageNum++;	
		iTotalBoxNum = 0;
	}

	iCurrentExactBoxCount=0;
}

void CGameCore::ProcessInputWithTouch(float x, float y)
{

	x *= GAME_WIDTH;
	y *= GAME_HEIGHT;
	x = x / (float)widthTileNum;
	y = y / (float)heightTileNum;	

	int fingerX = x;
	int fingerY = y;

	if (hero_x == fingerX && fingerY <= hero_y - 1)
	{
		ProcessDirective(0, -1);
	}
	else if (hero_x - 1 >= fingerX && fingerY == hero_y)
	{
		ProcessDirective(-1, 0);
	}
	else if (hero_x == fingerX && fingerY >= hero_y + 1)
	{
		ProcessDirective(0, 1);
	}
	else if (hero_x + 1 <= fingerX && fingerY == hero_y)
	{
		ProcessDirective(1, 0);
	}
}

void CGameCore::ProcessInput (int scanCode)
{
	if (scanCode == SDL_SCANCODE_UP)
	{
		ProcessDirective(0, -1);
	}
	if (scanCode == SDL_SCANCODE_LEFT)
	{
		ProcessDirective(-1, 0);
	}
	if (scanCode == SDL_SCANCODE_DOWN)
	{
		ProcessDirective(0, 1);
	}
	if (scanCode == SDL_SCANCODE_RIGHT)
	{
		ProcessDirective(1, 0);
	}
	if (scanCode == SDL_SCANCODE_P)
	{
		m_bLevelReset = false;
	}
}

bool CGameCore::ProcessGame()
{
	if (m_bLevelReset == false)
		InitStage();

	//ProcessInput();

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

	destRect.w = SDL_RATIO_X(destRect.w);
	destRect.h = SDL_RATIO_Y(destRect.h);

	SDL_SetTextureAlphaMod(src, 255);
	SDL_RenderCopyEx(SDLSingleton::GetInstance()->GetRenderer(), src, &srcRect, &destRect, 0, 0, SDL_FLIP_NONE);

}

void CGameCore::DrawWall()
{

	for (int index = 1; index< widthTileNum - 1; index++)
	{
		for (int index2 = 1; index2 <heightTileNum; index2++)
		{
			if (*(array_value + (index2 * tileSize) + index) == 0 &&
				*(array_value2 + (index2 * tileSize) + index) == 0)
			{
				DrawTexture(index*tileSize, index2*tileSize, tileSize, tileSize, background);
			}
			else if (*(&array_value[0][0] + (index2 * tileSize) + index) == 1)
			{
				DrawTexture(index*tileSize, index2*tileSize, tileSize, tileSize, pBoarder);
			}
		}
	}

}

void CGameCore::DrawBox()
{
	for (int index = 1; index< widthTileNum - 1; index++)
	{
		for (int index2 = 1; index2 <heightTileNum; index2++)
		{
		
			if (*(&array_value[0][0] + (index2 * tileSize) + index) == 3)
			{
				DrawTexture(index*tileSize, index2*tileSize, tileSize, tileSize, box);
			}
			else if (*(&array_value2[0][0] + (index2 * tileSize) + index) == 2)
			{
				DrawTexture(index*tileSize, index2*tileSize, tileSize, tileSize, loc);
			}
		}
	}
}

void CGameCore::DrawGround()
{
	for (int index = 1; index <widthTileNum-1; index++)
	for (int index2 = 1; index2 <heightTileNum; index2++)
		DrawTexture(index*tileSize, index2*tileSize, tileSize, tileSize, background);
}


void CGameCore::DrawPlayer()
{
	DrawTexture(hero_x*tileSize, hero_y*tileSize, tileSize, tileSize, hero);
}

void CGameCore::DrawBoader()
{
	for (int index = 0; index < widthTileNum; index++)
		DrawTexture(index * tileSize, 0, tileSize, tileSize, pBoarder);

	for (int index = 0; index < widthTileNum; index++)
		DrawTexture(index * tileSize, tileSize * widthTileNum, tileSize, tileSize, pBoarder);

	for (int index = 0; index < heightTileNum; index++)
		DrawTexture(0, index * tileSize, tileSize, tileSize, pBoarder);

	for (int index = 0; index < heightTileNum; index++)
		DrawTexture(tileSize * (heightTileNum - 1), index * tileSize, tileSize, tileSize, pBoarder);
}



void CGameCore::Render()
{		
	DrawBoader();
	DrawGround();
	DrawWall();
	DrawBox();
	DrawPlayer();
}

bool CGameCore::ProcessDirective(int iX, int iY)
{
	hero_x += iX;
	hero_y += iY;
	if(array_value[hero_y][hero_x] == ENUM_SPACE_BLOCK)
	{
		hero_x -= iX;
		hero_y -= iY;
		return false;
	}

	else if(array_value[hero_y][hero_x] == ENUM_SPACE_BOX)
	{
		if (array_value[hero_y + iY][hero_x + iX] == ENUM_SPACE_EMPTY || 
			array_value[hero_y + iY][hero_x + iX] == ENUM_SPACE_BOX_POINT)				
		{
			array_value[hero_y][hero_x] = ENUM_SPACE_EMPTY;				
			array_value[hero_y + iY][hero_x + iX] = ENUM_SPACE_BOX;
		}				
		else 
		{
			hero_x -= iX;
			hero_y -= iY;
			return false;
		}
	}

	return true;
}