#include "gdi32.h"
#include <string.h>
#include <math.h>
#include <memory.h>
#include <systemcall_impl.h>
#include <stdarg.h>

// 난수를 발생시키기 위한 변수
static volatile QWORD gs_qwRandomValue = 0;
#define KEY_FLAGS_DOWN           0x01

QWORD HexStringToQword(const char* pcBuffer)
{
	QWORD qwValue = 0;
	int i;

	// 문자열을 돌면서 차례로 변환
	for (i = 0; pcBuffer[i] != '\0'; i++)
	{
		qwValue *= 16;
		if (('A' <= pcBuffer[i]) && (pcBuffer[i] <= 'Z'))
		{
			qwValue += (pcBuffer[i] - 'A') + 10;
		}
		else if (('a' <= pcBuffer[i]) && (pcBuffer[i] <= 'z'))
		{
			qwValue += (pcBuffer[i] - 'a') + 10;
		}
		else
		{
			qwValue += pcBuffer[i] - '0';
		}
	}
	return qwValue;
}

/**
 *  10진수 문자열을 long으로 변환
 */
long DecimalStringToLong(const char* pcBuffer)
{
	long lValue = 0;
	int i;

	if (pcBuffer[0] == '-')
	{
		i = 1;
	}
	else
	{
		i = 0;
	}

	// 문자열을 돌면서 차례로 변환
	for (; pcBuffer[i] != '\0'; i++)
	{
		lValue *= 10;
		lValue += pcBuffer[i] - '0';
	}

	// 음수이면 - 추가
	if (pcBuffer[0] == '-')
	{
		lValue = -lValue;
	}
	return lValue;
}

void ReverseString(char* pcBuffer)
{
	int iLength;
	int i;
	char cTemp;


	iLength = strlen(pcBuffer);
	for (i = 0; i < iLength / 2; i++)
	{
		cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iLength - 1 - i];
		pcBuffer[iLength - 1 - i] = cTemp;
	}
}

