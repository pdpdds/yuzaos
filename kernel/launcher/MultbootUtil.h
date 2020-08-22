#pragma once
#include "MultiBoot.h"
#include "BootParams.h"

uint32_t GetTotalMemory(multiboot_info* bootinfo);
uint32_t GetModuleEnd(multiboot_info* bootinfo);
Module* FindModule(multiboot_info_t* pInfo, const char* szFileName);
uint32_t GetModuleSize(multiboot_info_t* pInfo, const char* szFileName);
bool IsKernel64(multiboot_info_t* pInfo, const char* szFileName);

typedef struct tag_KernelInfo
{
	uint32_t _kernelEntry;
	uint32_t _kernelBase;

}KernelInfo;
