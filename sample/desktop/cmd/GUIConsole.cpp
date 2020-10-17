#include "GUIConsole.h"
#include "memory.h"
#include "Mouse.h"
#include <stdio.h>
#include "stringdef.h"
#include "ConsoleShell.h"
#include <include/systemcall_impl.h>
#include <UserLibrary.h>

// 커맨드 테이블 정의
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
	{ "help", "Show Help", kHelp },
	{ "cls", "Clear Screen", kCls },
	{ "totalram", "Show Total RAM Size", kShowTotalRAMSize },
	{ "shutdown", "Shutdown And Reboot OS", kShutdown },
};

#define FONT_ENGLISHWIDTH   8
#define FONT_ENGLISHHEIGHT  16

// 한글 폰트의 너비와 길이
#define FONT_HANGULWIDTH   16
#define FONT_HANGULHEIGHT  16

GUIConsole::GUIConsole()
{
	
}


GUIConsole::~GUIConsole()
{
}

SHELLCOMMANDENTRY* GUIConsole::GetCommandArray()
{
	return gs_vstCommandTable;
}

bool GUIConsole::Initialize(int iX, int iY)
{
	memset(&m_ConsoleManager, 0, sizeof(CONSOLEMANAGER));

	memset(&m_vstScreenBuffer, 0, sizeof(m_vstScreenBuffer));
	memset(&m_vstKeyQueueBuffer, 0, sizeof(m_vstKeyQueueBuffer));

	m_ConsoleManager.pstScreenBuffer = (CHARACTER*)m_vstScreenBuffer;

	kInitializeQueue(&(m_ConsoleManager.stKeyQueueForGUI), m_vstKeyQueueBuffer,
		CONSOLE_GUIKEYQUEUE_MAXCOUNT, sizeof(KEYDATA));

	// 커서 위치 설정
	SetCursor(iX, iY);
	
	m_commandCount = sizeof(gs_vstCommandTable) / sizeof(SHELLCOMMANDENTRY);

	m_ConsoleManager.mutex = Syscall_CreateMutex("GUIConsole");

	return true;
}

