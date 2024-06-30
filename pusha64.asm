.DATA

.CODE

__pusha_64 PROC
	push rax	; push/pop rdi..rax length is 1 byte
	push rcx
	push rdx
	push rbx
	push rbp
	push rsi
	push rdi
	push r8		; push/pop r15...r8 length is 2 byte
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	sub rsp, 100h

__pusha_64 ENDP

END