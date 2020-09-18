#include "Kernel64.h"
#include "SkyConsole.h"
#include "ProcessUtil64.h"
#include "PEImage64.h"
#include <memory.h>
#include "Page.h"

extern "C" void SwitchAndExecute64bitKernel(int pml4EntryAddress, int kernelEntry, int bootinfo);

bool Is64BitSwitchPossible()
{
	if (DetectionCPUID() == false)
		return false;

	SkyConsole::Print("CPUID Detected..\n");

	if (IsLongModeCheckPossible() == false)
		return false;

	SkyConsole::Print("Long Mode Check Possible..\n");

	if (IsLongModePossible() == false)
		return false;

	SkyConsole::Print("Long Mode Possible..\n");

	return true;
}


bool DetectionCPUID()
{
	bool result = false;
	__asm
	{
		pushfd
		pop eax

		; Copy to ECX as well for comparing later on
		mov ecx, eax

		; Flip the ID bit
		xor eax, 1 << 21

		; Copy EAX to FLAGS via the stack
		push eax
		popfd

		; Copy FLAGS back to EAX(with the flipped bit if CPUID is supported)
		pushfd
		pop eax

		; Restore FLAGS from the old version stored in ECX(i.e.flipping the ID bit
		; back if it was ever flipped).
		push ecx
		popfd

		; Compare EAXand ECX.If they are equal then that means the bit wasn't
		; flipped, and CPUID isn't supported.
		xor eax, ecx
		jz NoCPUID
		mov result, 1
		NoCPUID:

	}

	return result;
}

bool IsLongModeCheckPossible()
{
	bool result = false;
	__asm
	{
		mov eax, 0x80000000; Set the A - register to 0x80000000.
		cpuid; CPU identification.
		cmp eax, 0x80000001; Compare the A - register with 0x80000001.
		jb NoLongMode; It is less, there is no long mode.
		mov result, 1
		NoLongMode:
	}

	return result;
}

bool IsLongModePossible()
{
	bool result = false;
	__asm
	{
		mov eax, 0x80000001; Set the A - register to 0x80000001.
		cpuid; CPU identification.
		test edx, 1 << 29; Test if the LM - bit, which is bit 29, is set in the D - register.
		jz NoLongMode; They aren't, there is no long mode.
		mov result, 1
		NoLongMode:
	}

	return result;
}

bool Boot64BitMode(multiboot_info_t* pBootInfo, char* szKernelName)
{
	if (Is64BitSwitchPossible() == false)
	{
		SkyConsole::Print("Impossible 64bit Mode\n");
		return false;
	}

	Module* pModule = FindModule(pBootInfo, szKernelName);

	if (pModule == nullptr)
	{
		LOG_FATAL(" %s Kernel Found Fail!!\n", szKernelName);
	}

	//커널의 이미지 베이스 주소와 커널 엔트리를 찾는다.
	uint32_t kernelEntry = 0;
	uint32_t imageBase = 0;
	kernelEntry = FindKernel64Entry(szKernelName, (char*)pModule->ModuleStart, imageBase);

	if (kernelEntry == 0 || imageBase == 0)
	{
		SkyConsole::Print("Invalid Kernel64 Address!!\n");
		return false;
	}

	//커널 이미지 베이스와 로드된 모듈주소와는 공간이 어느정도 비어 있다고 가정한다.
	//커널64의 이미지베이스 로드 주소는 0x200000이다.

	int pml4EntryAddress = 0x160000;

	//64커널 이미지 베이스 주소에 커널을 카피한다.
	memcpy((void*)imageBase, (void*)pModule->ModuleStart, ((int)pModule->ModuleEnd - (int)pModule->ModuleStart));

	InitializePageTables(pml4EntryAddress);
	SkyConsole::Print("Start %s!!\n", szKernelName);
	SwitchAndExecute64bitKernel(pml4EntryAddress, kernelEntry, (int)pBootInfo);

	return true;
}