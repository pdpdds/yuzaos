
void __declspec(naked) _alldiv()
{
	/* *INDENT-OFF* */
	__asm {
		push        edi
		push        esi
		push        ebx
		xor edi, edi
		mov         eax, dword ptr[esp + 14h]
		or eax, eax
		jge         L1
		inc         edi
		mov         edx, dword ptr[esp + 10h]
		neg         eax
		neg         edx
		sbb         eax, 0
		mov         dword ptr[esp + 14h], eax
		mov         dword ptr[esp + 10h], edx
		L1 :
		mov         eax, dword ptr[esp + 1Ch]
			or eax, eax
			jge         L2
			inc         edi
			mov         edx, dword ptr[esp + 18h]
			neg         eax
			neg         edx
			sbb         eax, 0
			mov         dword ptr[esp + 1Ch], eax
			mov         dword ptr[esp + 18h], edx
			L2 :
		or eax, eax
			jne         L3
			mov         ecx, dword ptr[esp + 18h]
			mov         eax, dword ptr[esp + 14h]
			xor edx, edx
			div         ecx
			mov         ebx, eax
			mov         eax, dword ptr[esp + 10h]
			div         ecx
			mov         edx, ebx
			jmp         L4
			L3 :
		mov         ebx, eax
			mov         ecx, dword ptr[esp + 18h]
			mov         edx, dword ptr[esp + 14h]
			mov         eax, dword ptr[esp + 10h]
			L5 :
			shr         ebx, 1
			rcr         ecx, 1
			shr         edx, 1
			rcr         eax, 1
			or ebx, ebx
			jne         L5
			div         ecx
			mov         esi, eax
			mul         dword ptr[esp + 1Ch]
			mov         ecx, eax
			mov         eax, dword ptr[esp + 18h]
			mul         esi
			add         edx, ecx
			jb          L6
			cmp         edx, dword ptr[esp + 14h]
			ja          L6
			jb          L7
			cmp         eax, dword ptr[esp + 10h]
			jbe         L7
			L6 :
		dec         esi
			L7 :
		xor edx, edx
			mov         eax, esi
			L4 :
		dec         edi
			jne         L8
			neg         edx
			neg         eax
			sbb         edx, 0
			L8 :
			pop         ebx
			pop         esi
			pop         edi
			ret         10h
	}
	/* *INDENT-ON* */
}