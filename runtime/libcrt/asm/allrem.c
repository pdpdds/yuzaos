#include "windef.h"
#include <stdint.h>

void __declspec(naked) _allrem()
{
	/* *INDENT-OFF* */
	__asm {
		push        ebx
		push        edi
		xor edi, edi
		mov         eax, dword ptr[esp + 10h]
		or eax, eax
		jge         L1
		inc         edi
		mov         edx, dword ptr[esp + 0Ch]
		neg         eax
		neg         edx
		sbb         eax, 0
		mov         dword ptr[esp + 10h], eax
		mov         dword ptr[esp + 0Ch], edx
		L1 :
		mov         eax, dword ptr[esp + 18h]
			or eax, eax
			jge         L2
			mov         edx, dword ptr[esp + 14h]
			neg         eax
			neg         edx
			sbb         eax, 0
			mov         dword ptr[esp + 18h], eax
			mov         dword ptr[esp + 14h], edx
			L2 :
		or eax, eax
			jne         L3
			mov         ecx, dword ptr[esp + 14h]
			mov         eax, dword ptr[esp + 10h]
			xor edx, edx
			div         ecx
			mov         eax, dword ptr[esp + 0Ch]
			div         ecx
			mov         eax, edx
			xor edx, edx
			dec         edi
			jns         L4
			jmp         L8
			L3 :
		mov         ebx, eax
			mov         ecx, dword ptr[esp + 14h]
			mov         edx, dword ptr[esp + 10h]
			mov         eax, dword ptr[esp + 0Ch]
			L5 :
			shr         ebx, 1
			rcr         ecx, 1
			shr         edx, 1
			rcr         eax, 1
			or ebx, ebx
			jne         L5
			div         ecx
			mov         ecx, eax
			mul         dword ptr[esp + 18h]
			xchg        eax, ecx
			mul         dword ptr[esp + 14h]
			add         edx, ecx
			jb          L6
			cmp         edx, dword ptr[esp + 10h]
			ja          L6
			jb          L7
			cmp         eax, dword ptr[esp + 0Ch]
			jbe         L7
			L6 :
		sub         eax, dword ptr[esp + 14h]
			sbb         edx, dword ptr[esp + 18h]
			L7 :
			sub         eax, dword ptr[esp + 0Ch]
			sbb         edx, dword ptr[esp + 10h]
			dec         edi
			jns         L8
			L4 :
		neg         edx
			neg         eax
			sbb         edx, 0
			L8 :
			pop         edi
			pop         ebx
			ret         10h
	}
	/* *INDENT-ON* */
}