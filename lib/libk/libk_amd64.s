#   libk_amd64.s
#   Contains some extremely basic libc functions for x86-64 using low-level optimizations.
#   Also implements some low-level functions, such as hardware rng reads, which can't be constexpr.
    .code64
    .section    .text
    .altmacro
    .macro  defglobal   name
    .global     \name
    .type       \name,    @function
\name:
    .endm
    defglobal   memcmp
    xorl    %eax,       %eax
    movq    %rdx,       %rcx
    jrcxz   .L0
    movl    $-1,        %edx
    repz    cmpsb
    cmovll  %edx,       %eax
    setg    %al
.L0:
    ret
    .size       memcmp, .-memcmp
    defglobal   memcpy
    movq    %rdi,       %rax
    movq    %rdx,       %rcx
    jrcxz   .L0
    rep     movsb
    mfence
    ret
    .size       memcpy, .-memcpy
    defglobal   memchr
	movq	%rdx, 		%rcx
	movq	%rsi, 		%rax
	repne	scasb
	jne		.L1
	leaq	-1(%rdi),	%rax
	jmp		.L2
.L1:
	xorq	%rax,	    %rax
.L2:
	ret
	.size	    memchr,		.-memchr
    defglobal   __errno
    movq	$errno,		%rax
	ret
    .size       __errno,    .-__errno
    defglobal   __rdseed
    rdseed      %rax
    cmovncq     %rdi,   %rax
    ret
    .size     __rdseed,   .-__rdseed
    defglobal set_gs_base
    wrgsbase    %rdi
    ret
    .size     set_gs_base, .-set_gs_base
    defglobal set_fs_base
    wrfsbase    %rdi
    ret
    .size     set_fs_base, .-set_fs_base
    defglobal get_gs_base
    rdgsbase    %rax
    ret
    .size     get_gs_base, .-get_gs_base
    defglobal get_fs_base
    rdfsbase    %rax
    ret
    .size     get_fs_base, .-get_fs_base
    defglobal get_flags
    pushfq
    popq        %rax
    ret
    .size     get_flags,   .-get_flags
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
