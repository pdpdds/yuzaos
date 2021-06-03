#include "windef.h"
#include "stdint.h"
#include "Page.h"
#include "PageCache.h"
#include "physicalmap.h"
#include "AddressSpace.h"
#include "kerneldef.h"
#include "Timer.h"
#include "Team.h"
#include "Thread.h"
#include "Processor.h"
#include "TeamManager.h"
#include <SystemAPI.h>
#include <Debugger.h>
#include <memory_layout.h>
#include <Area.h>
#include <SkyConsole.h>
#include <x86arch.h>
#include <intrinsic.h>

extern bool InitKernelSystem()
{
	Debugger::GetInstance()->Bootstrap();

	Thread::Bootstrap();
	
	Timer::Bootstrap();
	Page::Bootstrap();
	
	PageCache::Bootstrap();
	
	PhysicalMap::Bootstrap();
	AddressSpace::Bootstrap();
	
	TeamManager::GetInstance()->Bootstrap();
	
	Processor::Bootstrap();

	kprintf("kernel system init complete.\n");

	return true;
}

void StartNativeSystem(void* param)
{
	AddressSpace::PageDaemonLoop();
}