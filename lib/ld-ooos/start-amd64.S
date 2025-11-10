	.code64
	.section	.text
	.global		_start
	.hidden		_start
	.type		_start,		@function
_start:
	pushq		%rbp
	movq		%rsp,		%rbp
	pushq		%rsi
	pushq		%rdx
	call		dlbegin
	testq		%rax,		%rax
	jnz			1f
	popq		%rdx
	popq		%rsi
	movq		argc(%rip),	%rdi
	popq		%rbp
	movq		%rbx,		%rax
	jmp			*%rax
1:
	movq		%rax,		%rdi
	xorq		%rax,		%rax
	syscall
	.size		_start,		.-_start
