#include "revision.h"
#include <SystemAPI.h>
#include <SkyConsole.h>
#include <Debugger.h>
#include "test.h"

void PrintInfomation();
DWORD WINAPI WatchDogProc(LPVOID parameter);
DWORD WINAPI SystemIdle(LPVOID parameter);

#include <audio/_pci.h>
#include <audio/audio.h>
void OrangeOSConsole()
{
	SkyConsole::Clear();
	PrintInfomation();
#if !defined(SKY_EMULATOR)	
	pci_scan();
	pci_installDevices();
	audio_test(0);
#endif

	//avl_tests();
	//atomic_tests();
	//cbuf_tests();
	//test_timers();
	//kCreateProcess("console.dll", nullptr, 16);
	
	kCreateProcess("helloworld.exe", nullptr, 16);
	//kCreateThread((THREAD_START_ENTRY)SystemIdle, "Systemidle", 0, 16);

	//Debugger::GetInstance()->DebugKernel();

	WatchDogProc(0);
	
	//not reached
}

#define TS_WATCHDOG_CLOCK_POS		(0xb8000+(80-1)*2)
#define TIMEOUT_PER_SECOND		1000
static bool m_bShowTSWatchdogClock = true;

DWORD WINAPI WatchDogProc(LPVOID parameter)
{
	int pos = 0;
	char* addr = (char*)TS_WATCHDOG_CLOCK_POS, status[] = { '-', '\\', '|', '/', '-', '\\', '|', '/' };
	int first = kGetTickCount();
	int j = 1;
	kprintf("j %x\n", &j);
	while (1)
	{
		int second = kGetTickCount();
		if (second - first >= TIMEOUT_PER_SECOND)
		{
			if (++pos > 7)
				pos = 0;

			if (m_bShowTSWatchdogClock)
			{
#if !SKY_EMULATOR
				* addr = status[pos];
#endif // !SKY_EMULAOTR				
			}

			first = kGetTickCount();
		}

		kSleep(1000);
	}

	return 0;
}

void PrintInfomation()
{
	kprintf("\nYUZA OS Main Entry Entered\n");
	kprintf("revision %d\n", REVISION_BUILD);
	kprintf("version %d.%d\n", REVISION_MAJOR, REVISION_MINOR);
	kprintf("build time %s %s\n\n", BUILD_DATE, BUILD_TIME);
}

