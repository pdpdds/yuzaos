#include "SkyGUISystem.h"
#include "ModuleManager.h"
#include "SkyInputHandler.h"
#include <BuildOption.h>

extern BootParams g_bootParams;

SkyGUISystem* SkyGUISystem::m_GUISystem = nullptr;

SkyGUISystem::SkyGUISystem()
{
	m_GUIEnable = false;
	m_frontEnd = nullptr;	
}

SkyGUISystem::~SkyGUISystem()
{
}

typedef I_GUIEngine* (*PGUIEngine)();

bool SkyGUISystem::Initialize(const char* moduleName)
{
	m_videoRamInfo._pVideoRamPtr = (void*)g_bootParams.framebuffer_addr;
	m_videoRamInfo._width = g_bootParams.framebuffer_width;
	m_videoRamInfo._height = g_bootParams.framebuffer_height;
	m_videoRamInfo._bpp = g_bootParams.framebuffer_bpp;
	m_videoRamInfo._framebuffer_type = g_bootParams.framebuffer_type;

	void* hwnd = ModuleManager::GetInstance()->LoadPE(moduleName);
	PGUIEngine GUIEngine = (PGUIEngine)ModuleManager::GetInstance()->GetModuleFunction(hwnd, "GetGUIEngine");

	m_frontEnd = GUIEngine();
		
	LinearBufferInfo info;
	info.pBuffer = (unsigned long*)m_videoRamInfo._pVideoRamPtr;
	info.width = m_videoRamInfo._width;
	info.height = m_videoRamInfo._height;
	info.depth = m_videoRamInfo._bpp;
	info.type = m_videoRamInfo._framebuffer_type;
	info.isDirectVideoBuffer = true;

	m_frontEnd->SetLinearBuffer(info);
	m_frontEnd->Initialize();

#if !SKY_EMULATOR
	SkyInputHandler::GetInstance();
	SkyInputHandler::GetInstance()->Initialize(nullptr);
#endif

	m_GUIEnable = true;

	return true;
}

bool SkyGUISystem::Run()
{	
	if(m_frontEnd)
		m_frontEnd->Run();

	return true;
}

bool SkyGUISystem::Print(QWORD taskId, char* pMsg)
{
	bool result = false;
	if (m_frontEnd)
		result = m_frontEnd->Print(taskId, pMsg);

	return result;
}

char SkyGUISystem::GetCh()
{
	if (m_frontEnd)
		return m_frontEnd->GetCh();

	return 0;
}

bool SkyGUISystem::Clear()
{
	bool result = false;
	if (m_frontEnd)
		result = m_frontEnd->Clear();

	return result;
}

bool SkyGUISystem::PutKeyboardQueue(KEYDATA* pData)
{
	bool result = false;
	if (m_frontEnd)
		result = m_frontEnd->PutKeyboardQueue(pData);

	return result;
}

bool SkyGUISystem::PutMouseQueue(MOUSEDATA* pData)
{
	bool result = false;
	if (m_frontEnd)
		result = m_frontEnd->PutMouseQueue(pData);

	return result;
}

bool SkyGUISystem::CreateWindow(RECT* rect, const char* title, DWORD flags, QWORD* windowId)
{
	if (m_frontEnd)
		return (QWORD*)m_frontEnd->CreateSkyWindow(rect, title, flags, windowId);

	return false;
}

bool SkyGUISystem::DrawWindow(QWORD* windowId, char* buffer, RECT* rect)
{
	if (m_frontEnd)
		return m_frontEnd->DrawWindow(windowId, buffer, rect);

	return false;
}

bool SkyGUISystem::DeleteWindow(QWORD* windowId)
{
	if (m_frontEnd)
		return (QWORD*)m_frontEnd->DeleteWindow(windowId);

	return false;
}

bool SkyGUISystem::ReceiveEventFromWindowQueue(QWORD* windowId, EVENT* pstEvent)
{
	if (m_frontEnd)
		return (QWORD*)m_frontEnd->ReceiveEventFromWindowQueue(windowId, pstEvent);

	return false;
}

bool SkyGUISystem::GetScreenArea(RECT* pScreenArea)
{
	if (m_frontEnd)
		return (QWORD*)m_frontEnd->GetScreenArea(pScreenArea);

	return false;
}

