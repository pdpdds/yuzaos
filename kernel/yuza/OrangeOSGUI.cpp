#include "SystemAPI.h"
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
DWORD WINAPI SystemIdle(LPVOID parameter);
void OrangeOSGUI()
{
	//kCreateThread(GUIManagerThread, "GUIManager", "GUIConsole.dll", 16);
	kCreateThread(GUIManagerThread, "GUIManager", "desktopmgr.dll", 16);
	//kCreateThread(GUIManagerThread, "GUIManager", "luakernel.dll", 16);
	//kCreateThread(GUIManagerThread, "GUIManager", "imgui_mgr.dll", 16);
#if SKY_EMULATOR
	StartVirtualFramework(0);
#else
	SystemIdle(0);
#endif

	//not reached
}