#include "yuza_support.h"
#include "SkyFacade.h"

#define TS_WATCHDOG_CLOCK_POS		(0xb8000+(80-1)*2)
#define TIMEOUT_PER_SECOND		1000
static bool m_bShowTSWatchdogClock = true;

DWORD WINAPI WatchDogProc(LPVOID parameter)
{
	int pos = 0;
	char* addr = (char*)TS_WATCHDOG_CLOCK_POS, status[] = { '-', '\\', '|', '/', '-', '\\', '|', '/' };
	int first = kGetTickCount();

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

DWORD WINAPI SystemIdle(LPVOID parameter)
{
	while (1)
	{
		kSleep(1000);
	}

	return 0;
}

void kmain(BootParams* pBootParam)
{
	InitOSSystem(pBootParam);
	//not reached!!
}