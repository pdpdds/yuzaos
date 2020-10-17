#include "windef.h"
#include <memory.h>
#include <skyoswindow.h>
#include <SkyInputHandler.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <yuzaapi.h>
#include "panel.h"
#include <systemcall_impl.h>


/**
 *  사각형의 너비를 반환
 */
inline int GetRectangleWidth(const RECT* pstArea)
{
	int iWidth;

	iWidth = pstArea->right - pstArea->left + 1;

	if (iWidth < 0)
	{
		return -iWidth;
	}

	return iWidth;
}

/**
 *  사각형의 높이를 반환
 */
inline int GetRectangleHeight(const RECT* pstArea)
{
	int iHeight;

	iHeight = pstArea->bottom - pstArea->top + 1;

	if (iHeight < 0)
	{
		return -iHeight;
	}

	return iHeight;
}

int main(int argc, char** argv)
{
	bool bApplicationPanelEventResult;
	bool bApplicationListEventResult;

	//--------------------------------------------------------------------------
	// 윈도우를 생성
	//--------------------------------------------------------------------------
	// 애플리케이션 패널 윈도우와 응용프로그램 리스트 윈도우를 생성
	if ((CreateApplicationPanelWindow() == FALSE) ||
		(CreateApplicationListWindow() == FALSE))
	{
		return 0;
	}

	//--------------------------------------------------------------------------
	// GUI 태스크의 이벤트 처리 루프
	//--------------------------------------------------------------------------
	while (1)
	{
		// 윈도우 이벤트를 처리
		bApplicationPanelEventResult = ProcessApplicationPanelWindowEvent();
		bApplicationListEventResult = ProcessApplicationListWindowEvent();

		// 처리한 이벤트가 없으면 프로세서를 반환
		if ((bApplicationPanelEventResult == FALSE) &&
			(bApplicationListEventResult == FALSE))
		{
			int j = 1;
			//20180628
			Syscall_Sleep(1);
		}
	}

	return 0;
}