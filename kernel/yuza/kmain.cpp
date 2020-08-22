#include "SystemAPI.h"
#include "SkyFacade.h"
#include "BootParams.h"
#include <memory.h>
#include <memory_layout.h>
#include <IDT.h>
#include <x86arch.h>

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