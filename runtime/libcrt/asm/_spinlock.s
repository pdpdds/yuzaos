;x86-32 Spinlock Code
;
bits 32
segment .text

;Functions in this asm
global __spinlock_acquire
global __spinlock_test
global __spinlock_release

; int spinlock_acquire(spinlock_t *spinlock)
; We wait for the spinlock to become free
; then set value to 1 to mark it in use.
__spinlock_acquire:
	; Stack Frame
	push ebp
	mov ebp, esp
	
	; Save stuff
	push ebx

	; Get address of lock
	mov ebx, dword [ebp + 8]

	; Sanity
	test ebx, ebx
	je .gotlock

	; We use this to test
	mov eax, 1

	; Try to get lock
	.trylock:
	xchg dword [ebx], eax
	test eax, eax
	je .gotlock

	; Busy-wait loop
	.lockloop:
	pause
	cmp dword [ebx], 0
	jne .lockloop
	jmp .trylock

	.gotlock:
	; Release stack frame
	mov eax, 1
	pop ebx
	pop ebp
	ret

; int spinlock_test(spinlock_t *spinlock)
; This tests whether or not spinlock is
; set or not
__spinlock_test:
	; Stack Frame
	push ebp
	mov ebp, esp
	
	; Save stuff
	push ebx

	; Get address of lock
    mov eax, 0
	mov ebx, dword [ebp + 8]

	; Sanity
	test ebx, ebx
	je .end

	; We use this to test
	mov eax, 1

	; Try to get lock
	xchg dword [ebx], eax
	test eax, eax
	je .gotlock

	; nah, no lock for us
	mov eax, 0
	jmp .end

	.gotlock:
	; Release stack frame
	mov eax, 1

	.end:
	pop ebx
	pop ebp
	ret


; void spinlock_release(spinlock_t *spinlock)
; We set the spinlock to value 0
__spinlock_release:
	; Stack Frame
	push ebp
	mov ebp, esp

	; Save stuff
	push ebx

	; Get address of lock
	mov ebx, dword [ebp + 8]
	
	; Sanity
	test ebx, ebx
	je .done

	; Ok, we assume valid pointer, set it to 0
	mov dword [ebx], 0

	; Release stack frame
	.done:
	pop ebx
	pop ebp
	ret