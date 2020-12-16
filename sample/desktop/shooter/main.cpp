#include "main.h"
#include <systemcall_impl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gdi32.h>


#define FONT_ENGLISHWIDTH   8
#define FONT_ENGLISHHEIGHT  16

// 한글 폰트의 너비와 길이
#define FONT_HANGULWIDTH   16
#define FONT_HANGULHEIGHT  16

// 게임 관련 정보를 저장하는 자료구조
GAMEINFO g_stGameInfo = { 0, };

int main(int argc, char** argv)
{
	QWORD qwWindowID;
	EVENT stEvent;
	QWORD qwLastTickCount;
	char* pcStartMessage = "Please LButton Down To Start~!";
	POINT stMouseXY;
	RECT stScreenArea;
	int iX;
	int iY;

	//--------------------------------------------------------------------------
	// 윈도우를 화면 가운데에 가로x세로 250x350 크기로 생성
	//--------------------------------------------------------------------------
	Syscall_GetScreenArea(&stScreenArea);
	iX = (GetRectangleWidth(&stScreenArea) - WINDOW_WIDTH) / 2;
	iY = (GetRectangleHeight(&stScreenArea) - WINDOW_HEIGHT) / 2;
	RECT rect;
	rect.left = iX;
	rect.top = iY;
	rect.right = rect.left + WINDOW_WIDTH;
	rect.bottom = rect.top + WINDOW_HEIGHT;
	Syscall_CreateWindow(&rect, "Bubble Shooter", WINDOW_FLAGS_DEFAULT, &qwWindowID);
	if (qwWindowID == WINDOW_INVALIDID)
	{
		printf("Window create fail\n");
		return -1;
	}

	//--------------------------------------------------------------------------
	// 게임에 관련된 정보를 초기화하고 사용할 버퍼를 할당
	//--------------------------------------------------------------------------
	// 마우스 좌표 설정
	stMouseXY.iX = WINDOW_WIDTH / 2;
	stMouseXY.iY = WINDOW_HEIGHT / 2;

	// 게임 정보를 초기화
	if (Initialize() == FALSE)
	{
		// 초기화에 실패하면 윈도우를 삭제
		Syscall_DeleteWindow(&qwWindowID);
		return -1;
	}

	// 난수 초깃값(Random Seed) 설정
	srand(Syscall_GetTickCount());

	//--------------------------------------------------------------------------
	// 게임 정보와 게임 영역을 출력하고 게임 시작 대기 메시지를 표시
	//--------------------------------------------------------------------------
	DrawInformation(qwWindowID);
	DrawGameArea(qwWindowID, &stMouseXY);
	POINT point;
	point.iX = 5;
	point.iY = 150;

	TEXTCOLOR textColor;
	textColor.textColor = RGB(255, 255, 255);
	textColor.backgroundColor = RGB(0, 0, 0);
	Syscall_DrawText(&qwWindowID, &point, &textColor, pcStartMessage, strlen(pcStartMessage));

	// 출력된 메시지를 화면에 표시
	Syscall_ShowWindow(&qwWindowID, TRUE);

	//--------------------------------------------------------------------------
	// GUI 태스크의 이벤트와 게임 루프를 처리하는 부분
	//--------------------------------------------------------------------------
	qwLastTickCount = Syscall_GetTickCount();
	while (1)
	{
		//----------------------------------------------------------------------
		// 이벤트 처리 부분
		//----------------------------------------------------------------------
		// 이벤트 큐에서 이벤트를 수신
		if (Syscall_ReceiveEventFromWindowQueue(&qwWindowID, &stEvent) == TRUE)
		{
			// 수신된 이벤트를 타입에 따라 나누어 처리
			switch (stEvent.qwType)
			{
				// 마우스 클릭 처리
			case EVENT_MOUSE_LBUTTONDOWN:
				// 게임 시작을 원하는 클릭이면 게임을 시작
				if (g_stGameInfo.bGameStart == FALSE)
				{
					// 게임 정보를 초기화
					Initialize();

					// 게임 시작 플래그를 설정
					g_stGameInfo.bGameStart = TRUE;
					break;
				}

				// 마우스가 클릭된 곳에 있는 물방울을 삭제
				DeleteBubbleUnderMouse(&(stEvent.stMouseEvent.stPoint));

				// 마우스 위치 저장
				memcpy(&stMouseXY, &(stEvent.stMouseEvent.stPoint),
					sizeof(stMouseXY));
				break;

				// 마우스 이동  정보
			case EVENT_MOUSE_MOVE:
				// 마우스 위치 저장
				memcpy(&stMouseXY, &(stEvent.stMouseEvent.stPoint),
					sizeof(stMouseXY));
				break;

				// 윈도우 닫기 버튼 처리
			case EVENT_WINDOW_CLOSE:
				// 윈도우를 삭제하고 메모리를 해제
				Syscall_DeleteWindow(&qwWindowID);
				free(g_stGameInfo.pstBubbleBuffer);
				return 0;
				break;
			}
		}

		//----------------------------------------------------------------------
		// 게임 루프 처리 부분
		//----------------------------------------------------------------------
		// 게임이 시작 되었다면 50ms마다 생성된 물방울을 아래로 이동
		if ((g_stGameInfo.bGameStart == TRUE) &&
			((Syscall_GetTickCount() - qwLastTickCount) > 50))
		{
			qwLastTickCount = Syscall_GetTickCount();

			// 물방울을 생성
			if ((rand() % 7) == 1)
			{
				CreateBubble();
			}

			// 물방울을 이동
			MoveBubble();

			// 게임 영역을 표시
			DrawGameArea(qwWindowID, &stMouseXY);

			// 게임 정보를 표시
			DrawInformation(qwWindowID);

			// 플레이어의 생명이 0이라면 게임 종료
			if (g_stGameInfo.iLife <= 0)
			{
				g_stGameInfo.bGameStart = FALSE;

				// 게임 종료 메시지를 출력
				POINT point;
				point.iX = 80;
				point.iY = 130;

				TEXTCOLOR textColor;
				textColor.textColor = RGB(255, 255, 255);
				textColor.backgroundColor = RGB(0, 0, 0);
				Syscall_DrawText(&qwWindowID, &point, &textColor,"Game Over~!!!", 13);

				textColor.textColor = RGB(255, 255, 255);
				textColor.backgroundColor = RGB(0, 0, 0);
				point.iX = 5;
				point.iY = 150;
				Syscall_DrawText(&qwWindowID, &point, &textColor, pcStartMessage, strlen(pcStartMessage));
			}

			// 변경된 윈도우의 내부를 화면에 업데이트
			Syscall_ShowWindow(&qwWindowID, TRUE);
		}
		else
		{
			Syscall_Sleep(0);
		}
	}

	return 0;
}

