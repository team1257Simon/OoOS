	.global	_start
	.extern	_init
	.extern	_fini
	.extern	module_init
	.global	__module_frame_tag
	.global	__frame_functions
	.type		_start,				@function
	.type		_init,				@function
	.type		_fini,				@function
	.type		module_init,		@function
	.type		__module_frame_tag,	@object
	.type		__frame_functions,	@object
	.text
_start:
	pushq	%rbp
	movq	%rsp,						%rbp
	test	%rdi,						%rdi
	jz		1f
	leaq	__module_frame_tag(%rip),	%rsi
	leaq	__frame_functions(%rip),	%rdx
	leaq	_init(%rip),				%rcx
	leaq	_fini(%rip),				%r8
	call	module_init
	jmp	2f
1:
	xorq	%rax,						%rax
2:
	leave
	ret
	.size	_start,						.-_start
	.data
__module_frame_tag:
	.quad	0
	.size	__module_frame_tag,			8
__frame_functions:
	.zero	64
	.size	__frame_functions,			64