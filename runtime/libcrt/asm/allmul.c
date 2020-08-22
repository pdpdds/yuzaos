void
__declspec(naked)
_allmul()
{
	/* *INDENT-OFF* */
	__asm {
		mov         eax, dword ptr[esp + 8]
		mov         ecx, dword ptr[esp + 10h]
		or ecx, eax
		mov         ecx, dword ptr[esp + 0Ch]
		jne         hard
		mov         eax, dword ptr[esp + 4]
		mul         ecx
		ret         10h
		hard :
		push        ebx
			mul         ecx
			mov         ebx, eax
			mov         eax, dword ptr[esp + 8]
			mul         dword ptr[esp + 14h]
			add         ebx, eax
			mov         eax, dword ptr[esp + 8]
			mul         ecx
			add         edx, ebx
			pop         ebx
			ret         10h
	}
	/* *INDENT-ON* */
}

