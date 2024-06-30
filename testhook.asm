.DATA

.CODE

__test_hook PROC
	mov rax, 0DEADC0DEh
	mov rcx, 5
	mov rdx, 5
	mov r8, 6
	mov r9, 6
	mov r10, 6
	mov r11, 7
	ret

__test_hook ENDP

END