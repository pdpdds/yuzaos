#pragma once
#include "MultbootUtil.h"

bool Boot32BitMode(unsigned long magic, multiboot_info_t* pBootInfo, char* szKernelName);
bool LoadKernel(multiboot_info_t* pBootInfo, const char* szKernelName, KernelInfo* pModuleInfo);
void BuildBootParam(BootParams* pBootParams, multiboot_info_t* pBootInfo, char* szKernelName);
bool MapHeap(BootParams* pBootParams);
bool MapStack(BootParams* pBootParams);
bool MapFrameBuffer(BootParams* pBootParams);
int GetSutableHeapFrameCount(UINT64 memorySize);
void VerifyMemory(BootParams* pBootParams);