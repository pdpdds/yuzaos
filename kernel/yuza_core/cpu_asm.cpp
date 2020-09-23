#include "cpu_asm.h"
#include <string>
#include "Thread.h"
#include "Team.h"
#include "AddressSpace.h"
#include "physicalmap.h"
#include <BuildOption.h>

void read_tsc(int *cl_Low, int *cl_High)
{
	__asm {
		_emit 0Fh
		_emit 31h
		mov ecx, dword ptr cl_Low
		mov[ecx], eax
		mov ecx, dword ptr cl_High
		mov[ecx], edx
	}
}

/// Return the physical address of the current page directory
unsigned int GetCurrentPageDir()
{
#if SKY_EMULATOR

	if (Thread::GetRunningThread() == 0 || Thread::GetRunningThread()->GetTeam() == 0)
	{
		return 0;
	}

	Team* pTeam = Thread::GetRunningThread()->GetTeam();
	return pTeam->GetAddressSpace()->GetPhysicalMap()->GetPageDir();
#endif 

	unsigned int val;
	__asm
	{
		mov eax, cr3
		mov val, eax
	}

	return val;
}