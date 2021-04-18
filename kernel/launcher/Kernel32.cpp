#include "Kernel32.h"
#include "Memory/PhysicalMemoryManager.h"
#include "Memory/VirtualMemoryManager.h"
#include <memory_layout.h>
#include <SkyConsole.h>
#include "ProcessUtil32.h"
#include "InitCPU.h"
#include "BootParams.h"
#include <x86arch.h>
#include <IDT.h>
#include <Constants.h>
#include <memory.h>
#include <string.h>
#include <BuildOption.h>
#include <intrinsic.h>
#include <ktypes.h>

//i번째 인터럽트 디스크립트를 얻어온다.
extern idt_descriptor* GetInterruptDescriptor(uint32_t i);
void SampleFillRect(ULONG* lfb0, int x, int y, int w, int h, int col);

bool Boot32BitMode(unsigned long magic, multiboot_info_t* pBootInfo, char* szKernelName)
{
	BootParams* pBootParams = 0;
	if (FALSE == InitCPU())
	{
		LOG_FATAL("Init CPU Fail!!\n");
	}

	InitPhysicalMemorySystem(pBootInfo);
	pBootParams = (BootParams *)PhysicalMemoryManager::AllocBlock();
	BuildBootParam(pBootParams, pBootInfo, szKernelName);

	//16MB Identity Mapping
	int maskPageCount = 4;
	InitVirtualMemorySystem(maskPageCount);

	KernelInfo info;
	if (LoadKernel(pBootInfo, szKernelName, &info) == 0)
	{
		LOG_FATAL(" Kernel %s Load Fail!!\n", szKernelName);
	}
	
	pBootParams->_memoryInfo._kernelBase = info._kernelBase;
	pBootParams->magic = magic;
	pBootParams->_memoryInfo._kIdentityBase = 0;
	pBootParams->_memoryInfo._kIdentitySize = 1024 * PAGE_SIZE * 4; //16MB

	//int fl = DisableInterrupts2();
	EnablePaging(false);

	MapHeap(pBootParams);
	MapStack(pBootParams);

	MapFrameBuffer(pBootParams);

	pBootParams->SetAllocated(pBootParams->_memoryInfo._kIdentityBase, pBootParams->_memoryInfo._kIdentitySize, MEMORY_REGION_HIBERNATE);
	pBootParams->SetAllocated(pBootParams->_memoryInfo._kHeapBase, pBootParams->_memoryInfo._kHeapSize, MEMORY_REGION_HIBERNATE);
	pBootParams->SetAllocated(pBootParams->_memoryInfo._kStackBase, pBootParams->_memoryInfo._kStackSize, MEMORY_REGION_HIBERNATE);
	EnablePaging(true);
	//RestoreInterrupts(fl);
	
	VerifyMemory(pBootParams);
	SkyConsole::Print(" Kernel Start! Kernel Entry : %s 0x%x\n", szKernelName, info._kernelEntry);
	
	__asm
	{
		push    pBootParams
		mov		eax, info._kernelEntry
		call	eax;
	}

	return true;
}

bool LoadKernel(multiboot_info_t* pBootInfo, const char* szKernelName, KernelInfo* pModuleInfo)
{
	Module* pModule = FindModule(pBootInfo, szKernelName);
	if (pModule == nullptr)
	{
		SkyConsole::Print("Module Find Fail!! : %s\n", szKernelName);
		return 0;
	}

	bool result = FindModuleEntry(szKernelName, (char*)pModule->ModuleStart, pModuleInfo);

	if (result == false)
	{
		SkyConsole::Print("Invalid kernelEntry Address!!\n");
		return 0;
	}

	if (GetModuleEnd(pBootInfo) > pModuleInfo->_kernelBase)
	{
		SkyConsole::Print("Module space and SKYOS32 image base address was overraped. 0x%x 0x%x\n", GetModuleEnd(pBootInfo), pModuleInfo->_kernelBase);
		SkyConsole::Print("Modify Kernel image base address and check entry point(kmain)\n");
		return false;
	}

	SkyConsole::Print("Copy Image 0x%x to 0x%x\n", pModule->ModuleStart, pModuleInfo->_kernelBase);
	memcpy((void*)pModuleInfo->_kernelBase, (void*)pModule->ModuleStart, ((int)pModule->ModuleEnd - (int)pModule->ModuleStart));

	SkyConsole::Print("Kernel %s Load Success. Location : %x\n", szKernelName, pModuleInfo->_kernelBase);

	Module* pKernel = FindModule(pBootInfo, szKernelName);
	if (pKernel == nullptr)
		return false;

	int pageCount = ((int)pModule->ModuleEnd - (int)pModule->ModuleStart) / PAGE_SIZE;
	if (((int)pKernel->ModuleEnd - (int)pKernel->ModuleStart) % PAGE_SIZE)
		pageCount += 1;
	
	PhysicalMemoryManager::MarkMemBitmap(pModuleInfo->_kernelBase, pageCount * PAGE_SIZE);

	return true;
}

