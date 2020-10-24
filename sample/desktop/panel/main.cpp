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

int main(int argc, char** argv)
{
	bool bApplicationPanelEventResult;
	bool bApplicationListEventResult;

	if ((CreateApplicationPanelWindow() == FALSE) ||
		(CreateApplicationListWindow() == FALSE))
	{
		return 0;
	}

	//이벤트 루프
	while (1)
	{
		// 윈도우 이벤트를 처리
		bApplicationPanelEventResult = ProcessApplicationPanelWindowEvent();
		bApplicationListEventResult = ProcessApplicationListWindowEvent();

		// 처리한 이벤트가 없으면 프로세서를 반환
		if ((bApplicationPanelEventResult == FALSE) &&
			(bApplicationListEventResult == FALSE))
		{
			Syscall_Sleep(1);
		}
	}

	return 0;
}