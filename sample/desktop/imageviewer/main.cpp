#include "windef.h"
#include <memory.h>
#include <skyoswindow.h>
#include <systemcall_impl.h>
#include <SkyInputHandler.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "JPEG.h"
#include <gdi32.h>

#define FONT_ENGLISHWIDTH   8
#define FONT_ENGLISHHEIGHT  16

// 한글 폰트의 너비와 길이
#define FONT_HANGULWIDTH   16
#define FONT_HANGULHEIGHT  16

#define FILESYSTEM_MAXFILENAMELENGTH        24

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

void DrawFileName(QWORD qwWindowID, RECT* pstArea, char* pcFileName, int iNameLength);
bool CreateImageViewerWindowAndExecute(QWORD qwMainWindowID, const char* pcFileName);

int main(int argc, char** argv)
{
	QWORD qwWindowID;
	int iWindowWidth, iWindowHeight;
	int iEditBoxWidth;
	RECT stEditBoxArea;
	RECT stButtonArea;
	RECT stScreenArea;
	EVENT stReceivedEvent;
	EVENT stSendEvent;
	char vcFileName[FILESYSTEM_MAXFILENAMELENGTH + 1];
	int iFileNameLength;
	MOUSEEVENT* pstMouseEvent;
	KEYEVENT* pstKeyEvent;
	POINT stScreenXY;
	POINT stClientXY;

	//--------------------------------------------------------------------------
	// 윈도우를 생성
	//--------------------------------------------------------------------------
	// 전체 화면 영역의 크기를 반환
	Syscall_GetScreenArea(&stScreenArea);

	// 윈도우의 크기 설정, 화면 버퍼에 들어가는 문자의 최대 너비와 높이를 고려해서 계산
	iWindowWidth = FONT_ENGLISHWIDTH * FILESYSTEM_MAXFILENAMELENGTH + 165;
	iWindowHeight = 35 + WINDOW_TITLEBAR_HEIGHT + 5;

	// 윈도우 생성 함수 호출, 화면 가운데에 생성
	RECT rect;
	rect.left = (stScreenArea.right - iWindowWidth) / 2;
	rect.top = (stScreenArea.bottom - iWindowHeight) / 2;
	rect.right = rect.left + iWindowWidth;
	rect.bottom = rect.top + iWindowHeight;

	 Syscall_CreateWindow(&rect, "Image Viewer", WINDOW_FLAGS_DEFAULT & ~WINDOW_FLAGS_SHOW, &qwWindowID);
	// 윈도우를 생성하지 못했으면 실패
	if (qwWindowID == WINDOW_INVALIDID)
	{
		return 0;
	}

	// 파일 이름을 입력하는 에디트 박스 영역을 표시
	POINT point;
	point.iX = 5;
	point.iY = WINDOW_TITLEBAR_HEIGHT + 6;

	TEXTCOLOR textColor;
	textColor.textColor = RGB(0, 0, 0);
	textColor.backgroundColor = WINDOW_COLOR_BACKGROUND;
	Syscall_DrawText(&qwWindowID, &point, &textColor, "FILE NAME", 9);
	iEditBoxWidth = FONT_ENGLISHWIDTH * FILESYSTEM_MAXFILENAMELENGTH + 4;
	SetRectangleData(85, WINDOW_TITLEBAR_HEIGHT + 5, 85 + iEditBoxWidth,
		WINDOW_TITLEBAR_HEIGHT + 25, &stEditBoxArea);
	Syscall_DrawRect(&qwWindowID, &stEditBoxArea, RGB(0, 0, 0), FALSE);

	// 파일 이름 버퍼를 비우고 에디트 박스에 빈 파일 이름을 표시
	iFileNameLength = 0;
	memset(vcFileName, 0, sizeof(vcFileName));
	DrawFileName(qwWindowID, &stEditBoxArea, vcFileName, iFileNameLength);

	// 이미지 출력 버튼 영역을 지정
	SetRectangleData(stEditBoxArea.right + 10, stEditBoxArea.top,
		stEditBoxArea.right + 70, stEditBoxArea.bottom, &stButtonArea);
	Syscall_DrawButton(&qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND, "Show",
		RGB(0, 0, 0));

	// 윈도우를 표시
	Syscall_ShowWindow(&qwWindowID, TRUE);

	//--------------------------------------------------------------------------
	// GUI 태스크의 이벤트 처리 루프
	//--------------------------------------------------------------------------
	while (1)
	{
		// 이벤트 큐에서 이벤트를 수신
		if (Syscall_ReceiveEventFromWindowQueue(&qwWindowID, &stReceivedEvent) == FALSE)
		{
			//20180628
			Syscall_Sleep(0);
			continue;
		}

		// 수신된 이벤트를 타입에 따라 나누어 처리
		switch (stReceivedEvent.qwType)
		{
			// 마우스 이벤트 처리
		case EVENT_MOUSE_LBUTTONDOWN:
			pstMouseEvent = &(stReceivedEvent.stMouseEvent);

			// 마우스 왼쪽 버튼이 이미지 출력 버튼 위에서 눌러졌으면 저장된 파일 이름을 
			// 이용하여 이미지를 화면에 표시
			if (IsInRectangle(&stButtonArea, pstMouseEvent->stPoint.iX,
				pstMouseEvent->stPoint.iY) == TRUE)
			{
				// 버튼을 눌린 것으로 표시
				Syscall_DrawButton(&qwWindowID, &stButtonArea, RGB(79, 204, 11), "Show",
					RGB(255, 255, 255));
				// 버튼이 있는 영역만 화면 업데이트
				Syscall_UpdateScreenByWindowArea(&qwWindowID, &(stButtonArea));

				// 이미지 출력 윈도우를 생성하고 이벤트를 처리
				if (CreateImageViewerWindowAndExecute(qwWindowID, vcFileName)
					== FALSE)
				{
					// 윈도우 생성에 실패하면 버튼이 눌려졌다가 떨어지는 효과를 주려고
					// 200ms 대기
					//20180628	
					Syscall_Sleep(200);
				}

				// 버튼을 떨어진 것으로 표시
				Syscall_DrawButton(&qwWindowID, &stButtonArea, WINDOW_COLOR_BACKGROUND,
					"Show", RGB(0, 0, 0));
				// 버튼이 있는 영역만 화면 업데이트
				Syscall_UpdateScreenByWindowArea(&qwWindowID, &(stButtonArea));
			}
			break;

			// 키 이벤트 처리
		case EVENT_KEY_DOWN:
			pstKeyEvent = &(stReceivedEvent.stKeyEvent);

			// 백스페이스(Backspace) 키는 삽입된 문자를 삭제
			if ((pstKeyEvent->bASCIICode == KEY_BACKSPACE) &&
				(iFileNameLength > 0))
			{
				// 버퍼에 삽입된 마지막 문자를 삭제
				vcFileName[iFileNameLength] = '\0';
				iFileNameLength--;

				// 입력된 내용을 에디트 박스에 표시
				DrawFileName(qwWindowID, &stEditBoxArea, vcFileName,
					iFileNameLength);
			}
			// 엔터(Enter) 키는 이미지 출력 버튼이 눌린 것으로 처리
			else if ((pstKeyEvent->bASCIICode == KEY_ENTER) &&
				(iFileNameLength > 0))
			{
				// 버튼의 XY 좌표를 화면 좌표로 변환하여 마우스 이벤트의 좌표로 사용
				stClientXY.iX = stButtonArea.left + 1;
				stClientXY.iY = stButtonArea.top + 1;
				ConvertPointClientToScreen(qwWindowID, &stClientXY, &stScreenXY);

				// 이미지 출력 버튼에 마우스 왼쪽 버튼이 눌린 것처럼 마우스 이벤트를 전송
				SetMouseEvent(qwWindowID, EVENT_MOUSE_LBUTTONDOWN, stScreenXY.iX + 1, stScreenXY.iY + 1, 0, &stSendEvent);
				Syscall_SendEventToWindow(&qwWindowID, &stSendEvent);
			}
			// ESC 키는 윈도우 닫힘 버튼이 눌린 것으로 처리
			else if (pstKeyEvent->bASCIICode == KEY_ESC)
			{
				// 윈도우 닫기 이벤트를 윈도우로 전송
				SetWindowEvent(qwWindowID, EVENT_WINDOW_CLOSE, &stSendEvent);
				Syscall_SendEventToWindow(qwWindowID, &stSendEvent);
			}
			// 그 외 키는 파일 이름 버퍼에 공간이 있는 경우만 버퍼에 삽입
			else if ((pstKeyEvent->bASCIICode <= 128) &&
				(pstKeyEvent->bASCIICode != KEY_BACKSPACE) &&
				(iFileNameLength < FILESYSTEM_MAXFILENAMELENGTH))
			{
				// 입력된 키를 파일 이름 버퍼의 마지막에 삽입
				vcFileName[iFileNameLength] = pstKeyEvent->bASCIICode;
				iFileNameLength++;

				// 입력된 내용을 에디트 박스에 표시
				DrawFileName(qwWindowID, &stEditBoxArea, vcFileName,
					iFileNameLength);
			}
			break;

			// 윈도우 이벤트 처리
		case EVENT_WINDOW_CLOSE:
			if (stReceivedEvent.qwType == EVENT_WINDOW_CLOSE)
			{
				// 윈도우 삭제
				Syscall_DeleteWindow(&qwWindowID);
				return 0;
			}
			break;

			// 그 외 정보
		default:
			// 여기에 알 수 없는 이벤트 처리 코드 넣기
			break;
		}
	}

	return 1;
}

