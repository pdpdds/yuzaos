bits 32
segment .text

;Functions in this asm
global _sqrt
global _sqrtf
global _sqrtl
global __CIsqrt

; Computes the square-root
_sqrt:
	push	ebp
	mov		ebp, esp
	fld		qword [ebp + 8]
	fsqrt
	pop     ebp
	ret

; Computes the square-root
_sqrtf:
	push	ebp
	mov		ebp, esp
	fld		dword [ebp + 8]
	fsqrt
	pop     ebp
	ret

; Computes the square-root
_sqrtl:
	push	ebp
	mov		ebp, esp
	fld		tword [ebp + 8]
	fsqrt
	pop     ebp
	ret

; Msvc version of sqrt
__CIsqrt:
	fsqrt
	ret