#   libk_amd64.s
#   Contains some extremely basic libc functions for x86-64 using low-level optimizations.
#   Also implements some low-level functions, such as hardware rng reads, which can't be constexpr.
    .code64
    .section    .text
    .global memcmp
    .global memcpy
    .global memchr    
    .global __errno
    .global __rdseed
    .global get_flags    
    .type   memcmp,     @function
    .type   memcpy,     @function
    .type   memchr,     @function 
    .type   __errno,    @function
    .type   __rdseed,   @function
    .type   get_flags,  @function
memcmp:
    xorl    %eax,       %eax
    movq    %rdx,       %rcx
    jrcxz   1f
    movl    $-1,        %edx
    repz    cmpsb
    setg    %al    
    cmovll  %edx,       %eax
1:
    ret
    .size   memcmp,     .-memcmp
memcpy:
    movq    %rdi,       %rax
    movq    %rdx,       %rcx
    jrcxz   1f
    rep     movsb
1:
    ret
    .size   memcpy,     .-memcpy
memchr:
	movq	%rdx, 		%rcx
	movq	%rsi, 		%rax
	repne	scasb
	jne		1f
	leaq	-1(%rdi),	%rax
	jmp		2f
1:
	xorq	%rax,	    %rax
2:
	ret
	.size	memchr,		.-memchr
__errno:
    movq	$errno,		%rax
	ret
    .size   __errno,    .-__errno
__rdseed:
    rdseed      %rax
    cmovncq     %rdi,   %rax
    ret
    .size   __rdseed,   .-__rdseed
get_flags:
    pushfq
    popq        %rax
    ret
    .size     get_flags, .-get_flags 
    .section  .data
	.global	  __atexit_guard
    .type     __atexit_guard, @object
__atexit_guard:
    .byte     0
    .size     __atexit_guard, 1
errno:
	.long	  0
	.size	  errno,		  4
	.type	  errno,		  @object
