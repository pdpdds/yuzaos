#include "yuza_support.h"
#include "BootParams.h"

#define TS_WATCHDOG_CLOCK_POS		(0xb8000+(80-1)*2)
#define TIMEOUT_PER_SECOND		1000
static bool m_bShowTSWatchdogClock = true;
extern bool InitOSSystem(BootParams* pBootParam);

void SampleFillRect(ULONG* lfb0, int x, int y, int w, int h, int col)
{
	for (int k = 0; k < h; k++)
		for (int j = 0; j < w; j++)
		{
			int index = ((j + x) + (k + y) * 1024);
			lfb0[index] = col;
		}
}

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