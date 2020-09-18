#include "SystemAPI.h"
#include <yuza_support.h>
#include "SkyVirtualInput.h"
#include <SkyGUISystem.h>

#if SKY_EMULATOR
#include "../win32stub/SkyOSWin32Stub.h"

extern unsigned int g_tickCount;

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

void OrangeOSGUI(char* desktopName)
{	
	kCreateThread(GUIManagerThread, "GUIManager", desktopName, 16);

#if SKY_EMULATOR
	StartVirtualFramework(0);
#else
	SystemIdle(0);
#endif

	//not reached
}