/**
*  커서의 위치를 설정
*      문자를 출력할 위치도 같이 설정
*/
void GUIConsole::SetCursor(int iX, int iY)
{
	int iLinearValue;
	int i;

	// 커서의 위치를 계산
	iLinearValue = iY * CONSOLE_WIDTH + iX;


	// 이전에 커서가 있던 위치가 그대로 커서로 남아있으면 커서를 지움
	for (i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
	{
		// 커서가 있으면 삭제
		if ((m_ConsoleManager.pstScreenBuffer[i].bCharacter == '_') &&
			(m_ConsoleManager.pstScreenBuffer[i].bAttribute == 0x00))
		{
			m_ConsoleManager.pstScreenBuffer[i].bCharacter = ' ';
			m_ConsoleManager.pstScreenBuffer[i].bAttribute =
				CONSOLE_DEFAULTTEXTCOLOR;
			break;
		}
	}

	// 새로운 위치에 커서를 출력
	m_ConsoleManager.pstScreenBuffer[iLinearValue].bCharacter = '_';
	m_ConsoleManager.pstScreenBuffer[iLinearValue].bAttribute = 0x00;

	// 문자를 출력할 위치 업데이트
	m_ConsoleManager.iCurrentPrintOffset = iLinearValue;
}

/**
*  현재 커서의 위치를 반환
*/
void GUIConsole::GetCursor(int *piX, int *piY)
{
	*piX = m_ConsoleManager.iCurrentPrintOffset % CONSOLE_WIDTH;
	*piY = m_ConsoleManager.iCurrentPrintOffset / CONSOLE_WIDTH;
}

void GUIConsole::PrintPrompt()
{
	// 프롬프트 출력 및 커맨드 버퍼 초기화
	char* newPath = ConsoleManager::MakePathName();
	Printf(newPath);
}

/**
*  전체 화면을 삭제
*/
void GUIConsole::ClearScreen(void)
{
	CHARACTER* pstScreen;
	int i;

	// 화면 버퍼를 설정
	pstScreen = m_ConsoleManager.pstScreenBuffer;

	// 화면 전체를 공백으로 채우고, 커서의 위치를 0, 0으로 옮김
	for (i = 0; i < CONSOLE_WIDTH * CONSOLE_HEIGHT; i++)
	{
		pstScreen[i].bCharacter = ' ';
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}

	// 커서를 화면 상단으로 이동
	SetCursor(0, 0);
}

/**
*  문자열의 순서를 뒤집음
*/
void GUIConsole::ReverseString(char* pcBuffer)
{
	int iLength;
	int i;
	char cTemp;


	// 문자열의 가운데를 중심으로 좌/우를 바꿔서 순서를 뒤집음
	iLength = strlen(pcBuffer);
	for (i = 0; i < iLength / 2; i++)
	{
		cTemp = pcBuffer[i];
		pcBuffer[i] = pcBuffer[iLength - 1 - i];
		pcBuffer[iLength - 1 - i] = cTemp;
	}
}

/**
*  16진수 값을 문자열로 변환
*/
QWORD GUIConsole::HexToString(QWORD qwValue, char* pcBuffer)
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
			pcBuffer[i] = (char)('A' + (qwCurrentValue - 10));
		}
		else
		{
			pcBuffer[i] = (char)('0' + qwCurrentValue);
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
int GUIConsole::DecimalToString(QWORD lValue, char* pcBuffer)
{
	long i;

	// 0이 들어오면 바로 처리
	if (lValue == 0)
	{
		pcBuffer[0] = '0';
		pcBuffer[1] = '\0';
		return 1;
	}

	// 만약 음수이면 출력 버퍼에 '-'를 추가하고 양수로 변환
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

	// 버퍼에 1의 자리부터 10, 100, 1000 ...의 자리 순서로 숫자 삽입
	for (; lValue > 0; i++)
	{
		pcBuffer[i] = '0' + lValue % 10;
		lValue = lValue / 10;
	}
	pcBuffer[i] = '\0';

	// 버퍼에 들어있는 문자열을 뒤집어서 ... 1000, 100, 10, 1의 자리 순서로 변경
	if (pcBuffer[0] == '-')
	{
		// 음수인 경우는 부호를 제외하고 문자열을 뒤집음
		ReverseString(&(pcBuffer[1]));
	}
	else
	{
		ReverseString(pcBuffer);
	}

	return i;
}

QWORD GUIConsole::IToA(QWORD lValue, char* pcBuffer, int iRadix)
{
	QWORD iReturn;

	switch (iRadix)
	{
		// 16진수
	case 16:
		iReturn = HexToString(lValue, pcBuffer);
		break;

		// 10진수 또는 기타
	case 10:
	default:
		iReturn = DecimalToString(lValue, pcBuffer);
		break;
	}

	return iReturn;
}

/**
*  vsprintf() 함수의 내부 구현
*      버퍼에 포맷 문자열에 따라 데이터를 복사
*/
int GUIConsole::VSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap)
{
	QWORD i, k;
	int iBufferIndex = 0;
	int iFormatLength, iCopyLength;
	char* pcCopyString;
	QWORD qwValue;
	int iValue;
	double dValue;

	// 포맷 문자열의 길이를 읽어서 문자열의 길이만큼 데이터를 출력 버퍼에 출력
	iFormatLength = strlen(pcFormatString);
	for (i = 0; i < iFormatLength; i++)
	{
		// %로 시작하면 데이터 타입 문자로 처리
		if (pcFormatString[i] == '%')
		{
			// % 다음의 문자로 이동
			i++;
			switch (pcFormatString[i])
			{
				// 문자열 출력  
			case 's':
				// 가변 인자에 들어있는 파라미터를 문자열 타입으로 변환
				pcCopyString = (char*)(va_arg(ap, char*));
				iCopyLength = strlen(pcCopyString);
				// 문자열의 길이만큼을 출력 버퍼로 복사하고 출력한 길이만큼 
				// 버퍼의 인덱스를 이동
				memcpy(pcBuffer + iBufferIndex, pcCopyString, iCopyLength);
				iBufferIndex += iCopyLength;
				break;

				// 문자 출력
			case 'c':
				// 가변 인자에 들어있는 파라미터를 문자 타입으로 변환하여 
				// 출력 버퍼에 복사하고 버퍼의 인덱스를 1만큼 이동
				pcBuffer[iBufferIndex] = (char)(va_arg(ap, int));
				iBufferIndex++;
				break;

				// 정수 출력
			case 'd':
			case 'i':
				// 가변 인자에 들어있는 파라미터를 정수 타입으로 변환하여
				// 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
				iValue = (int)(va_arg(ap, int));
				iBufferIndex += (long)IToA(iValue, pcBuffer + iBufferIndex, 10);
				break;

				// 4바이트 Hex 출력
			case 'x':
			case 'X':
				// 가변 인자에 들어있는 파라미터를 DWORD 타입으로 변환하여
				// 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
				qwValue = (DWORD)(va_arg(ap, DWORD)) & 0xFFFFFFFF;
				iBufferIndex += (long)IToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;

				// 8바이트 Hex 출력
			case 'q':
			case 'Q':
			case 'p':
				// 가변 인자에 들어있는 파라미터를 QWORD 타입으로 변환하여
				// 출력 버퍼에 복사하고 출력한 길이만큼 버퍼의 인덱스를 이동
				qwValue = (QWORD)(va_arg(ap, QWORD));
				iBufferIndex += (long)IToA(qwValue, pcBuffer + iBufferIndex, 16);
				break;

				// 소수점 둘째 자리까지 실수를 출력
			case 'f':
				dValue = (double)(va_arg(ap, double));
				// 셋째 자리에서 반올림 처리
				dValue += 0.005;
				// 소수점 둘째 자리부터 차례로 저장하여 버퍼를 뒤집음
				pcBuffer[iBufferIndex] = '0' + (QWORD)(dValue * 100) % 10;
				pcBuffer[iBufferIndex + 1] = '0' + (QWORD)(dValue * 10) % 10;
				pcBuffer[iBufferIndex + 2] = '.';
				for (k = 0; ; k++)
				{
					// 정수 부분이 0이면 종료
					if (((QWORD)dValue == 0) && (k != 0))
					{
						break;
					}
					pcBuffer[iBufferIndex + 3 + k] = '0' + ((QWORD)dValue % 10);
					dValue = dValue / 10;
				}
				pcBuffer[iBufferIndex + 3 + k] = '\0';
				// 값이 저장된 길이만큼 뒤집고 길이를 증가시킴
				ReverseString(pcBuffer + iBufferIndex);
				iBufferIndex += (int)(3 + k);
				break;

				// 위에 해당하지 않으면 문자를 그대로 출력하고 버퍼의 인덱스를
				// 1만큼 이동
			default:
				pcBuffer[iBufferIndex] = pcFormatString[i];
				iBufferIndex++;
				break;
			}
		}
		// 일반 문자열 처리
		else
		{
			// 문자를 그대로 출력하고 버퍼의 인덱스를 1만큼 이동
			pcBuffer[iBufferIndex] = pcFormatString[i];
			iBufferIndex++;
		}
	}

	// NULL을 추가하여 완전한 문자열로 만들고 출력한 문자의 길이를 반환
	pcBuffer[iBufferIndex] = '\0';
	return iBufferIndex;
}

