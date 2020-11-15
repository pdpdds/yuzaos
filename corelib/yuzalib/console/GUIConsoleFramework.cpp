#include "GUIConsoleFramework.h"
#include <memory.h>
#include <skyoswindow.h>
#include <systemcall_impl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <yuzaapi.h>
#include "GUIConsole.h"

#define FONT_ENGLISHWIDTH   8
#define FONT_ENGLISHHEIGHT  16

// 한글 폰트의 너비와 길이
#define FONT_HANGULWIDTH   16
#define FONT_HANGULHEIGHT  16


GUIConsole* pConsole = 0;
QWORD g_qwWindowID = 0;

DWORD WINAPI GUIConsoleProc(LPVOID parameter)
{
	Syscall_RegisterWindowId(&g_qwWindowID);
	CONSOLE_START_STRUCT* console = (CONSOLE_START_STRUCT*)(parameter);
	return console->entry(console->argc, console->argv);
}

bool GUIConsoleFramework::Run(int argc, char** argv, MAIN_IMPL entry)
{
	bool isGrahpicMode = Syscall_IsGraphicMode();

	if (isGrahpicMode)
	{
		CONSOLE_START_STRUCT consoleStruct;
		consoleStruct.argc = argc;
		consoleStruct.argv = argv;
		consoleStruct.entry = entry;

		return MainLoop(&consoleStruct);
	}
		
	return entry(argc, argv);
}

bool GUIConsoleFramework::MainLoop(CONSOLE_START_STRUCT* args)
{
	QWORD qwWindowID = WINDOW_INVALIDID;
	int iWindowWidth, iWindowHeight;
	EVENT stReceivedEvent;
	KEYEVENT* pstKeyEvent;
	WINDOWEVENT* windowEvent;
	RECT stScreenArea;
	KEYDATA stKeyData;


	Syscall_GetScreenArea(&stScreenArea);

	// 윈도우의 크기 설정, 화면 버퍼에 들어가는 문자의 최대 너비와 높이를 고려해서 계산
	iWindowWidth = CONSOLE_WIDTH * FONT_ENGLISHWIDTH + 4;
	iWindowHeight = CONSOLE_HEIGHT * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT + 2;

	// 윈도우 생성 함수 호출, 화면 가운데에 생성
	RECT rect;
	rect.left = (stScreenArea.right - iWindowWidth) / 2;
	rect.top = (stScreenArea.bottom - iWindowHeight) / 2;
	rect.right = rect.left + iWindowWidth;
	rect.bottom = rect.top + iWindowHeight;

	bool result = Syscall_CreateWindow(&rect, args->argv[0], WINDOW_FLAGS_DEFAULT | WINDOW_FLAGS_RESIZABLE, &qwWindowID);
	// 윈도우를 생성하지 못했으면 실패
	if (qwWindowID == WINDOW_INVALIDID)
	{
		return 0;
	}

	g_qwWindowID = qwWindowID;

	pConsole = new GUIConsole();
	pConsole->Initialize(0, 0);
	pConsole->SetWindowId(qwWindowID);

	Syscall_CreateThread(GUIConsoleProc, args->argv[0], args, 16, 0);
	CONSOLE_START_STRUCT* console = (CONSOLE_START_STRUCT*)args;
	// 이전 화면 버퍼를 초기화
	CHARACTER* vstPreviousScreenBuffer = new CHARACTER[CONSOLE_WIDTH * CONSOLE_HEIGHT];
	memset(vstPreviousScreenBuffer, (char)0xFF, CONSOLE_WIDTH * CONSOLE_HEIGHT * sizeof(CHARACTER));

	//--------------------------------------------------------------------------
	// GUI 태스크의 이벤트 처리 루프
	//--------------------------------------------------------------------------
	while (1)
	{
		// 화면 버퍼의 변경된 내용을 윈도우에 업데이트
		pConsole->ProcessConsoleBuffer(qwWindowID, vstPreviousScreenBuffer);

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
			// 키 이벤트 처리
		case EVENT_KEY_DOWN:
		case EVENT_KEY_UP:
			// 여기에 키보드 이벤트 처리 코드 넣기
			pstKeyEvent = &(stReceivedEvent.stKeyEvent);
			stKeyData.bASCIICode = pstKeyEvent->bASCIICode;
			stKeyData.bFlags = pstKeyEvent->bFlags;
			stKeyData.bScanCode = pstKeyEvent->bScanCode;

			
			// 키를 그래픽 모드용 키 큐로 삽입하여 셸 태스크의 입력으로 전달
			pConsole->PutKeyToGUIKeyQueue(&stKeyData);		

			break;
		case EVENT_CONSOLE_KEY:
		{
			pstKeyEvent = &(stReceivedEvent.stKeyEvent);
			if (pstKeyEvent->bFlags & KEY_FLAGS_DOWN)
			{
				//int iNextPrintOffset = pConsole->ConsolePrintString((char*)windowEvent->stringEvent);
				//if (strlen((char*)windowEvent->stringEvent) == 0)
					//break;
				if(pstKeyEvent->bASCIICode != '\n')
					pConsole->ProcessKey(pstKeyEvent->bASCIICode);
				//pConsole->SetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
			}
			break;
		}

		case EVENT_CONSOLE_PRINT:
		{
			windowEvent = &(stReceivedEvent.stWindowEvent);

			int iNextPrintOffset = pConsole->ConsolePrintString((char*)windowEvent->stringEvent);
			if (strlen((char*)windowEvent->stringEvent) == 0)
				break;

			pConsole->SetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);

			break;
		}
		case EVENT_CONSOLE_COMMAND_END:
		{
			pConsole->PrintPrompt();

			break;
		}
		// 윈도우 이벤트 처리
		case EVENT_WINDOW_CLOSE:
			// 생성한 셸 태스크가 종료되도록 종료 플래그를 설정하고 종료될 때까지 대기
		   // kSetConsoleShellExitFlag( TRUE );
			//while( kIsTaskExist( qwConsoleShellTaskID ) == TRUE )
		{
			//20180628
			Syscall_Sleep(1);
		}

		// 윈도우를 삭제하고 윈도우 ID를 초기화
		Syscall_DeleteWindow(&qwWindowID);
		qwWindowID = WINDOW_INVALIDID;
		return 0;

		break;

		// 그 외 정보
		default:
			// 여기에 알 수 없는 이벤트 처리 코드 넣기
			break;
		}
	}

	return true;
}