/**
 *  에디트 박스 영역에 문자를 출력
 */
void DrawFileName(QWORD qwWindowID, RECT* pstArea, char* pcFileName,
	int iNameLength)
{
	// 에디트 박스의 배경을 모두 흰색으로 채움
	RECT rect = *pstArea;
	rect.left += 1;
	rect.top += 1;
	rect.right -= 1;
	rect.bottom -= 1;

	Syscall_DrawRect(&qwWindowID, &rect, WINDOW_COLOR_BACKGROUND, TRUE);

	// 파일 이름을 출력
	POINT point{ pstArea->left + 2, pstArea->top + 2 };
	
	TEXTCOLOR textColor;
	textColor.textColor = RGB(0, 0, 0);
	textColor.backgroundColor = WINDOW_COLOR_BACKGROUND;
	Syscall_DrawText(&qwWindowID, &point, &textColor, pcFileName, iNameLength);

	// 파일 이름의 길이가 파일 시스템이 정의한 최대 길이가 아니면 커서를 출력
	if (iNameLength < FILESYSTEM_MAXFILENAMELENGTH)
	{
		point.iX = pstArea->left + 2 + FONT_ENGLISHWIDTH * iNameLength, pstArea->top + 2;
		point.iY = pstArea->top + 2;
	
		textColor.textColor = RGB(0, 0, 0);
		textColor.backgroundColor = WINDOW_COLOR_BACKGROUND;
		Syscall_DrawText(&qwWindowID, &point, &textColor, "_", 1);
	}

	// 에디트 박스 영역만 화면 업데이트
	Syscall_UpdateScreenByWindowArea(&qwWindowID, pstArea);
}


