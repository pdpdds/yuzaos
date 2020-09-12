#include "revision.h"
#include <yuza_support.h>
#include "test.h"

void PrintInfomation();
void TestCode();

#define YUZA_DEBUGGER 0

void OrangeOSConsole(char* consoleName)
{
	SkyConsole::Clear();
	PrintInfomation();
	PrintCurrentTime();
	 
#if YUZA_DEBUGGER
	Debugger::GetInstance()->DebugKernel();
#else
	//TestCode();
	HANDLE handle = kCreateProcess("console.dll", nullptr, 16);
	SKY_ASSERT(handle != 0, "console.dll exec fail!!\n");
	WatchDogProc(0);
#endif
	
	//not reached
}

void PrintInfomation()
{
	kprintf("\nYUZA OS Main Entry Entered\n");
	kprintf("revision %d\n", REVISION_BUILD);
	kprintf("version %d.%d\n", REVISION_MAJOR, REVISION_MINOR);
	kprintf("build time %s %s\n\n", BUILD_DATE, BUILD_TIME);
}

#include <audio/_pci.h>
#include <audio/audio.h>
void TestCode()
{

#if !defined(SKY_EMULATOR)	
	pci_scan();
	pci_installDevices();
	audio_test(0);
#endif

	avl_tests();
	atomic_tests();
	cbuf_tests();
	test_timers();

	//kCreateThread((THREAD_START_ENTRY)SystemIdle, "Systemidle", 0, 16);
	//Debugger::GetInstance()->DebugKernel();
}
