#pragma once
#include "minwindef.h"
#include "stdint.h"
#include <MultiBoot.h>

	
#define PMM_BLOCK_SIZE	4096	
#define PMM_BLOCK_ALIGN	BLOCK_SIZE
#define PMM_BITS_PER_INDEX	32

namespace PhysicalMemoryManager
{
	void	Initialize(UINT32 moduleEnd, UINT memorySize, multiboot_info_t* pBootInfo);

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
	void*	AllocBlock();
	void	FreeBlock(void* pa);

	void*	AllocBlocks(size_t);
	void	FreeBlocks(void* pa, size_t);

	void MarkMemBitmap(UINT32 base, size_t size);
	void UnmarkMemBitmap(UINT32 base, size_t size);

	int		GetBitmapEnd();
	
	
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
	UINT32	GetTotalMemory();
	UINT32	GetFreeMemory();	
	void	Dump(); //Debug
}	
