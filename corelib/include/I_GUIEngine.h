#pragma once
#include "windef.h"
#include "skyoswindow.h"
#include "I_SkyInput.h"

// 키 큐에 대한 매크로
// 키 큐의 최대 크기
#define KEY_MAXQUEUECOUNT	100

////////////////////////////////////////////////////////////////////////////////
// 마우스 큐에 대한 매크로
#define MOUSE_MAXQUEUECOUNT 100

// 버튼의 상태를 나타내는 매크로
#define MOUSE_LBUTTONDOWN   0x01
#define MOUSE_RBUTTONDOWN   0x02
#define MOUSE_MBUTTONDOWN   0x04

#pragma pack( push, 1 )

// 스캔 코드 테이블을 구성하는 항목
typedef struct kKeyMappingEntryStruct
{
	// Shift 키나 Caps Lock 키와 조합되지 않는 ASCII 코드
	BYTE bNormalCode;

	// Shift 키나 Caps Lock 키와 조합된 ASCII 코드
	BYTE bCombinedCode;
} KEYMAPPINGENTRY;

typedef struct kKeycodeAsciiMappingEntryStruct
{	
	unsigned int sdlKeycode;
	BYTE bNormalCode;
	BYTE bCombinedCode;
} KEYCODEASCIIMAPPINGENTRY;

// 키보드의 상태를 관리하는 자료구조
typedef struct tag_KEYBOARDSTATE
{
	// 조합 키 정보
	bool bShiftDown;
	bool bCapsLockOn;
	bool bNumLockOn;
	bool bScrollLockOn;

	// 확장 키를 처리하기 위한 정보
	bool bExtendedCodeIn;
	int iSkipCountForPause;
} KEYBOARDSTATE;

typedef struct tag_LinearBufferInfo
{
	unsigned long* pBuffer;
	unsigned long width;
	unsigned long height;
	unsigned long depth;
	unsigned type;
	bool isDirectVideoBuffer;

} LinearBufferInfo;

#pragma pack( pop )

#ifndef QWORD
typedef long long QWORD;
#endif // !QWORD


class I_GUIEngine
{
public:
	virtual bool Initialize() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void SetLinearBuffer(LinearBufferInfo& linearBufferInfo) = 0;
	virtual bool PutKeyboardQueue(KEYDATA* pData) = 0;
	virtual bool PutMouseQueue(MOUSEDATA* pData) = 0;
	virtual bool Run() = 0;
	virtual bool Print(QWORD taskId, char* pMsg) = 0;
	virtual bool Clear() = 0;
	virtual char GetCh() = 0;

	virtual bool CreateSkyWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId) { return false; }
	virtual bool DrawWindow(QWORD* windowId, char* buffer, RECT* rect) { return false; }
	virtual bool DeleteWindow(QWORD* windowId) { return false; }
	virtual bool ReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent) { return false; }
	
	virtual bool GetScreenArea(RECT* pScreenArea) { return false; }
	virtual bool GetWindowArea(QWORD* windowId, RECT* pWindowArea) { return false; }
	virtual bool DrawText(QWORD* windowId, POINT* point, TEXTCOLOR* textColor, const char* text, int length) { return false; }
	virtual bool ShowWindow(QWORD* windowId, bool show) { return false; }
	virtual bool DrawRect(QWORD* qwWindowID, RECT* rect, COLOR color, bool fill) { return false; }
	virtual bool DrawLine(int left, int top, int right, int bottom, COLOR color) { return false; }
	virtual bool DrawCircle(int iX, int iY, int iRadius, COLOR color, bool fill) { return false; }
	
	virtual bool SendEventToWindow(QWORD* qwWindowID, const EVENT* pstEvent) { return false; }
	virtual bool SendEventToWindowManager(const EVENT* pstEvent) { return false; }
	virtual bool FindWindowByTitle(const char* pcTitle, QWORD* qwWindowId) { return false; }
	virtual bool DrawButton(QWORD* windowId, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor) { return false; }
	virtual bool UpdateScreenByID(QWORD* qwWindowID) { return false; }
	virtual bool UpdateScreenByWindowArea(QWORD* qwWindowID, const RECT* pstArea) { return false; }
	virtual bool GetCursorPosition(int* piX, int* piY) { return false; }
	virtual bool GetTopWindowID(QWORD* windowId) { return false; }
	virtual bool BitBlt(QWORD* qwWindowID, RECT* rect, COLOR* pstBuffer, int width, int height) { return false; }
	virtual bool MoveWindowToTop(QWORD* qwWindowID) { return false; }
	virtual bool MoveWindow(QWORD* qwWindowID, int x, int y) { return false; }

	LinearBufferInfo m_linearBufferInfo;
};