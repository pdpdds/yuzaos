/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2016 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
	 claim that you wrote the original software. If you use this software
	 in a product, an acknowledgment in the product documentation would be
	 appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_DUMMY

/* Being a null driver, there's no event stream. We just define stubs for
   most of the API. */

#include "../../events/SDL_events_c.h"

#include "SDL_nullvideo.h"
#include "SDL_nullevents_c.h"
#include "SDL_skyosmouse.h"
#include "SDL_skyoskeyboard.h"
#include "skyoswindow.h"

void
DUMMY_PumpEvents(_THIS)
{		
	EVENT stReceivedEvent;
	MOUSEEVENT* pstMouseEvent;
	KEYEVENT* pstKeyEvent;
	WINDOWEVENT* pstWindowEvent;

	// 이벤트 큐에서 이벤트를 수신
	if (ReceiveEventFromWindowQueue(0, &stReceivedEvent) == TRUE)
	{
		// 수신된 이벤트를 타입에 따라 나누어 처리
		switch (stReceivedEvent.qwType)
		{
			// 마우스 이벤트 처리
		case EVENT_MOUSE_MOVE:
		case EVENT_MOUSE_LBUTTONDOWN:
		case EVENT_MOUSE_LBUTTONUP:
		case EVENT_MOUSE_RBUTTONDOWN:
		case EVENT_MOUSE_RBUTTONUP:
		case EVENT_MOUSE_MBUTTONDOWN:
		case EVENT_MOUSE_MBUTTONUP:
		{
			// 여기에 마우스 이벤트 처리 코드 넣기
			pstMouseEvent = &(stReceivedEvent.stMouseEvent);

			//20191024
			//SDL_Window* focusWindow = SDL_GetWindowFromID(0);
			//SDL_Window* focusWindow = SDL_GetMouseFocus();
			SKYos_OnMouse(_this->windows, stReceivedEvent.qwType, pstMouseEvent->stPoint.iX, pstMouseEvent->stPoint.iY - WINDOW_TITLEBAR_HEIGHT);
		}
		break;

		// 키 이벤트 처리
		case EVENT_KEY_DOWN:
		{
			SDL_SetKeyboardFocus(_this->windows);
			pstKeyEvent = &(stReceivedEvent.stKeyEvent);			
			SkyOS_OnKeyDown(pstKeyEvent->bASCIICode);
		}
		break;
		case EVENT_KEY_UP:
		{
			SDL_SetKeyboardFocus(_this->windows);
			// 여기에 키보드 이벤트 처리 코드 넣기
			pstKeyEvent = &(stReceivedEvent.stKeyEvent);			
			SkyOS_OnKeyUp(pstKeyEvent->bASCIICode);
		}
		break;

			// 윈도우 이벤트 처리
		case EVENT_WINDOW_SELECT:
		case EVENT_WINDOW_DESELECT:
		case EVENT_WINDOW_MOVE:
		case EVENT_WINDOW_RESIZE:
		case EVENT_WINDOW_CLOSE:
			// 여기에 윈도우 이벤트 처리 코드 넣기
			pstWindowEvent = &(stReceivedEvent.stWindowEvent);

			//------------------------------------------------------------------
			// 윈도우 닫기 이벤트이면 윈도우를 삭제하고 루프를 빠져나가 태스크를 종료
			//------------------------------------------------------------------
			if (stReceivedEvent.qwType == EVENT_WINDOW_CLOSE)
			{
				// 윈도우 삭제
				DeleteWindow(0);
				SDL_Quit();
				ExitThread(0);
				
				return;
			}
			break;

			// 그 외 정보
		default:
			// 여기에 알 수 없는 이벤트 처리 코드 넣기
			break;
		}
	}
}


#endif /* SDL_VIDEO_DRIVER_DUMMY */

/* vi: set ts=4 sw=4 expandtab: */
