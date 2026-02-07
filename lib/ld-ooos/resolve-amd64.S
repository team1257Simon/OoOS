	.code64
	.section	.text
	.global		resolve
	.hidden		resolve
	.type		resolve,	@function
resolve:
	xchgq	8(%rsp),		%rdi
	xchgq	(%rsp),			%rsi
	pushq	%rcx
	movq	$29,			%rax
	syscall
	cmpq	$-4096,			%rax
	ja		1f
	popq	%rcx
	popq	%rsi
	popq	%rdi
	jmp		*%rax
1:
	movq	$6,				last_error_action(%rip)
	negq	%rax
	movq	%rax,			%rdi
	xorl	%eax,			%eax
	syscall
	.size	resolve,		.-resolve
