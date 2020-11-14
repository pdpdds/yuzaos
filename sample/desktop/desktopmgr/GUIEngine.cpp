#include "GUIEngine.h"
#include "windef.h"
#include "stringdef.h"
#include "memory.h"
#include "WindowManagerTask.h"
#include "InputQueue.h"
#include <SystemCall_Impl.h>
#include <Window.h>

extern "C" __declspec(dllexport) I_GUIEngine* GetGUIEngine()
{
	return new GUIEngine();
}

extern QUEUE gs_stKeyQueue;
extern QUEUE gs_stMouseQueue;

GUIEngine::GUIEngine()
{
	
}

GUIEngine::~GUIEngine()
{
}

bool GUIEngine::Run()
{
	Syscall_CreateProcess("panel.dll", 0, 16);
	Syscall_CreateProcess("cmd.dll", 0, 16);

	Update(0);

	return true;
}

bool GUIEngine::Print(QWORD taskId, char* pMsg)
{
	if (taskId)
	{
		EVENT stEvent;
		stEvent.qwType = EVENT_CONSOLE_PRINT;
		
		stEvent.stWindowEvent.qwWindowID = taskId;
		strcpy(stEvent.stWindowEvent.stringEvent, pMsg);

		kSendEventToWindow(taskId, &stEvent);
		
		return true;
	}

	return false;
}



bool GUIEngine::Clear()
{
	return true;
}

bool GUIEngine::Initialize() 
{	
	kStartWindowManager(&m_linearBufferInfo);

	return true;
}

void GUIEngine::Update(float deltaTime) 
{
	kUpdate(m_linearBufferInfo.isDirectVideoBuffer);
}

void GUIEngine::SetLinearBuffer(LinearBufferInfo& linearBufferInfo)
{
	m_linearBufferInfo = linearBufferInfo;
}

bool GUIEngine::PutKeyboardQueue(KEYDATA* pData)
{
	return kPutQueue(&gs_stKeyQueue, pData);
}

bool  GUIEngine::PutMouseQueue(MOUSEDATA* pData)
{
	return kPutQueue(&gs_stMouseQueue, pData);
}

bool GUIEngine::CreateSkyWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId)
{
	*windowId = kCreateWindow(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, flags, title);
	//*windowId =  kCreateSkyWindow(rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, flags, title);

	if (*windowId == 0 || *windowId == WINDOW_INVALIDID)
		return false;

	m_windowList[Syscall_GetCurrentThreadId()] = *windowId;
	return true;
}

bool GUIEngine::DrawWindow(QWORD* windowId, char* buffer, RECT* rect)
{
	QWORD windowIdentifer = 0;
	if (windowId != 0)
		windowIdentifer = *windowId;
	else windowIdentifer = m_windowList[Syscall_GetCurrentThreadId()];

	bool result = kBitBlt(windowIdentifer, rect->left + 2, WINDOW_TITLEBAR_HEIGHT, (COLOR*)buffer,
		rect->right - rect->left, rect->bottom - rect->top);
	kShowWindow(windowIdentifer, true);
	return result;
}

bool GUIEngine::DeleteWindow(QWORD* windowId)
{
	QWORD windowIdentifer = 0;
	if (windowId != 0)
		windowIdentifer = *windowId;
	else windowIdentifer = m_windowList[Syscall_GetCurrentThreadId()];
	return kDeleteWindow(windowIdentifer);
}

char GUIEngine::GetCh()
{
	QWORD qwWindowID = 0;
	Syscall_GetCurrentConsoleWindowId(&qwWindowID);

	return kGetKeyFromConsoleWindow(qwWindowID);
}

bool GUIEngine::ReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent)
{
	QWORD windowIdentifer = 0;
	if (windowId != 0)
		windowIdentifer = *windowId;
	else windowIdentifer = m_windowList[Syscall_GetCurrentThreadId()];

	return kReceiveEventFromWindowQueue(windowIdentifer, pstEvent);
}

bool GUIEngine::GetScreenArea(RECT* pScreenArea)
{
	kGetScreenArea(pScreenArea);
	return true;
}

