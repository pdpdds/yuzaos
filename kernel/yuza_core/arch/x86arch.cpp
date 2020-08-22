#include <x86arch.h>
#include <stringdef.h>

void LoadPDBR(UINT32 pa)
{
	_asm
	{
		mov	eax, [pa]
		mov	cr3, eax		// PDBR is cr3 register in i86
	}
}

UINT32 GetPDBR()
{
	_asm
	{
		mov	eax, cr3
		ret
	}
}


void EnablePaging(bool state)
{
	_asm
	{
		mov	eax, cr0
		cmp[state], 1
		je	enable
		jmp disable

	enable :
		or eax, 0x80000000		//set bit 31
		mov	cr0, eax
		jmp done
		
	disable :		
		and eax, 0x7FFFFFFF		//clear bit 31
		mov	cr0, eax
	done :
	}
}

bool IsPaging()
{
	UINT32 res = 0;

	_asm 
	{
		mov	eax, cr0
		mov[res], eax
	}

	return (res & 0x80000000) ? false : true;
}

void FlushTranslationLockBufferEntry(UINT32 addr)
{
#ifdef _MSC_VER
	_asm {
		cli
		invlpg	addr
		sti
	}
#endif
}

void SetPageDirectory(UINT32 dir)
{
	_asm
	{
		mov	eax, [dir]
		mov	cr3, eax		// PDBR is cr3 register in i86
	}
}