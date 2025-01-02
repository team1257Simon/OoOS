    .code64
    .section    .text
    .altmacro
    .macro  defglobal   name
    .global     \name
    .type       \name,    @function
\name:
    .endm
    defglobal   memcmp
    xorl    %eax,   %eax
    movq    %rdx,   %rcx
    jrcxz   .L0
    movl    $-1,    %edx
    repz    cmpsb
    cmovll  %edx,   %eax
    setg    %al
.L0:
    ret
    .size   memcmp, .-memcmp
    defglobal   memcpy
    movq    %rdi,   %rax
    movq    %rdx,   %rcx
    jrcxz   .L0
    rep     movsb
    ret
    .size   memcpy, .-memcpy
    defglobal   memchr
	movq	%rdx, 		%rcx
	movq	%rsi, 		%rax
	repne	scasb
	jne		.L1
	leaq	-1(%rdi),	%rax
	jmp		.L2
.L1:
	xorq	%rax,	%rax
.L2:
	ret
	.size	memchr,		.-memchr
    defglobal tlb_flush
    movq    %cr3,   %rax
    movq    %rax,   %cr3
    ret
    .size   tlb_flush,    .-tlb_flush
    defglobal set_cr3
    movq    %rdi,   %rax
    movq    %rax,   %cr3
    ret
    .size     set_cr3,     .-set_cr3
    defglobal get_cr3
    movq    %cr3,   %rax
    ret
    .size     get_cr3,     .-get_cr3
    defglobal __errno
    movq	$errno,		%rax
	ret
    .size     __errno,    .-__errno
    .section    .data
	.global		__atexit_guard
    .type       __atexit_guard,     @object
__atexit_guard:
    .byte 0
    .size       __atexit_guard, 1
errno:
	.long	0
	.size		errno,			4
	.type		errno,			@object
