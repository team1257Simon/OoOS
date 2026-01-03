	.global	_start
	.extern	kernel_stack_top
	.extern	kmain
	.global		kernel_pages
	.extern		__kernel_pages
	.extern		__start
	.extern		__end
	.extern		__plocal_data_size
	.global		kernel_stack_base
	.global		kernel_stack_top
	.extern		processor_local_data
	.global		processor_local_data_size
	.type		_start,						@function
	.type		kmain,						@function
	.type		kernel_stack_top,			@object
	.type		kernel_pages,				@object
	.type		__kernel_pages,				@object
	.type		__start,					@object
	.type		__end,						@object
	.type		kernel_stack_base,			@object
	.type		kernel_stack_top,			@object
	.type		processor_local_data,		@object
	.type		processor_local_data_size,	@object
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
	.data
	.section .rodata
processor_local_data_size:
	.quad		__plocal_data_size
	.size		processor_local_data_size,	.-processor_local_data_size
kernel_pages:
	.quad		__kernel_pages
	.size		kernel_pages,	.-kernel_pages
	.section	.data.plocal
	.align		4096
kernel_stack_base:
	.zero		65536
kernel_stack_top:
	.size		kernel_stack_base,	.-kernel_stack_base
	.size		kernel_stack_top,	.-kernel_stack_top
