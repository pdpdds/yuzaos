#pragma once
#include "PlatformAPI.h"
#include "I_GUIEngine.h"
#include "skyoswindow.h"

typedef struct tagVideoRamInfo
{
	void* _pVideoRamPtr;
	int _width;
	int _height;
	int _bpp;
	UINT8 _framebuffer_type;

	tagVideoRamInfo()
	{
		_pVideoRamPtr = nullptr;
		_width = 0;
		_height = 0;
		_bpp = 0;
		_framebuffer_type = 0;
	}

}VideoRamInfo;

class SkyGUISystem
{
public:	
	~SkyGUISystem();

	bool Initialize(const char* moduleName);
	bool Run();
	bool PutKeyboardQueue(KEYDATA* pData);
	bool PutMouseQueue(MOUSEDATA* pData);
	bool CreateWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId);
	bool DrawWindow(QWORD* windowId, char* buffer, RECT* rect);
	bool DeleteWindow(QWORD* windowId);
	bool ReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent);
	
	bool GetScreenArea(RECT* pScreenArea);
	bool GetWindowArea(QWORD* windowId, RECT* pWindowArea);
	bool DrawText(QWORD* windowId, POINT* point, TEXTCOLOR* textColor, const char* text, int length);
	bool ShowWindow(QWORD* windowId, bool show);
	bool DrawRect(QWORD* qwWindowID, RECT* rect, COLOR color, bool fill);
	bool DrawLine(int left, int top, int right, int bottom, COLOR color);
	bool DrawCircle(int iX, int iY, int iRadius, COLOR color, bool fill);
	bool SendEventToWindow(QWORD* qwWindowID, const EVENT* pstEvent);
	bool SendEventToWindowManager(const EVENT* pstEvent);
	bool FindWindowByTitle(const char* pcTitle, QWORD* qwWindowId);
	bool DrawButton(QWORD* windowId, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor);
	bool UpdateScreenByID(QWORD* qwWindowID);
	bool UpdateScreenByWindowArea(QWORD* qwWindowID, const RECT* pstArea);
	bool BitBlt(QWORD* qwWindowID, RECT* rect, COLOR* pstBuffer, int width, int height);
	void GetCursorPosition(int* piX, int* piY);
	bool GetTopWindowID(QWORD* windowId);
	bool MoveWindowToTop(QWORD* qwWindowID);
	bool MoveWindow(QWORD* qwWindowID, int x, int y);


	bool Print(QWORD taskId, char* pMsg);
	char GetCh();
	bool Clear();

	static SkyGUISystem* GetInstance()
	{
		if (m_GUISystem == nullptr)
			m_GUISystem = new SkyGUISystem();

		return m_GUISystem;
	}

	bool GUIEnable() 
	{ 
		return m_GUIEnable; 
	}

	VideoRamInfo& GetVideoRamInfo() { return m_videoRamInfo; }
	void  SetVideoRamInfo(VideoRamInfo& info) { m_videoRamInfo = info; }

protected:	

private:
	SkyGUISystem();
	static SkyGUISystem* m_GUISystem;

	bool m_GUIEnable;
	VideoRamInfo m_videoRamInfo;
	I_GUIEngine* m_frontEnd;	
};