void GUIConsole::Printf(const char* pcFormatString, ...)
{
	va_list ap;
	char vcBuffer[1024];
	int iNextPrintOffset;

	// 가변 인자 리스트를 사용해서 vsprintf()로 처리
	va_start(ap, pcFormatString);
	VSPrintf(vcBuffer, pcFormatString, ap);
	va_end(ap);

	// 포맷 문자열을 화면에 출력
	iNextPrintOffset = ConsolePrintString(vcBuffer);

	// 커서의 위치를 업데이트
	SetCursor(iNextPrintOffset % CONSOLE_WIDTH, iNextPrintOffset / CONSOLE_WIDTH);
}

/**
*  문자열을 X, Y 위치에 출력
*/
void GUIConsole::PrintStringXY(int iX, int iY, const char* pcString)
{
	CHARACTER* pstScreen;
	int i;

	// 화면 버퍼를 설정
	pstScreen = m_ConsoleManager.pstScreenBuffer;

	// 현재 출력할 위치를 계산
	pstScreen += (iY * CONSOLE_WIDTH) + iX;
	// 문자열의 길이만큼 루프를 돌면서 문자와 속성을 저장
	for (i = 0; pcString[i] != 0; i++)
	{
		pstScreen[i].bCharacter = pcString[i];
		pstScreen[i].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
	}
}

