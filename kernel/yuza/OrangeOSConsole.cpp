#include "revision.h"
#include <yuza_support.h>

#if SKY_EMULATOR
#include <SkyVirtualInput.h>
#include "../win32stub/SkyOSWin32Stub.h"

extern unsigned int g_tickCount;

bool StartWin32Timer()
{
	return StartWin32StubTimer(new SkyVirtualInput(), g_tickCount);
} 
#endif

void PrintInfomation()
{
	kprintf("\nYUZA OS Main Entry Entered\n");
	kprintf("revision %d\n", REVISION_BUILD);
	kprintf("version %d.%d\n", REVISION_MAJOR, REVISION_MINOR);
	kprintf("build time %s %s\n\n", BUILD_DATE, BUILD_TIME);
}

void YuzaOSConsole(char* consoleName)
{
	SkyConsole::Clear();
	PrintInfomation();
	PrintCurrentTime();  
	  
	HANDLE handle = kCreateProcess(consoleName, nullptr, 16);  
	SKY_ASSERT(handle != 0, "YuzaOSConsole exec fail!!\n"); 
	WatchDogProc(0); 
	
	//not reached
} 

