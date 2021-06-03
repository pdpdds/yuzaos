#include "MultbootUtil.h"
#include "windef.h"
#include "MultiBoot.h"
#include "Constants.h"
#include <stringdef.h>
#include <BuildOption.h>
#include <SkyConsole.h>
#include <ProcessUtil64.h>
#include <ktypes.h>

uint32_t GetTotalMemory(multiboot_info* bootinfo)
{
	uint32_t mmapEntryNum = bootinfo->mmap_length / sizeof(multiboot_memory_map_t);

	SkyConsole::Print("Memory Region Information\n");
	SkyConsole::Print("Memory Map Entry Num : %d\n", mmapEntryNum);

	multiboot_mmap_entry* mmapAddr = (multiboot_mmap_entry*)bootinfo->mmap_addr;
	
	uint32_t memorySize = 0;
	
	for (uint32_t i = 0; i < mmapEntryNum; i++)
	{
		uint64_t areaStart = (uint64_t)mmapAddr[i].baseAddressLower | ((uint64_t)mmapAddr[i].baseAddressHigher << 32);
		uint64_t areaEnd = areaStart + ((uint64_t)mmapAddr[i].lengthLower | ((uint64_t)mmapAddr[i].lengthHigher << 32));
		
		switch (mmapAddr[i].type)
		{
		case MEMORY_REGION_AVAILABLE:
		{
			SkyConsole::Print("MEMORY_REGION_AVAILABLE : %q, %q\n", areaStart, areaEnd);
		
			if (areaEnd > memorySize)
				memorySize = areaEnd;

			//pParam->SetAllocated(areaStart, areaEnd, mmapAddr[i].type);
		}
			break;
		case MEMORY_REGION_HIBERNATE:
			SkyConsole::Print("MEMORY_REGION_HIBERNATE : %q, %q\n", areaStart, areaEnd);
			break;
		default:
			SkyConsole::Print("MEMORY_REGION %d : %q, %q\n", mmapAddr[i].type, areaStart, areaEnd);
			break;
		}
	}
	
	return memorySize;
}

/**
uint32_t GetKernelSize(multiboot_info* bootinfo)
{
	uint64_t endAddress = 0;
	uint32_t mods_count = bootinfo->mods_count;   // Get the amount of modules available 
	uint32_t mods_addr = (uint32_t)bootinfo->Modules;     // And the starting address of the modules 
	for (uint32_t i = 0; i < mods_count; i++) {
		Module* module = (Module*)(mods_addr + (i * sizeof(Module)));     // Loop through all modules 

		if (strcmp(module->Name, KERNEL32_NAME) == 0)
		{
			uint32_t moduleStart = PAGE_ALIGN_DOWN((uint32_t)module->ModuleStart);
			uint32_t moduleEnd = PAGE_ALIGN_UP((uint32_t)module->ModuleEnd);

			return moduleEnd - moduleStart;
		}
	}

	return 0;
}*/

uint32_t GetModuleEnd(multiboot_info* bootinfo)
{
	uint32_t endAddress = 0;
	uint32_t mods_count = bootinfo->mods_count;   /* Get the amount of modules available */
	uint32_t mods_addr = (uint32_t)bootinfo->Modules;     /* And the starting address of the modules */
	for (uint32_t i = 0; i < mods_count; i++) {
		Module* module = (Module*)(mods_addr + (i * sizeof(Module)));     /* Loop through all modules */

		uint32_t moduleStart = PAGE_ALIGN_DOWN((uint32_t)module->ModuleStart);
		uint32_t moduleEnd = PAGE_ALIGN_UP((uint32_t)module->ModuleEnd);

		if (endAddress < moduleEnd)
		{
			endAddress = moduleEnd;
		}

		SkyConsole::Print("%s (0x%x - 0x%x)\n", module->Name, moduleStart, moduleEnd);
	}

	return (uint32_t)endAddress;
}

Module* FindModule(multiboot_info_t* pInfo, const char* szFileName)
{
	uint32_t mb_flags = pInfo->flags;
	void* kentry = nullptr;

	if (mb_flags & MULTIBOOT_INFO_MODS)
	{
		uint32_t mods_count = pInfo->mods_count;
		uint32_t mods_addr = (uint32_t)pInfo->Modules;

		for (uint32_t mod = 0; mod < mods_count; mod++)
		{
			Module* module = (Module*)(mods_addr + (mod * sizeof(Module)));

			const char* module_string = (const char*)module->Name;

			SkyConsole::Print("Module Name : %s\n", module_string);

			if (strcmp(module_string, szFileName) == 0)
			{
				return module;
			}
		}
	}

	return nullptr;
}

uint32_t GetModuleSize(multiboot_info_t* pInfo, const char* szFileName)
{
	Module* pModule = FindModule(pInfo, szFileName);

	if (pModule == 0)
		return 0;

	int mapPageCount = ((int)pModule->ModuleEnd - (int)pModule->ModuleStart) / PAGE_SIZE;

	if (((int)pModule->ModuleEnd - (int)pModule->ModuleStart) % PAGE_SIZE > 0)
		mapPageCount += 1;

	return mapPageCount * PAGE_SIZE;
}

bool IsKernel64(multiboot_info_t* pInfo, const char* szFileName)
{
	Module* pModule = FindModule(pInfo, szFileName);

	if (pModule == nullptr)
		return false;

	return ValidatePEImage64((void*)pModule->ModuleStart);
}

void LOG_FATAL(char* fmt, ...)
{
	char buf[4096];

	va_list arglist;
	va_start(arglist, fmt);
	vsnprintf(buf, 4096, fmt, arglist);
	va_end(arglist);
	buf[4095] = 0;

	SkyConsole::Print(buf);
	for (;;);
}