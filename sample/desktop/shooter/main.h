#pragma once
#include <windef.h>
#include <skyoswindow.h>

////////////////////////////////////////////////////////////////////////////////
//
// 매크로
//
////////////////////////////////////////////////////////////////////////////////
// 물방울의 최대 개수
#define MAXBUBBLECOUNT      50
// 물방울의 반지름
#define RADIUS              16
// 물방울의 기본 속도
#define DEFAULTSPEED        3
// 플레이어의 최대 생명
#define MAXLIFE             20

// 윈도우의 너비와 높이
#define WINDOW_WIDTH        250
#define WINDOW_HEIGHT       350

// 게임 정보 영역의 높이
#define INFORMATION_HEIGHT  20


////////////////////////////////////////////////////////////////////////////////
//
// 구조체
//
////////////////////////////////////////////////////////////////////////////////
// 물방울의 정보를 저장하는 자료구조
typedef struct BubbleStruct
{
	// X, Y 좌표
	int qwX;
	int qwY;

	// 떨어지는 속도(Y축 변화량)
	int qwSpeed;

	// 물방울 색깔
	COLOR stColor;

	// 살아있는지 여부
	bool bAlive;
} BUBBLE;

// 게임 정보를 저장하는 자료구조
typedef struct GameInfoStruct
{
	//-------------------------------------------------------------------------
	// 물방울을 관리하는데 필요한 필드
	//-------------------------------------------------------------------------
	// 물방울의 정보를 저장하는 버퍼
	BUBBLE* pstBubbleBuffer;

	// 살아 있는 물방울의 수
	int iAliveBubbleCount;

	//-------------------------------------------------------------------------
	// 게임을 진행하는데 필요한 필드
	//-------------------------------------------------------------------------
	// 플레이어의 생명
	int iLife;

	// 유저의 점수
	QWORD qwScore;

	// 게임의 시작 여부
	bool bGameStart;
} GAMEINFO;

////////////////////////////////////////////////////////////////////////////////
//
// 함수
//
////////////////////////////////////////////////////////////////////////////////
bool Initialize(void);
bool CreateBubble(void);
void MoveBubble(void);
void DeleteBubbleUnderMouse(POINT* pstMouseXY);
void DrawInformation(QWORD qwWindowID);
void DrawGameArea(QWORD qwWindowID, POINT* pstMouseXY);