int HexToString(QWORD qwValue, char* pcBuffer)
{
	QWORD i;
	QWORD qwCurrentValue;

	// 0이 들어오면 바로 처리
	if (qwValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	// 버퍼에 1의 자리부터 16, 256, ...의 자리 순서로 숫자 삽입
	for (i = 0; qwValue > 0; i++)
	{
		qwCurrentValue = qwValue % 16;
		if (qwCurrentValue >= 10)
		{
			pcBuffer[i] = 'A' + (qwCurrentValue - 10);
		}
		else
		{
			pcBuffer[i] = '0' + qwCurrentValue;
		}

		qwValue = qwValue / 16;
	}
	pcBuffer[i] = '\0';

	// 버퍼에 들어있는 문자열을 뒤집어서 ... 256, 16, 1의 자리 순서로 변경
	ReverseString(pcBuffer);
	return i;
}

/**
 *  10진수 값을 문자열로 변환
 */
int DecimalToString(long lValue, char* pcBuffer)
{
	long i;

	// 0이 들어오면 바로 처리
	if (lValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	if (lValue < 0)
	{
		i = 1;
		pcBuffer[0] = '-';
		lValue = -lValue;
	}
	else
	{
		i = 0;
	}

	for (; lValue > 0; i++)
	{
		pcBuffer[i] = '0' + lValue % 10;
		lValue = lValue / 10;
	}
	pcBuffer[i] = '\0';

	if (pcBuffer[0] == '-')
	{

		ReverseString(&(pcBuffer[1]));
	}
	else
	{
		ReverseString(pcBuffer);
	}

	return i;
}

//==============================================================================
// GUI 시스템 관련
//==============================================================================
/**
 *  (x, y)가 사각형 영역 안에 있는지 여부를 반환
 */
bool IsInRectangle(const RECT* pstArea, int iX, int iY)
{
	// 화면에 표시되는 영역을 벗어났다면 그리지 않음
	if ((iX < pstArea->left) || (pstArea->right < iX) ||
		(iY < pstArea->top) || (pstArea->bottom < iY))
	{
		return FALSE;
	}

	return TRUE;
}

/**
 *  사각형의 너비를 반환
 */
int GetRectangleWidth(const RECT* pstArea)
{
	int iWidth;

	iWidth = pstArea->left - pstArea->right + 1;

	if (iWidth < 0)
	{
		return -iWidth;
	}

	return iWidth;
}

/**
 *  사각형의 높이를 반환
 */
int GetRectangleHeight(const RECT* pstArea)
{
	int iHeight;

	iHeight = pstArea->bottom - pstArea->top + 1;

	if (iHeight < 0)
	{
		return -iHeight;
	}

	return iHeight;
}

/**
 *  영역 1과 영역 2의 겹치는 영역을 반환
 */
bool GetOverlappedRectangle(const RECT* pstArea1, const RECT* pstArea2,
	RECT* pstIntersection)
{
	int iMaxX1;
	int iMinX2;
	int iMaxY1;
	int iMinY2;

	// X축의 시작점은 두 점 중에서 큰 것을 찾음
	iMaxX1 = MAX(pstArea1->left, pstArea2->left);
	// X축의 끝점은 두 점 중에서 작은 것을 찾음
	iMinX2 = MIN(pstArea1->right, pstArea2->right);
	// 계산한 시작점의 위치가 끝점의 위치보다 크다면 두 사각형은 겹치지 않음
	if (iMinX2 < iMaxX1)
	{
		return FALSE;
	}

	// Y축의 시작점은 두 점 중에서 큰 것을 찾음
	iMaxY1 = MAX(pstArea1->top, pstArea2->top);
	// Y축의 끝점은 두 점 중에서 작은 것을 찾음
	iMinY2 = MIN(pstArea1->bottom, pstArea2->bottom);
	// 계산한 시작점의 위치가 끝점의 위치보다 크다면 두 사각형은 겹치지 않음
	if (iMinY2 < iMaxY1)
	{
		return FALSE;
	}

	// 겹치는 영역의 정보 저장
	pstIntersection->left = iMaxX1;
	pstIntersection->top = iMaxY1;
	pstIntersection->right = iMinX2;
	pstIntersection->bottom = iMinY2;

	return TRUE;
}

/**
 *  전체 화면을 기준으로 한 X,Y 좌표를 윈도우 내부 좌표로 변환
 */
bool ConvertPointScreenToClient(QWORD qwWindowID, const POINT* pstXY,
	POINT* pstXYInWindow)
{
	RECT stArea;

	// 윈도우 영역을 반환
	if (Syscall_GetWindowArea(&qwWindowID, &stArea) == FALSE)
	{
		return FALSE;
	}

	pstXYInWindow->iX = pstXY->iX - stArea.left;
	pstXYInWindow->iY = pstXY->iY - stArea.top;
	return TRUE;
}

/**
 *  윈도우 내부를 기준으로 한 X,Y 좌표를 화면 좌표로 변환
 */
bool ConvertPointClientToScreen(QWORD qwWindowID, const POINT* pstXY,
	POINT* pstXYInScreen)
{
	RECT stArea;

	// 윈도우 영역을 반환
	if (Syscall_GetWindowArea(&qwWindowID, &stArea) == FALSE)
	{
		return FALSE;
	}

	pstXYInScreen->iX = pstXY->iX + stArea.left;
	pstXYInScreen->iY = pstXY->iY + stArea.top;
	return TRUE;
}

/**
 *  전체 화면을 기준으로 한 사각형 좌표를 윈도우 내부 좌표로 변환
 */
bool ConvertRectScreenToClient(QWORD qwWindowID, const RECT* pstArea,
	RECT* pstAreaInWindow)
{
	RECT stWindowArea;

	// 윈도우 영역을 반환
	if (Syscall_GetWindowArea(&qwWindowID, &stWindowArea) == FALSE)
	{
		return FALSE;
	}

	pstAreaInWindow->left = pstArea->left - stWindowArea.left;
	pstAreaInWindow->top = pstArea->top - stWindowArea.top;
	pstAreaInWindow->right = pstArea->right - stWindowArea.left;
	pstAreaInWindow->bottom = pstArea->bottom - stWindowArea.top;
	return TRUE;
}

/**
 *  윈도우 내부를 기준으로 한 사각형 좌표를 화면 좌표로 변환
 */
bool ConvertRectClientToScreen(QWORD qwWindowID, const RECT* pstArea,
	RECT* pstAreaInScreen)
{
	RECT stWindowArea;

	// 윈도우 영역을 반환
	if (Syscall_GetWindowArea(&qwWindowID, &stWindowArea) == FALSE)
	{
		return FALSE;
	}

	pstAreaInScreen->left = pstArea->left + stWindowArea.left;
	pstAreaInScreen->top = pstArea->top + stWindowArea.top;
	pstAreaInScreen->right = pstArea->right + stWindowArea.left;
	pstAreaInScreen->bottom = pstArea->bottom + stWindowArea.top;
	return TRUE;
}

/**
 *  사각형 자료구조를 채움
 *      x1과 x2, y1과 y2를 비교해서 x1 < x2, y1 < y2가 되도록 저장
 */
void SetRectangleData(int iX1, int iY1, int iX2, int iY2, RECT* pstRect)
{
	// x1 < x2가 되도록 RECT 자료구조에 X좌표를 설정
	if (iX1 < iX2)
	{
		pstRect->left = iX1;
		pstRect->right = iX2;
	}
	else
	{
		pstRect->left = iX2;
		pstRect->right = iX1;
	}

	// y1 < y2가 되도록 RECT 자료구조에 Y좌표를 설정
	if (iY1 < iY2)
	{
		pstRect->top = iY1;
		pstRect->bottom = iY2;
	}
	else
	{
		pstRect->top = iY2;
		pstRect->bottom = iY1;
	}
}

/**
 *  마우스 이벤트 자료구조를 설정
 */
bool SetMouseEvent(QWORD qwWindowID, QWORD qwEventType, int iMouseX, int iMouseY,
	BYTE bButtonStatus, EVENT* pstEvent)
{
	POINT stMouseXYInWindow;
	POINT stMouseXY;

	switch (qwEventType)
	{
	case EVENT_MOUSE_MOVE:
	case EVENT_MOUSE_LBUTTONDOWN:
	case EVENT_MOUSE_LBUTTONUP:
	case EVENT_MOUSE_RBUTTONDOWN:
	case EVENT_MOUSE_RBUTTONUP:
	case EVENT_MOUSE_MBUTTONDOWN:
	case EVENT_MOUSE_MBUTTONUP:
	{
		// 마우스의 X, Y좌표를 설정
		stMouseXY.iX = iMouseX;
		stMouseXY.iY = iMouseY;

		// 마우스 X, Y좌표를 윈도우 내부 좌표로 변환
		if (ConvertPointScreenToClient(qwWindowID, &stMouseXY, &stMouseXYInWindow) == false)
		{
			return FALSE;
		}

		// 이벤트 타입 설정
		pstEvent->qwType = qwEventType;
		// 윈도우 ID 설정
		pstEvent->stMouseEvent.qwWindowID = qwWindowID;
		// 마우스 버튼의 상태 설정
		pstEvent->stMouseEvent.bButtonStatus = bButtonStatus;
		// 마우스 커서의 좌표를 윈도우 내부 좌표로 변환한 값을 설정
		memcpy(&(pstEvent->stMouseEvent.stPoint), &stMouseXYInWindow,
			sizeof(POINT));
	}
	break;
	default:
		return FALSE;
		break;
	}
	return TRUE;
}

/**
 *  윈도우 이벤트 자료구조를 설정
 */
bool SetWindowEvent(QWORD qwWindowID, QWORD qwEventType, EVENT* pstEvent)
{
	RECT stArea;

	// 이벤트 종류를 확인하여 윈도우 이벤트 생성
	switch (qwEventType)
	{
		// 윈도우 이벤트 처리
	case EVENT_WINDOW_SELECT:
	case EVENT_WINDOW_DESELECT:
	case EVENT_WINDOW_MOVE:
	case EVENT_WINDOW_RESIZE:
	case EVENT_WINDOW_CLOSE:
		// 이벤트 타입 설정
		pstEvent->qwType = qwEventType;
		// 윈도우 ID 설정
		pstEvent->stWindowEvent.qwWindowID = qwWindowID;
		// 윈도우 영역을 반환
		if (Syscall_GetWindowArea(&qwWindowID , &stArea) == FALSE)
		{
			return FALSE;
		}

		// 윈도우의 현재 좌표를 설정
		memcpy(&(pstEvent->stWindowEvent.stArea), &stArea, sizeof(RECT));
		break;

	default:
		return FALSE;
		break;
	}
	return TRUE;
}

/**
 *  키 이벤트 자료구조를 설정
 */
void SetKeyEvent(QWORD qwWindow, const KEYDATA* pstKeyData, EVENT* pstEvent)
{
	// 눌림 또는 떨어짐 처리
	if (pstKeyData->bFlags & KEY_FLAGS_DOWN)
	{
		pstEvent->qwType = EVENT_KEY_DOWN;
	}
	else
	{
		pstEvent->qwType = EVENT_KEY_UP;
	}

	// 키의 각 정보를 설정
	pstEvent->stKeyEvent.bASCIICode = pstKeyData->bASCIICode;
	pstEvent->stKeyEvent.bScanCode = pstKeyData->bScanCode;
	pstEvent->stKeyEvent.bFlags = pstKeyData->bFlags;
}