/**
 *  게임에 관련된 정보를 초기화
 */
bool Initialize(void)
{
	// 물방울의 최대 개수만큼 메모리를 할당
	if (g_stGameInfo.pstBubbleBuffer == NULL)
	{
		g_stGameInfo.pstBubbleBuffer = (BUBBLE*)malloc(sizeof(BUBBLE) * MAXBUBBLECOUNT);
		if (g_stGameInfo.pstBubbleBuffer == NULL)
		{
			printf("Memory allocate fail\n");
			return FALSE;
		}
	}

	// 물방울의 정보를 초기화
	memset(g_stGameInfo.pstBubbleBuffer, 0, sizeof(BUBBLE) *
		MAXBUBBLECOUNT);
	g_stGameInfo.iAliveBubbleCount = 0;

	// 게임이 시작되었다는 정보와 점수, 그리고 생명 설정
	g_stGameInfo.bGameStart = FALSE;
	g_stGameInfo.qwScore = 0;
	g_stGameInfo.iLife = MAXLIFE;

	return TRUE;
}

/**
 *  물방울을 생성
 */
bool CreateBubble(void)
{
	BUBBLE* pstTarget;
	int i;

	// 물방울의 최대 개수와 살아있는 물방울의 개수를 비교하여 생성할지 여부를 결정
	if (g_stGameInfo.iAliveBubbleCount >= MAXBUBBLECOUNT)
	{
		return FALSE;
	}

	// 빈 물방울 자료구조를 검색
	for (i = 0; i < MAXBUBBLECOUNT; i++)
	{
		// 물방울이 살아있지 않으면 다시 할당해서 사용
		if (g_stGameInfo.pstBubbleBuffer[i].bAlive == FALSE)
		{
			// 선택된 물방울 자료구조
			pstTarget = &(g_stGameInfo.pstBubbleBuffer[i]);

			// 물방울이 살아 있다고 설정하고 물방울의 이동 속도를 초기화
			pstTarget->bAlive = TRUE;
			pstTarget->qwSpeed = (rand() % 8) + DEFAULTSPEED;

			// X좌표와 Y좌표는 물방울이 게임 영역 내부에 위치하도록 설정
			pstTarget->qwX = rand() % (WINDOW_WIDTH - 2 * RADIUS) + RADIUS;
			pstTarget->qwY = INFORMATION_HEIGHT + WINDOW_TITLEBAR_HEIGHT + RADIUS + 1;

			// 물방울의 색깔 설정
			pstTarget->stColor = RGB(rand() % 256, rand() % 256, rand() % 256);

			// 살아있는 물방울의 수를 증가
			g_stGameInfo.iAliveBubbleCount++;
			return TRUE;
		}
	}

	return FALSE;
}

