; x86-32 Syscall Assembly Routine

bits 32
segment .text

;Functions in this asm
global __syscall

; int _syscall(int Function, int Arg0, int Arg1, int Arg2, int Arg3, int Arg4)
__syscall:
	; Stack Frame
	push ebp
	mov ebp, esp

	; Save
	push ebx
	push ecx
	push edx
	push esi
	push edi

	; Get params
	mov eax, [ebp + 8]
	mov ebx, [ebp + 12]
	mov ecx, [ebp + 16]
	mov edx, [ebp + 20]
	mov esi, [ebp + 24]
	mov edi, [ebp + 28]

	; Syscall
	int	80h

	; Restore
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx

	; Release stack frame
	pop ebp
	ret 