void BuildBootParam(BootParams* pBootParams, multiboot_info_t* pBootInfo, char* szKernelName)
{
	strcpy(pBootParams->_szBootLoaderName, pBootInfo->boot_loader_name);

	pBootParams->_memoryInfo._hyperSpacePTE = (DWORD)PhysicalMemoryManager::AllocBlock();
	pBootParams->_memoryInfo._memorySize = GetTotalMemory(pBootInfo);
	pBootParams->_memoryInfo._IDT = (DWORD)GetInterruptDescriptor(0);
	pBootParams->_memoryInfo._kernelSize = GetModuleSize(pBootInfo, szKernelName);

	pBootParams->_memoryInfo._kPlacementSize = PAGE_SIZE * 10;
	pBootParams->_memoryInfo._kPlacementBase = (DWORD)PhysicalMemoryManager::AllocBlocks(pBootParams->_memoryInfo._kPlacementSize / PAGE_SIZE);
	
	if (SKY_CONSOLE_MODE)
		pBootParams->bGraphicMode = false;
	else
	{
		pBootParams->bGraphicMode = true;
		pBootParams->framebuffer_addr = pBootInfo->framebuffer_addr;
	}

	pBootParams->_moduleCount = pBootInfo->mods_count;
	if (pBootParams->_moduleCount > 0)
	{
		int modules = sizeof(BootModule) * pBootInfo->mods_count / PAGE_SIZE;
		if ((sizeof(BootModule) * pBootInfo->mods_count) % PAGE_SIZE)
			modules += 1;

		pBootParams->Modules = (BootModule*)PhysicalMemoryManager::AllocBlocks(modules);

		for (unsigned int i = 0; i < pBootInfo->mods_count; i++)
		{
			pBootParams->Modules[i].ModuleStart = pBootInfo->Modules[i].ModuleStart;
			pBootParams->Modules[i].ModuleEnd = pBootInfo->Modules[i].ModuleEnd;
			pBootParams->Modules[i].Name = pBootInfo->Modules[i].Name;
		}
	}
}

UINT32 GetSutableHeapSize(UINT64 memorySize)
{
	if (memorySize < 16 * 1024 * 1024)
	{
		LOG_FATAL(" Not Enough Memory %ld\n", memorySize);
	}

	if (memorySize >= 16 * 1024 * 1024 && memorySize < 32 * 1024 * 1024)
	{
		return 1 * 1024 * 1024 * 4;
	}
	if (memorySize >= 32 * 1024 * 1024 && memorySize < 64 * 1024 * 1024)
	{
		return 2 * 1024 * 1024 * 4;
	}
	if (memorySize >= 64 * 1024 * 1024 && memorySize < 128 * 1024 * 1024)
	{
		return 4 * 1024 * 1024 * 4;
	}
	if (memorySize >= 128 * 1024 * 1024 && memorySize < 256 * 1024 * 1024)
	{
		return 8 * 1024 * 1024 * 4;
	}
	if (memorySize >= 256 * 1024 * 1024 && memorySize < 512 * 1024 * 1024)
	{
		return 16 * 1024 * 1024 * 4;
	}
	if (memorySize >= 512 * 1024 * 1024 && memorySize < 1024 * 1024 * 1024)
	{
		return 32 * 1024 * 1024 * 4;
	}
	
	return 64 * 1024 * 1024 * 4;
}

void VerifyMemory(BootParams* pBootParams)
{
	memset((void*)kHeapBase, 0, pBootParams->_memoryInfo._kHeapSize);
}

bool MapHeap(BootParams* pBootParams)
{
	//전체 메모리 크기를 통해 적절한 힙 프레임 카운트 계산
	pBootParams->_memoryInfo._kHeapSize = GetSutableHeapSize(pBootParams->_memoryInfo._memorySize);

	VirtualMemoryManager::MapAddress(VirtualMemoryManager::GetKernelPageDirectory(), kHeapBase, pBootParams->_memoryInfo._kHeapSize / PAGE_SIZE);

	pBootParams->_memoryInfo._kHeapBase = VirtualMemoryManager::GetPhysicalAddress(VirtualMemoryManager::GetKernelPageDirectory(), kHeapBase);
	
	return true;
}

bool MapStack(BootParams* pBootParams)
{
	//커널 부트스택
	DWORD bootStackFrameCount = kKernelStackSize / PAGE_SIZE;
	pBootParams->_memoryInfo._kStackSize = kKernelStackSize;
	pBootParams->_memoryInfo._kStackBase = (UINT32)PhysicalMemoryManager::AllocBlocks(bootStackFrameCount);
	SkyConsole::Print("kernel physical boot stack : %x\n", pBootParams->_memoryInfo._kStackBase);

	return true;
}

bool MapFrameBuffer(BootParams* pBootParams)
{
	//그래픽 주소 매핑
	if (pBootParams->bGraphicMode == true)
	{
		VirtualMemoryManager::MapDMAAddress(VirtualMemoryManager::GetCurPageDirectory(), pBootParams->framebuffer_addr,
			pBootParams->framebuffer_addr, pBootParams->framebuffer_addr + 0x01000000);

		return true;
	}
	return false;
}