/**
 *  물방울을 이동
 */
void MoveBubble(void)
{
	BUBBLE* pstTarget;
	int i;

	// 살아있는 모든 물방울을 이동
	for (i = 0; i < MAXBUBBLECOUNT; i++)
	{
		// 물방울이 살아있으면 이동
		if (g_stGameInfo.pstBubbleBuffer[i].bAlive == TRUE)
		{
			// 현재 물방울 자료구조
			pstTarget = &(g_stGameInfo.pstBubbleBuffer[i]);

			// 물방울의 Y좌표에 이동 속도를 더함
			pstTarget->qwY += pstTarget->qwSpeed;

			// 게임 영역 끝에 닿으면 물방울을 제거하고 플레이어가 물방울을 막지
			// 못했으므로 생명을 하나 줄임
			if ((pstTarget->qwY + RADIUS) >= WINDOW_HEIGHT)
			{
				pstTarget->bAlive = FALSE;

				// 살아 있는 물방울의 수를 줄이고 생명도 하나 줄임
				g_stGameInfo.iAliveBubbleCount--;
				if (g_stGameInfo.iLife > 0)
				{
					g_stGameInfo.iLife--;
				}
			}
		}
	}
}

/**
 *  마우스 아래에 있는 물방울을 삭제하고 점수를 증가
 */
void DeleteBubbleUnderMouse(POINT * pstMouseXY)
{
	BUBBLE* pstTarget;
	int i;
	QWORD qwDistance;

	// 살아있는 모든 물방울을 검색하여 마우스 아래에 있는 물방울을 제거
	for (i = MAXBUBBLECOUNT - 1; i >= 0; i--)
	{
		// 물방울이 살아있으면 거리를 계산해서 삭제 여부를 결정
		if (g_stGameInfo.pstBubbleBuffer[i].bAlive == TRUE)
		{
			// 현재 물방울 자료구조
			pstTarget = &(g_stGameInfo.pstBubbleBuffer[i]);

			// 마우스가 클릭된 위치와 원의 중심이 반지름 거리 안쪽이면 삭제
			qwDistance = ((pstMouseXY->iX - pstTarget->qwX) *
				(pstMouseXY->iX - pstTarget->qwX)) +
				((pstMouseXY->iY - pstTarget->qwY) *
				(pstMouseXY->iY - pstTarget->qwY));
			// 물방울의 중심과 마우스 클릭 위치 사이의 거리를 물방울 반지름과
			// 비교하여 작다면 물방울 내부에 클릭된 것이므로 삭제
			if (qwDistance < (RADIUS * RADIUS))
			{
				pstTarget->bAlive = FALSE;

				// 살아있는 물방울의 수를 줄이고 점수를 증가
				g_stGameInfo.iAliveBubbleCount--;
				g_stGameInfo.qwScore++;
				break;
			}
		}
	}
}

