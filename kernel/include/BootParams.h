#pragma once
#include "minwindef.h"

typedef struct tag_BootModule
{
	UINT32 ModuleStart;
	UINT32 ModuleEnd;
	char *Name;
	unsigned int Reserved;
}BootModule;

typedef struct tag_PhysicalMemoryInfo
{
	UINT64 _kIdentityBase;
	UINT64 _kIdentitySize;

	UINT32 _kernelBase;
	UINT64 _kernelSize;
	
	UINT32 _kHeapBase;
	UINT32 _kHeapSize;

	UINT32 _kPlacementBase;
	UINT32 _kPlacementSize;

	UINT32 _kStackBase;
	UINT32 _kStackSize;

	UINT64 _memorySize;
	DWORD _hyperSpacePTE;
	DWORD _GDT;
	DWORD _IDT;
}PhysicalMemoryInfo;

typedef struct tag_BootParams
{
	unsigned long magic = 0;
	bool bGraphicMode = 0;
	PhysicalMemoryInfo _memoryInfo;
	
	char _szBootLoaderName[256] = { 0, };
	int  _moduleCount;
	BootModule *Modules;

	UINT64 framebuffer_addr = 0;
	UINT32 framebuffer_pitch = 0;
	UINT32 framebuffer_width = 0;
	UINT32 framebuffer_height = 0;
	UINT8 framebuffer_bpp = 0;

#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
	UINT8 framebuffer_type;
	int _memoryRegionCount;

#define MEMORY_REGION_AVAILABLE 1
#define MEMORY_REGION_HIBERNATE 2
	struct MemoryRegion {
		UINT64 begin;
		UINT64 end;
		int type;
	} MemoryRegion[64];

	tag_BootParams()
	{
		magic = 0;
		bGraphicMode = false;
		
		_memoryRegionCount = 0;
		_moduleCount = 0;
		Modules = 0;
	}

	inline void SetAllocated(UINT64 begin, UINT64 end, int type)
	{
		MemoryRegion[_memoryRegionCount].begin = begin;
		MemoryRegion[_memoryRegionCount].end = end;
		MemoryRegion[_memoryRegionCount].type = type;
		_memoryRegionCount++;
	}

}BootParams;