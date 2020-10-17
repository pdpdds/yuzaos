#pragma once
#include "I_GUIEngine.h"
#include <memory.h>
#include <map>

class GUIEngine : public I_GUIEngine
{
public:
	GUIEngine();
	virtual ~GUIEngine();

	virtual bool Initialize() override;
	virtual void Update(float deltaTime) override;
	virtual void SetLinearBuffer(LinearBufferInfo& linearBufferInfo) override;
	
	virtual bool PutKeyboardQueue(KEYDATA* pData) override;
	virtual bool PutMouseQueue(MOUSEDATA* pData) override;
	virtual bool CreateSkyWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId) override;
	virtual bool DrawWindow(QWORD* windowId, char* buffer,RECT* rect) override;
	virtual bool DeleteWindow(QWORD* windowId) override;
	virtual bool ReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent) override;
	virtual bool GetScreenArea(RECT* pScreenArea) override;
	virtual bool GetWindowArea(QWORD* windowId, RECT* pWindowArea) override;
	virtual bool DrawText(QWORD* windowId, POINT* point, TEXTCOLOR* textColor, const char* text, int length) override;
	virtual bool ShowWindow(QWORD* windowId, bool show) override;
	virtual bool DrawRect(QWORD* qwWindowID, RECT* rect, COLOR color, bool fill) override;
	virtual bool DrawLine(int left, int top, int right, int bottom, COLOR color) override;
	virtual bool DrawCircle(int iX, int iY, int iRadius, COLOR color, bool fill) override;
	virtual bool SendEventToWindow(QWORD* qwWindowID, const EVENT* pstEvent) override;
	virtual bool SendEventToWindowManager(const EVENT* pstEvent) override;
	virtual bool FindWindowByTitle(const char* pcTitle, QWORD* qwWindowId) override;
	virtual bool DrawButton(QWORD* windowId, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor) override;
	virtual bool UpdateScreenByID(QWORD* qwWindowID) override;
	virtual bool GetCursorPosition(int* piX, int* piY) override;
	virtual bool GetTopWindowID(QWORD* windowId) override;
	virtual bool UpdateScreenByWindowArea(QWORD* qwWindowID, const RECT* pstArea) override;
	virtual bool BitBlt(QWORD* qwWindowID, RECT* rect, COLOR* pstBuffer, int width, int height) override;
	virtual bool MoveWindowToTop(QWORD* qwWindowID) override;
	virtual bool MoveWindow(QWORD* qwWindowID, int x, int y) override;

	virtual bool Run() override;
	virtual bool Print(QWORD taskId, char* pMsg) override;
	virtual bool Clear() override;
	virtual char GetCh() override;

private:
	std::map<int, QWORD> m_windowList;
};

