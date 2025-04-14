//   libk_amd64.S
//   Contains some extremely basic libc functions for x86-64 using low-level optimizations for size (and, somewhat, for speed as well).
//   Also implements some low-level functions, such as hardware rng reads, which can't be constexpr.
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
//   This sort of feels hacky, since we shaved off every possible byte using conditional set and conditional move in addition to the string instruction.
//   I don't particularly mind, though, since it ends up being another stupidly short routine.
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
//   This is just about the smallest implementation you can have for a memcpy on amd64 at 11 bytes assembled. 
//   It's also quite fast, beating out anything that doesn't use speculative fetching.
//   Funnily enough, it would be only 8 bytes if we didn't have to move the size argument into the C register.
//   The ABI did us dirty there, because it would be pretty hilarious to have a C standard function whose code fits in a general-purpose register.
memcpy:
    movq    %rdi,       %rax
    movq    %rdx,       %rcx
    jrcxz   1f
    rep     movsb
1:
    ret
    .size   memcpy,     .-memcpy
//   The scas implementation of memchr saves less code than the movs implementation of memcpy, but it's still fast.
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