/**
*  getch() 함수의 구현
*/
BYTE GUIConsole::GetCh(void)
{
	KEYDATA stData;

	// 키가 눌러질 때까지 대기함
	while (1)
	{

		while (GetKeyFromGUIKeyQueue(&stData) == FALSE)
		{
			// 그래픽 모드에서 동작하는 중에 셸 태스크를 종료해야될 경우 루프를 종료
			if (m_ConsoleManager.bExit == TRUE)
			{
				return 0xFF;
			}
			//kSchedule();

			Syscall_Sleep(1);
		}

		// 키가 눌렸다는 데이터가 수신되면 ASCII 코드를 반환
		if (stData.bFlags & KEY_FLAGS_DOWN)
		{
			return stData.bASCIICode;
		}
	}
}

/**
*  콘솔을 관리하는 자료구조를 반환
*/
CONSOLEMANAGER* GUIConsole::GetConsoleManager(void)
{
	return &m_ConsoleManager;
}

/**
*  그래픽 모드용 키 큐에서 키 데이터를 제거
*/
bool GUIConsole::GetKeyFromGUIKeyQueue(KEYDATA* pstData)
{
	bool bResult;

	// 큐에 데이터가 없으면 실패
	if (kIsQueueEmpty(&(m_ConsoleManager.stKeyQueueForGUI)) == TRUE)
	{
		return FALSE;
	}
	
	Syscall_LockMutex(m_ConsoleManager.mutex);
	bResult = kGetQueue(&(m_ConsoleManager.stKeyQueueForGUI), pstData);
	Syscall_UnlockMutex(m_ConsoleManager.mutex);
	
	return bResult;
}

/**
*  그래픽 모드용 키 큐에 데이터를 삽입
*/
bool GUIConsole::PutKeyToGUIKeyQueue(KEYDATA* pstData)
{
	bool bResult;

	// 큐에 데이터가 가득 찼으면 실패
	if (kIsQueueFull(&(m_ConsoleManager.stKeyQueueForGUI)) == TRUE)
	{
		return FALSE;
	}

	
	// 큐에 데이터를 삽입
	Syscall_LockMutex(m_ConsoleManager.mutex);
	bResult = kPutQueue(&(m_ConsoleManager.stKeyQueueForGUI), pstData);
	Syscall_UnlockMutex(m_ConsoleManager.mutex);
	
	return bResult;
}

/**
*  콘솔 셸 태스크 종료 플래그를 설정
*/
void GUIConsole::SetConsoleShellExitFlag(bool bFlag)
{
	m_ConsoleManager.bExit = bFlag;
}

