	.code64
	.section    .text
	.global     __copy
	.global     __zero
	.global     __strterm
	.hidden     __copy
	.hidden     __zero
	.hidden     __strterm
	.type       __copy,     @function
	.type       __zero,     @function
	.type       __strterm,  @function
__copy:
	movq        %rdx,       %rcx
	jrcxz       1f
	rep         movsb
1:  ret
	.size       __copy,     .-__copy
__zero:
	movq        %rsi,       %rcx
	jrcxz       2f
	xorl        %eax,       %eax
	rep         stosb
2:  ret
	.size       __zero,     .-__zero
__strterm:
	movq        $256,       %rcx
	xorl        %eax,       %eax
	repne       scasb
	movq        %rdi,       %rax
	ret
	.size       __strterm,  .-__strterm
