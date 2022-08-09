.code

extern is_reloc:proc
vm_is_reloc PROC
	
	push rax
	push rcx
	push rbx
	push rdx
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov rcx, rsp
	pushfq

	sub rsp, 20h
	call is_reloc
	add rsp, 20h

	popfq

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rdx
	pop rbx
	pop rcx
	pop rax

	ret

vm_is_reloc ENDP
END