//
//  화면 버퍼의 변경된 내용을 GUI 콘솔 셸 윈도우 화면에 업데이트
//
void GUIConsole::ProcessConsoleBuffer(QWORD qwWindowID, CHARACTER* vstPreviousScreenBuffer)
{
	int i;
	int j;
	CONSOLEMANAGER* pstManager;
	CHARACTER* pstScreenBuffer;
	CHARACTER* pstPreviousScreenBuffer;
	RECT stLineArea;
	bool bChanged;
	static DWORD dwLastTickCount = 0;
	bool bFullRedraw;

	// 콘솔을 관리하는 자료구조를 반환 받아 화면 버퍼의 어드레스를 저장하고 
	// 이전 화면 버퍼의 어드레스도 저장
	pstManager = GetConsoleManager();
	pstScreenBuffer = pstManager->pstScreenBuffer;
	pstPreviousScreenBuffer = vstPreviousScreenBuffer;

	// 화면을 전체를 업데이트 한 지 5초가 지났으면 무조건 화면 전체를 다시 그림
	if (Syscall_GetTickCount() - dwLastTickCount > 5000)
	{
		dwLastTickCount = Syscall_GetTickCount();
		bFullRedraw = TRUE;
	}
	else
	{
		bFullRedraw = FALSE;
	}
	//bFullRedraw = TRUE;

	// 화면 버퍼의 높이만큼 반복
	for (j = 0; j < CONSOLE_HEIGHT; j++)
	{
		// 처음에는 변경되지 않은 것으로 플래그 설정
		bChanged = FALSE;

		// 현재 라인에 변화가 있는지 비교하여 변경 여부 플래그를 처리
		for (i = 0; i < CONSOLE_WIDTH; i++)
		{
			// 문자를 비교하여 다르거나 전체를 새로 그려야 하면 이전 화면 버퍼에
			// 업데이트하고 변경 여부 플래그를 설정
			if ((pstScreenBuffer->bCharacter != pstPreviousScreenBuffer->bCharacter) ||
				(bFullRedraw == TRUE))
			{
				// 문자를 화면에 출력
				POINT loc;
				loc.iX = i * FONT_ENGLISHWIDTH + 2;
				loc.iY = j * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT;

				TEXTCOLOR textColor;
				textColor.textColor = RGB(0, 255, 0);
				textColor.backgroundColor = RGB(0, 0, 0);
				Syscall_DrawText(&qwWindowID, &loc, &textColor, (const char*)&(pstScreenBuffer->bCharacter), 1);

				// 이전 화면 버퍼로 값을 옮겨 놓고 현재 라인에 이전과
				// 다른 데이터가 있음을 표시
				memcpy(pstPreviousScreenBuffer, pstScreenBuffer, sizeof(CHARACTER));
				bChanged = TRUE;
			}

			pstScreenBuffer++;
			pstPreviousScreenBuffer++;
		}

		// 현재 라인에서 변경된 데이터가 있다면 현재 라인만 화면에 업데이트
		if (bChanged == TRUE)
		{
			// 현재 라인의 영역을 계산
			SetRectangleData(2, j * FONT_ENGLISHHEIGHT + WINDOW_TITLEBAR_HEIGHT,
				5 + FONT_ENGLISHWIDTH * CONSOLE_WIDTH, (j + 1) * FONT_ENGLISHHEIGHT +
				WINDOW_TITLEBAR_HEIGHT - 1, &stLineArea);
			// 윈도우의 일부만 화면 업데이트
			Syscall_UpdateScreenByWindowArea(&qwWindowID, &stLineArea);
		}
	}
}

