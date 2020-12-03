#include "revision.h"
#include <yuza_support.h>

#if SKY_EMULATOR
#include "SkyVirtualInput.h"
#include "../win32stub/SkyOSWin32Stub.h"

extern unsigned int g_tickCount;
bool StartWin32Timer()
{
	return StartWin32StubTimer(new SkyVirtualInput(), g_tickCount);
}
#endif

void YuzaOSConsole(char* consoleName)
{
	SkyConsole::Clear();
	PrintCurrentTime();
	//HANDLE handle = kCreateProcess(consoleName, nullptr, 16);
	
	Debugger::GetInstance()->DebugKernel();

	WatchDogProc(0);
}

void YuzaOSGUI(char* desktopName)
{

}

