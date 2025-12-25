	.global	_start
	.extern	kernel_stack_top
	.extern	kmain
	.type	_start,					@function
	.type	kernel_stack_top,		@object
	.type	kmain,					@function
	.text
	// Very brief prolog for kmain; sets up the kernel stack before jumping there.
	// Most of the normal crt0 code, like calling global constructors, is handled in kmain.
	// Since there are global constructors that depend on the kernel heap allocator, that needs to be initialized first.
_start:
	leaq	kernel_stack_top,		%rsp
	leaq	-8(%rsp),				%rbp
	pushq	%rsp
	jmp		kmain
	.size	_start,					.-_start
