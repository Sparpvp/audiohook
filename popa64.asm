.DATA

.CODE

__popa_64 PROC
	add rsp, 108h
	pop r15		; push/pop r15...r8 length is 2 byte
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9		
	pop r8
	pop rdi		; push/pop rdi..rax length is 1 byte
	pop rsi
	pop rbp
	pop rbx
	pop rdx
	pop rcx
	pop rax

__popa_64 ENDP

END