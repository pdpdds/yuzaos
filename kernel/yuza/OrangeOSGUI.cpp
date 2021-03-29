#include "SystemAPI.h"
#include <yuza_support.h>
#include "SkyVirtualInput.h"
#include <SkyGUISystem.h>

#if SKY_EMULATOR
#include "../win32stub/SkyOSWin32Stub.h"

extern unsigned int g_tickCount;
extern bool SetFrameBufferInfo(WIN32_VIDEO* pVideoInfo);

DWORD WINAPI StartVirtualFramework(LPVOID parameter)
{
	LoopWin32(new SkyVirtualInput(), g_tickCount);
	return 0;
}

#endif

int GUIManagerThread(void* param)
{ 
	SkyGUISystem::GetInstance()->Initialize((const char*)param);
	SkyGUISystem::GetInstance()->Run();

	return 0;
}

void YuzaOSGUI(char* desktopName)
{	
#if SKY_EMULATOR
	StartVirtualFramework(0);
	WIN32_VIDEO* pVideo = GetFrameBufferInfo();
	SetFrameBufferInfo(pVideo);
#endif

	kCreateThread(GUIManagerThread, "GUIManager", desktopName, 16, 0);
	SystemIdle(0);

	//not reached
}