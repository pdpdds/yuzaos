#include "InitCPU.h"
#include "CPU/GDT.h"
#include "IDT.h"
#include "CPU/FPU.h"
#include "CPU/PIC.h"
#include "CPU/TSS.h"
#include "Memory/VirtualMemoryManager.h"
#include "Memory/PhysicalMemoryManager.h"
#include "SkyConsole.h"
#include <x86arch.h>

extern bool InitIDT(uint16_t codeSel);

bool InitCPU()
{
	InitGDT();
	InitIDT(0x8);
	
	//InitTSS(5, 0x10, 0);
	
	InitPIC(0x20, 0x28);
	//InitPIT(); //OLD TIMER INTERRPUT HANDLER.....
	
	if (false == InitFPU())
		return false;

	EnableFPU();

	SkyConsole::Print("CPU Init Pass\n");
	
	return true;
}

bool InitPhysicalMemorySystem(multiboot_info_t* pBootInfo)
{
	uint32_t moduleEndAddress = GetModuleEnd(pBootInfo);
	UINT memorySize = GetTotalMemory(pBootInfo);
	PhysicalMemoryManager::Initialize(moduleEndAddress, memorySize, pBootInfo);
	SkyConsole::Print("Physical Memory System Init\n");
	return true;
}

bool InitVirtualMemorySystem(int maskPageCount)
{

	VirtualMemoryManager::Initialize(maskPageCount);
	SkyConsole::Print("Virtual Memory System Init\n");
	return true;
	
}