/**
*  \n, \t와 같은 문자가 포함된 문자열을 출력한 후, 화면상의 다음 출력할 위치를
*  반환
*/
int GUIConsole::ConsolePrintString(const char* pcBuffer)
{
	CHARACTER* pstScreen;
	int i, j;
	int iLength;
	int iPrintOffset;

	// 화면 버퍼를 설정
	pstScreen = m_ConsoleManager.pstScreenBuffer;

	// 문자열을 출력할 위치를 저장
	iPrintOffset = m_ConsoleManager.iCurrentPrintOffset;

	// 문자열의 길이만큼 화면에 출력
	iLength = strlen(pcBuffer);
	for (i = 0; i < iLength; i++)
	{
		// 개행 처리
		if (pcBuffer[i] == '\n')
		{
			// 출력할 위치를 80의 배수 컬럼으로 옮김
			// 현재 라인의 남은 문자열의 수만큼 더해서 다음 라인으로 위치시킴
			iPrintOffset += (CONSOLE_WIDTH - (iPrintOffset % CONSOLE_WIDTH));
		}
		// 탭 처리
		else if (pcBuffer[i] == '\t')
		{
			// 출력할 위치를 8의 배수 컬럼으로 옮김
			iPrintOffset += (8 - (iPrintOffset % 8));
		}
		// 일반 문자열 출력
		else
		{
			// 비디오 메모리에 문자와 속성을 설정하여 문자를 출력하고
			// 출력할 위치를 다음으로 이동
			pstScreen[iPrintOffset].bCharacter = pcBuffer[i];
			pstScreen[iPrintOffset].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
			iPrintOffset++;
		}

		// 출력할 위치가 화면의 최댓값(80 * 25)을 벗어났으면 스크롤 처리
		if (iPrintOffset >= (CONSOLE_HEIGHT * CONSOLE_WIDTH))
		{
			// 가장 윗줄을 제외한 나머지를 한 줄 위로 복사
			memcpy(pstScreen, pstScreen + CONSOLE_WIDTH,
				(CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH * sizeof(CHARACTER));

			// 가장 마지막 라인은 공백으로 채움
			for (j = (CONSOLE_HEIGHT - 1) * (CONSOLE_WIDTH);
				j < (CONSOLE_HEIGHT * CONSOLE_WIDTH); j++)
			{
				// 공백 출력
				pstScreen[j].bCharacter = ' ';
				pstScreen[j].bAttribute = CONSOLE_DEFAULTTEXTCOLOR;
			}

			// 출력할 위치를 가장 아래쪽 라인의 처음으로 설정
			iPrintOffset = (CONSOLE_HEIGHT - 1) * CONSOLE_WIDTH;
		}
	}
	return iPrintOffset;
}

/*
*  커맨드 버퍼에 있는 커맨드를 비교하여 해당 커맨드를 처리하는 함수를 수행
*/
void GUIConsole::ExecuteCommand(const char* pcCommandBuffer)
{
	manager.RunCommand(pcCommandBuffer);

	EVENT stEvent;
	stEvent.qwType = EVENT_CONSOLE_COMMAND_END;

	stEvent.stWindowEvent.qwWindowID = m_windowId;
	Syscall_SendEventToWindow(&m_windowId, &stEvent);

	/*int i, iSpaceIndex;
	int iCommandBufferLength, iCommandLength;	

	// 공백으로 구분된 커맨드를 추출
	iCommandBufferLength = strlen(pcCommandBuffer);
	for (iSpaceIndex = 0; iSpaceIndex < iCommandBufferLength; iSpaceIndex++)
	{
		if (pcCommandBuffer[iSpaceIndex] == ' ')
		{
			break;
		}
	}
	
	for (i = 0; i < m_commandCount; i++)
	{
		iCommandLength = strlen(gs_vstCommandTable[i].pcCommand);
		// 커맨드의 길이와 내용이 완전히 일치하는지 검사
		if ((iCommandLength == iSpaceIndex) &&
			(memcmp(gs_vstCommandTable[i].pcCommand, pcCommandBuffer,
				iSpaceIndex) == 0))
		{
			gs_vstCommandTable[i].pfFunction(this, pcCommandBuffer + iSpaceIndex + 1);
			break;
		}
	}

	// 리스트에서 찾을 수 없다면 에러 출력
	if (i >= m_commandCount)
	{
		int result = Syscall_CreateProcess(pcCommandBuffer, nullptr, 16);

		if(result == 0)
			Printf("'%s' is not found.\n", pcCommandBuffer);
		else
			Printf("result %d\n", result);
	}*/
}