bool SkyGUISystem::GetWindowArea(QWORD* windowId, RECT* pWindowArea)
{
	if (m_frontEnd)
		return (QWORD*)m_frontEnd->GetWindowArea(windowId, pWindowArea);

	return false;
}

bool SkyGUISystem::DrawText(QWORD* windowId, POINT* point, TEXTCOLOR* textColor, const char* text, int length)
{
	if (m_frontEnd)
		return (QWORD*)m_frontEnd->DrawText(windowId, point, textColor, text, length);

	return false;
}

bool SkyGUISystem::ShowWindow(QWORD* windowId, bool show)
{
	if (m_frontEnd)
		return (QWORD*)m_frontEnd->ShowWindow(windowId, show);

	return false;
}

bool SkyGUISystem::DrawRect(QWORD* qwWindowID, RECT* rect, COLOR color, bool fill)
{
	if (m_frontEnd)
		return m_frontEnd->DrawRect(qwWindowID, rect, color, fill);

	return false;
}

bool SkyGUISystem::DrawLine(int left, int top, int right, int bottom, COLOR color)
{
	if (m_frontEnd)
		return m_frontEnd->DrawLine(left, top, right, bottom, color);

	return false;
}
bool SkyGUISystem::DrawCircle(int iX, int iY, int iRadius, COLOR color, bool fill)
{
	if (m_frontEnd)
		return m_frontEnd->DrawCircle(iX, iY, iRadius, color, fill);

	return false;
}

bool SkyGUISystem::SendEventToWindow(QWORD* qwWindowID, const EVENT* pstEvent)
{
	if (m_frontEnd)
		return m_frontEnd->SendEventToWindow(qwWindowID, pstEvent);

	return false;
}

bool SkyGUISystem::SendEventToWindowManager(const EVENT* pstEvent)
{
	if (m_frontEnd)
		return m_frontEnd->SendEventToWindowManager(pstEvent);

	return false;
}

bool SkyGUISystem::FindWindowByTitle(const char* pcTitle, QWORD* qwWindowId)
{
	if (m_frontEnd)
		return m_frontEnd->FindWindowByTitle(pcTitle, qwWindowId);

	return false;
}

bool SkyGUISystem::DrawButton(QWORD* windowId, RECT* pstButtonArea, COLOR stBackgroundColor, const char* pcText, COLOR stTextColor)
{
	if (m_frontEnd)
		return m_frontEnd->DrawButton(windowId, pstButtonArea, stBackgroundColor, pcText, stTextColor);

	return false;
}

bool SkyGUISystem::UpdateScreenByID(QWORD* qwWindowID)
{
	if (m_frontEnd)
		return m_frontEnd->UpdateScreenByID(qwWindowID);

	return false;
}

void SkyGUISystem::GetCursorPosition(int* piX, int* piY)
{
	if (m_frontEnd)
		m_frontEnd->GetCursorPosition(piX, piY);
}

bool SkyGUISystem::GetTopWindowID(QWORD* windowId)
{
	if (m_frontEnd)
		return m_frontEnd->GetTopWindowID(windowId);

	return false;
}

bool SkyGUISystem::UpdateScreenByWindowArea(QWORD* qwWindowID, const RECT* pstArea)
{
	if (m_frontEnd)
		return m_frontEnd->UpdateScreenByWindowArea(qwWindowID, pstArea);

	return false;
}

bool SkyGUISystem::BitBlt(QWORD* qwWindowID, RECT* rect, COLOR* pstBuffer, int width, int height)
{
	if (m_frontEnd)
		return m_frontEnd->BitBlt(qwWindowID, rect, pstBuffer, width, height);

	return false;
}

bool SkyGUISystem::MoveWindowToTop(QWORD* qwWindowID)
{
	if (m_frontEnd)
		return m_frontEnd->MoveWindowToTop(qwWindowID);

	return false;
}

bool SkyGUISystem::MoveWindow(QWORD* qwWindowID, int x, int y)
{
	if (m_frontEnd)
		return m_frontEnd->MoveWindow(qwWindowID, x, y);

	return false;
}