bool GUIEngine::DrawText(QWORD* windowId, POINT* point, TEXTCOLOR* textColor, const char* text, int length)
{
	QWORD windowIdentifer = 0;
	if (windowId != 0)
		windowIdentifer = *windowId;
	else windowIdentifer = m_windowList[Syscall_GetCurrentThreadId()];

	kDrawText(windowIdentifer, point->iX, point->iY, textColor->textColor, textColor->backgroundColor, text, length);
	return true;
}

bool GUIEngine::ShowWindow(QWORD* windowId, bool show)
{
	QWORD windowIdentifer = 0;
	if (windowId != 0)
		windowIdentifer = *windowId;
	else windowIdentifer = m_windowList[Syscall_GetCurrentThreadId()];
	kShowWindow(windowIdentifer, show);
	return true;
}

bool GUIEngine::DrawRect(QWORD* qwWindowID, RECT* rect, COLOR color, bool fill)
{
	kDrawRect(*qwWindowID, rect->left, rect->top, rect->right, rect->bottom, color, fill);
	return true;
}

bool GUIEngine::DrawLine(int left, int top, int right, int bottom, COLOR color)
{
	QWORD qwWindowID = m_windowList[Syscall_GetCurrentThreadId()];
	kDrawLine(qwWindowID, left, top, right, bottom, color);
	return true;
}

bool GUIEngine::DrawCircle(int iX, int iY, int iRadius, COLOR color, bool fill)
{
	QWORD qwWindowID = m_windowList[Syscall_GetCurrentThreadId()];
	kDrawCircle(qwWindowID, iX, iY, iRadius, color, fill);
	return true;
}

bool GUIEngine::GetWindowArea(QWORD* windowId, RECT* pWindowArea)
{
	QWORD windowIdentifer = 0;
	if (windowId != 0)
		windowIdentifer = *windowId;
	else windowIdentifer = m_windowList[Syscall_GetCurrentThreadId()];

	kGetWindowArea(windowIdentifer, pWindowArea);
	return true;
}

bool GUIEngine::SendEventToWindow(QWORD* qwWindowID, const EVENT* pstEvent)
{
	kSendEventToWindow(*qwWindowID, pstEvent);
	return true;
}

bool GUIEngine::SendEventToWindowManager(const EVENT* pstEvent)
{
	kSendEventToWindowManager(pstEvent);
	return true;
}

bool GUIEngine::FindWindowByTitle(const char* pcTitle, QWORD* qwWindowId)
{
	*qwWindowId = kFindWindowByTitle(pcTitle);

	if(*qwWindowId != 0)
		return true;

	return false;
}

bool GUIEngine::DrawButton(QWORD* windowId, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor)
{
	kDrawButton(*windowId, pstButtonArea, stBackgroundColor, pcText, stTextColor);
	return true;
}

bool GUIEngine::UpdateScreenByID(QWORD* qwWindowID)
{
	kUpdateScreenByID(*qwWindowID);
	return true;
}

bool GUIEngine::GetCursorPosition(int* piX, int* piY)
{
	kGetCursorPosition(piX, piY);
	return true;
}

bool GUIEngine::GetTopWindowID(QWORD* windowId)
{
	*windowId = kGetTopWindowID();
	return true;
}

bool GUIEngine::UpdateScreenByWindowArea(QWORD* qwWindowID, const RECT* pstArea)
{
	return kUpdateScreenByWindowArea(*qwWindowID, pstArea);
}

bool GUIEngine::BitBlt(QWORD* windowId, RECT* rect, COLOR* buffer, int width, int height)
{
	QWORD windowIdentifer = 0;
	if (windowId != 0)
		windowIdentifer = *windowId;
	else windowIdentifer = m_windowList[Syscall_GetCurrentThreadId()];

	bool result = kBitBltWithRect(windowIdentifer, rect, (COLOR*)buffer, width, height);
	kShowWindow(windowIdentifer, true);
	return result;
}

bool GUIEngine::MoveWindowToTop(QWORD* qwWindowID)
{
	return kMoveWindowToTop(*qwWindowID);
}

bool GUIEngine::MoveWindow(QWORD* qwWindowID, int x, int y)
{
	return kMoveWindow(*qwWindowID, x, y);
}