bool CreateImageViewerWindowAndExecute(QWORD qwMainWindowID, const char* pcFileName)
{
	DWORD dwFileSize;
	RECT stScreenArea;
	QWORD qwWindowID;
	BYTE* pbFileBuffer;
	COLOR* pstOutputBuffer;
	FILE* fp;
	JPEG* pstJpeg;
	EVENT stReceivedEvent;
	KEYEVENT* pstKeyEvent;
	bool bExit;

	// 초기화
	fp = NULL;
	pbFileBuffer = NULL;
	pstOutputBuffer = NULL;
	qwWindowID = WINDOW_INVALIDID;

	//--------------------------------------------------------------------------
	//  파일을 읽은 후 이미지 디코딩
	//--------------------------------------------------------------------------
	// 파일 읽기
	fp = fopen(pcFileName, "rb");
	if (fp == NULL)
	{
		printf("[ImageViewer] %s file open fail\n", pcFileName);
		return FALSE;
	}

	fseek(fp, 0, SEEK_END);
	dwFileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (dwFileSize == 0)
	{
		printf("[ImageViewer] %s file doesn't exist or size is zero\n", pcFileName);
		return FALSE;
	}

	// 메모리를 파일 크기만큼 할당하고 JPEG 자료구조를 할당
	pbFileBuffer = (BYTE*) new char[dwFileSize];
	pstJpeg = (JPEG*) new char[sizeof(JPEG)];
	if ((pbFileBuffer == NULL) || (pstJpeg == NULL))
	{
		printf("[ImageViewer] Buffer allocation fail\n");
		delete (pbFileBuffer);
		delete (pstJpeg);
		fclose(fp);
		return FALSE;
	}

	// 파일을 읽은 후 JPEG 파일 포맷인지 확인
	if ((fread(pbFileBuffer, 1, dwFileSize, fp) != dwFileSize) ||
		(kJPEGInit(pstJpeg, pbFileBuffer, dwFileSize) == FALSE))
	{
		printf("[ImageViewer] Read fail or file is not JPEG format\n");
		delete (pbFileBuffer);
		delete (pstJpeg);
		fclose(fp);
		return FALSE;
	}

	// 디코드 결과 출력용 버퍼를 생성
	pstOutputBuffer = (COLOR*)new char[pstJpeg->width * pstJpeg->height * sizeof(COLOR)];
	// 디코드를 수행한 뒤 정상적으로 처리되었다면 윈도우를 생성
	if ((pstOutputBuffer != NULL) &&
		(kJPEGDecode(pstJpeg, pstOutputBuffer) == TRUE))
	{
		// 전체 화면 영역의 크기를 반환
		Syscall_GetScreenArea(&stScreenArea);
		// 윈도우를 생성, 이미지의 크기와 제목 표시줄의 크기를 고려
		RECT rect;
		rect.left = (stScreenArea.right - pstJpeg->width) / 2;
		rect.top = (stScreenArea.bottom - pstJpeg->height) / 2;
		rect.right = rect.left + pstJpeg->width;
		rect.bottom = rect.top + pstJpeg->height + WINDOW_TITLEBAR_HEIGHT;
		Syscall_CreateWindow(&rect, pcFileName, WINDOW_FLAGS_DEFAULT & ~WINDOW_FLAGS_SHOW | WINDOW_FLAGS_RESIZABLE, &qwWindowID);
	}

	// 윈도우 생성에 실패하거나 출력 버퍼 할당 또는 디코딩에 실패하면 종료
	if ((qwWindowID == WINDOW_INVALIDID) || (pstOutputBuffer == NULL))
	{
		printf("[ImageViewer] Window create fail or output buffer allocation fail\n");
		delete (pbFileBuffer);
		delete (pstJpeg);
		delete (pstOutputBuffer);
		Syscall_DeleteWindow(&qwWindowID);
		return FALSE;
	}

	RECT rect;
	rect.left = 0;
	rect.top = WINDOW_TITLEBAR_HEIGHT;
	rect.right = pstJpeg->width;
	rect.bottom = rect.top + pstJpeg->height;
	Syscall_DrawWindow(&qwWindowID, pstOutputBuffer , &rect);

	// 파일 버퍼를 해제하고 윈도우를 화면에 표시
	delete (pbFileBuffer);
	Syscall_ShowWindow(&qwWindowID, TRUE);

	//--------------------------------------------------------------------------
	//  ESC 키와 윈도우 닫기 버튼을 처리하는 간단한 이벤트 루프
	//--------------------------------------------------------------------------
	// 정상적으로 윈도우를 생성하여 표시했으면 파일 이름 입력 윈도우는 숨김
	Syscall_ShowWindow(&qwMainWindowID, FALSE);

	bExit = FALSE;
	while (bExit == FALSE)
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
			// 키 이벤트 처리
		case EVENT_KEY_DOWN:
			pstKeyEvent = &(stReceivedEvent.stKeyEvent);
			// ESC 키가 눌리면 그림을 표시하는 윈도우를 삭제하고 파일 이름 입력 윈도우를
			// 표시한 뒤 종료
			if (pstKeyEvent->bASCIICode == KEY_ESC)
			{
				Syscall_DeleteWindow(&qwWindowID);
				Syscall_ShowWindow(qwMainWindowID, TRUE);
				bExit = TRUE;
			}
			break;

			// 윈도우 이벤트 처리
			// 윈도우 크기 변경 이벤트를 처리
		case EVENT_WINDOW_RESIZE:
			// 변경된 윈도우에 디코딩된 이미지를 전송
			rect.left = 0;
			rect.top = WINDOW_TITLEBAR_HEIGHT;
			rect.right = pstJpeg->width;
			rect.bottom = rect.top + pstJpeg->height;
			Syscall_DrawWindow(&qwWindowID, pstOutputBuffer, &rect);
			// 윈도우를 한번 더 표시하여 윈도우 내부에 전송된 이미지를 화면에 업데이트
			Syscall_ShowWindow(&qwWindowID, TRUE);
			break;

			// 윈도우 닫기 이벤트를 처리
		case EVENT_WINDOW_CLOSE:
			// 닫기 버튼이 눌리면 이미지 출력 윈도우를 삭제하고 파일 이름 입력 윈도우를
			// 표시한 뒤 종료
			if (stReceivedEvent.qwType == EVENT_WINDOW_CLOSE)
			{
				Syscall_DeleteWindow(&qwWindowID);
				Syscall_ShowWindow(&qwMainWindowID, TRUE);
				bExit = TRUE;
			}
			break;

			// 그 외 정보
		default:
			// 여기에 알 수 없는 이벤트 처리 코드 넣기
			break;
		}
	}

	// JPEG 이미지 파일 디코딩에 사용했던 버퍼를 반환
	delete (pstJpeg);
	delete (pstOutputBuffer);

	return TRUE;
}