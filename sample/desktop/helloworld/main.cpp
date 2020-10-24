#include <skyoswindow.h>
#include <systemcall_impl.h>

int main(int argc, char** argv)
{
	QWORD qwWindowID;
	int iMouseX, iMouseY;
	int iWindowWidth, iWindowHeight;
	EVENT stReceivedEvent;
	MOUSEEVENT* pstMouseEvent;
	KEYEVENT* pstKeyEvent;
	WINDOWEVENT* pstWindowEvent;


	//--------------------------------------------------------------------------
	// 윈도우를 생성
	//--------------------------------------------------------------------------
	// 마우스의 현재 위치를 반환
	Syscall_GetCursorPosition(&iMouseX, &iMouseY);

	// 윈도우의 크기와 제목 설정
	iWindowWidth = 500;
	iWindowHeight = 200;

	// 윈도우 생성 함수 호출, 마우스가 있던 위치를 기준으로 생성
	RECT rect;
	rect.left = iMouseX - 10;
	rect.top = iMouseY - WINDOW_TITLEBAR_HEIGHT / 2;
	rect.right = rect.left + iWindowWidth;
	rect.bottom = rect.top + iWindowHeight;
	Syscall_CreateWindow(&rect, "Hello World Window", WINDOW_FLAGS_DEFAULT | WINDOW_FLAGS_RESIZABLE, &qwWindowID);
	
	// 윈도우를 생성하지 못했으면 실패
	if (qwWindowID == WINDOW_INVALIDID)
	{
		return 0;
	}

	//--------------------------------------------------------------------------
	// GUI 태스크의 이벤트 처리 루프
	//--------------------------------------------------------------------------
	while (1)
	{
		// 이벤트 큐에서 이벤트를 수신
		if (Syscall_ReceiveEventFromWindowQueue(&qwWindowID, &stReceivedEvent) == FALSE)
		{

			//20180628
			Syscall_Sleep(1);
			continue;
		}

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
			// 여기에 마우스 이벤트 처리 코드 넣기
			pstMouseEvent = &(stReceivedEvent.stMouseEvent);
			break;

			// 키 이벤트 처리
		case EVENT_KEY_DOWN:
		case EVENT_KEY_UP:
			// 여기에 키보드 이벤트 처리 코드 넣기
			pstKeyEvent = &(stReceivedEvent.stKeyEvent);
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
				Syscall_DeleteWindow(&qwWindowID);
				return 1;
			}
			break;

			// 그 외 정보
		default:
			// 여기에 알 수 없는 이벤트 처리 코드 넣기
			break;
		}
	}

	return 0;
}