/**
 *  게임 정보를 화면에 출력
 */
void DrawInformation(QWORD qwWindowID)
{
	char vcBuffer[200];
	int iLength;

	// 게임 정보 영역을 표시
	RECT rect;
	rect.left = 1;
	rect.top = WINDOW_TITLEBAR_HEIGHT - 1;
	rect.right = WINDOW_WIDTH - 2;
	rect.bottom = WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT;
	Syscall_DrawRect(&qwWindowID, &rect, RGB(55, 215, 47), TRUE);

	// 임시 버퍼에 출력할 정보를 저장
	sprintf(vcBuffer, "Life: %d, Score: %ld\n", g_stGameInfo.iLife, g_stGameInfo.qwScore);
	iLength = strlen(vcBuffer);

	// 저장된 정보를 게임 정보 표시 영역의 가운데에 출력
	POINT point;
	point.iX = (WINDOW_WIDTH - iLength * FONT_ENGLISHWIDTH) / 2;
	point.iY = WINDOW_TITLEBAR_HEIGHT + 2;

	TEXTCOLOR textColor;
	textColor.textColor = RGB(255, 255, 255);
	textColor.backgroundColor = RGB(55, 215, 47);
	Syscall_DrawText(&qwWindowID, &point, &textColor, vcBuffer, strlen(vcBuffer));
}

/**
 *  게임 영역에 물방울을 표시
 */
void DrawGameArea(QWORD qwWindowID, POINT * pstMouseXY)
{
	BUBBLE* pstTarget;
	int i;

	// 게임 영역의 배경을 초기화
	RECT rect;
	rect.left = 0;
	rect.top = WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT;
	rect.right = WINDOW_WIDTH - 1;
	rect.bottom = WINDOW_HEIGHT - 1;
	Syscall_DrawRect(&qwWindowID, &rect, RGB(0, 0, 0), TRUE);

	// 살아있는 모든 물방울을 표시
	for (i = 0; i < MAXBUBBLECOUNT; i++)
	{
		// 물방울이 살아있으면 화면에 표시
		if (g_stGameInfo.pstBubbleBuffer[i].bAlive == TRUE)
		{
			// 현재 물방울 자료구조
			pstTarget = &(g_stGameInfo.pstBubbleBuffer[i]);

			// 물방울의 내부와 외부를 그림
			Syscall_DrawCircle(pstTarget->qwX, pstTarget->qwY, RADIUS,
				pstTarget->stColor, TRUE);
			Syscall_DrawCircle(pstTarget->qwX, pstTarget->qwY, RADIUS,
				~pstTarget->stColor, FALSE);
		}
	}

	// 마우스가 있는 위치를 검사하여 조준선 표시
	if (pstMouseXY->iY < (WINDOW_TITLEBAR_HEIGHT + RADIUS))
	{
		pstMouseXY->iY = WINDOW_TITLEBAR_HEIGHT + RADIUS;
	}

	// 조준선을 +로 표시
	Syscall_DrawLine(pstMouseXY->iX, pstMouseXY->iY - RADIUS, pstMouseXY->iX, pstMouseXY->iY + RADIUS, RGB(255, 0, 0));
	Syscall_DrawLine(pstMouseXY->iX - RADIUS, pstMouseXY->iY, pstMouseXY->iX + RADIUS, pstMouseXY->iY, RGB(255, 0, 0));


	// 게임 영역의 테두리를 표시
	rect.left = 0;
	rect.top = WINDOW_TITLEBAR_HEIGHT + INFORMATION_HEIGHT;
	rect.right = WINDOW_WIDTH - 1;
	rect.bottom = WINDOW_HEIGHT - 1;
	Syscall_DrawRect(&qwWindowID, &rect, RGB(0, 255, 0), FALSE);
}