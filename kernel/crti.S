	.global 	_init
	.global 	_fini
	.global		kernel_pages
	.extern		__kernel_pages
	.extern		__start
	.extern		__end
	.global		kernel_stack_base
	.global		kernel_stack_top
	.type 		_init,				@function
	.type 		_fini, 				@function
	.type		kernel_pages,		@object
	.type		__kernel_pages,		@object
	.type		__start,			@object
	.type		__end,				@object
	.type		kernel_stack_base,	@object
	.type		kernel_stack_top,	@object
	.section 	.init
_init:
	push 		%rbp
	movq 		%rsp, 	%rbp
	.section 	.fini
_fini:
	push 		%rbp
	movq 		%rsp, 	%rbp
	.section	.rodata
kernel_pages:
	.quad		__kernel_pages
	.size		kernel_pages,	.-kernel_pages
	.section	.data
	.align		4096
kernel_stack_base:
	.zero		65536
kernel_stack_top:
	.size		kernel_stack_base,	.-kernel_stack_base
	.size		kernel_stack_top,	.-kernel_stack_top
