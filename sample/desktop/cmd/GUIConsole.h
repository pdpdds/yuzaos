#pragma once
#include "windef.h"
#include "va_list.h"
#include "InputQueue.h"
#include "Keyboard.h"
#include "ConsoleShell.h"
#include <ConsoleManager.h>

// 비디오 메모리의 속성(Attribute) 값 설정
#define CONSOLE_BACKGROUND_BLACK            0x00
#define CONSOLE_BACKGROUND_BLUE             0x10
#define CONSOLE_BACKGROUND_GREEN            0x20
#define CONSOLE_BACKGROUND_CYAN             0x30
#define CONSOLE_BACKGROUND_RED              0x40
#define CONSOLE_BACKGROUND_MAGENTA          0x50
#define CONSOLE_BACKGROUND_BROWN            0x60
#define CONSOLE_BACKGROUND_WHITE            0x70
#define CONSOLE_BACKGROUND_BLINK            0x80
#define CONSOLE_FOREGROUND_DARKBLACK        0x00
#define CONSOLE_FOREGROUND_DARKBLUE         0x01
#define CONSOLE_FOREGROUND_DARKGREEN        0x02
#define CONSOLE_FOREGROUND_DARKCYAN         0x03
#define CONSOLE_FOREGROUND_DARKRED          0x04
#define CONSOLE_FOREGROUND_DARKMAGENTA      0x05
#define CONSOLE_FOREGROUND_DARKBROWN        0x06
#define CONSOLE_FOREGROUND_DARKWHITE        0x07
#define CONSOLE_FOREGROUND_BRIGHTBLACK      0x08
#define CONSOLE_FOREGROUND_BRIGHTBLUE       0x09
#define CONSOLE_FOREGROUND_BRIGHTGREEN      0x0A
#define CONSOLE_FOREGROUND_BRIGHTCYAN       0x0B
#define CONSOLE_FOREGROUND_BRIGHTRED        0x0C
#define CONSOLE_FOREGROUND_BRIGHTMAGENTA    0x0D
#define CONSOLE_FOREGROUND_BRIGHTYELLOW     0x0E
#define CONSOLE_FOREGROUND_BRIGHTWHITE      0x0F
// 기본 문자 색상
#define CONSOLE_DEFAULTTEXTCOLOR            ( CONSOLE_BACKGROUND_BLACK | \
        CONSOLE_FOREGROUND_BRIGHTGREEN )

// 콘솔의 너비(Width)와 높이(Height),그리고 비디오 메모리의 시작 어드레스 설정
#define CONSOLE_WIDTH                       80
#define CONSOLE_HEIGHT                      25

// 그래픽 모드에서 사용하는 키 큐에 저장할 수 있는 최대 개수
#define CONSOLE_GUIKEYQUEUE_MAXCOUNT        100     

typedef struct kCharactorStruct
{
	BYTE bCharacter;
	BYTE bAttribute;
} CHARACTER;


#pragma pack( push, 1 )

// 콘솔에 대한 정보를 저장하는 자료구조
typedef struct kConsoleManagerStruct
{
	// 현재 출력할 곳의 오프셋
	int iCurrentPrintOffset;

	DWORD mutex;

	// 출력할 화면 버퍼의 어드레스
	CHARACTER* pstScreenBuffer;

	// 그래픽 모드에서 사용할 키 큐와 뮤텍스
	QUEUE stKeyQueueForGUI;
	//    MUTEX stLock;

	// 셸 태스크를 종료할지 여부
	volatile bool bExit;
} CONSOLEMANAGER;

#pragma pack( pop )

class GUIConsole
{
public:
	GUIConsole();
	virtual ~GUIConsole();

	bool Initialize(int iX, int iY);

	void SetWindowId(QWORD windowId) { m_windowId = windowId; }
	QWORD GetWindowId() { return m_windowId;}
	void ProcessConsoleBuffer(QWORD qwWindowID, CHARACTER* vstPreviousScreenBuffer);
	void ExecuteCommand(const char* pcCommandBuffer);

	SHELLCOMMANDENTRY* GetCommandArray();
	int GetCommandCount() { return m_commandCount; }
	void PrintPrompt();

	void SetCursor(int iX, int iY);
	void GetCursor(int *piX, int *piY);	
	void ClearScreen(void);
	BYTE GetCh(void);	
	CONSOLEMANAGER* GetConsoleManager(void);
	bool GetKeyFromGUIKeyQueue(KEYDATA* pstData);
	bool PutKeyToGUIKeyQueue(KEYDATA* pstData);
	void SetConsoleShellExitFlag(bool bFlag);
	void PrintStringXY(int iX, int iY, const char* pcString);
	int VSPrintf(char* pcBuffer, const char* pcFormatString, va_list ap);
	void Printf(const char* pcFormatString, ...);
	int ConsolePrintString(const char* pcBuffer);

protected:
	QWORD IToA(QWORD lValue, char* pcBuffer, int iRadix);
	void ReverseString(char* pcBuffer);
	QWORD HexToString(QWORD qwValue, char* pcBuffer);
	int DecimalToString(QWORD lValue, char* pcBuffer);
	

private:
	QWORD m_windowId;	
	int m_commandCount;
	CONSOLEMANAGER m_ConsoleManager;	
	CHARACTER m_vstScreenBuffer[CONSOLE_WIDTH * CONSOLE_HEIGHT];	
	KEYDATA m_vstKeyQueueBuffer[CONSOLE_GUIKEYQUEUE_MAXCOUNT];
	ConsoleManager manager;
	
};