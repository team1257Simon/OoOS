//   libk_amd64.S
//   Contains some extremely basic libc functions for x86-64 using low-level optimizations.
//   Also implements some low-level functions, such as hardware rng reads.
    .code64
    .section    .text
    .global memcmp
    .global memcpy
    .global memset
    .global memchr    
    .global __errno
    .global __rdseed
    .global get_flags    
    .type   memcmp,     @function
    .type   memcpy,     @function
    .type   memset,     @function
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
//   Memcpy and memset taken from newlib, under the same license as in the legal.md
memcpy:
    movq %rdi,          %rax
    cmpq $16,           %rdx
    jb 3f
    movq %rdi,          %r8
    andq $7,            %r8
    jz 1f
    movq $8,            %rcx
    subq %r8,           %rcx
    subq %rcx,          %rdx
    rep movsb
1:
    cmpq $256,          %rdx
    jb 4f
    pushq %rax
    pushq %r12
    pushq %r13
    pushq %r14
    movq %rdx,          %rcx
    shrq $7,            %rcx
    .p2align            4
2:
    prefetchnta         768(%rsi)
    prefetchnta         832(%rsi)
    movq (%rsi),        %rax
    movq 8(%rsi),       %r8
    movq 16(%rsi),      %r9
    movq 24(%rsi),      %r10
    movq 32(%rsi),      %r11
    movq 40(%rsi),      %r12
    movq 48(%rsi),      %r13
    movq 56(%rsi),      %r14
    movntiq %rax,       (%rdi)
    movntiq %r8,        8(%rdi)
    movntiq %r9,        16(%rdi)
    movntiq %r10,       24(%rdi)
    movntiq %r11,       32(%rdi)
    movntiq %r12,       40(%rdi)
    movntiq %r13,       48(%rdi)
    movntiq %r14,       56(%rdi)
    movq 64(%rsi),      %rax
    movq 72(%rsi),      %r8
    movq 80(%rsi),      %r9
    movq 88(%rsi),      %r10
    movq 96(%rsi),      %r11
    movq 104(%rsi),     %r12
    movq 112(%rsi),     %r13
    movq 120(%rsi),     %r14
    movntiq %rax,       64(%rdi)
    movntiq %r8,        72(%rdi)
    movntiq %r9,        80(%rdi)
    movntiq %r10,       88(%rdi)
    movntiq %r11,       96(%rdi)
    movntiq %r12,       104(%rdi)
    movntiq %r13,       112(%rdi)
    movntiq %r14,       120(%rdi)
    leaq 128(%rsi),     %rsi
    leaq 128(%rdi),     %rdi
    dec  %rcx
    jnz 2b
    sfence
    movq %rdx,          %rcx
    andq $127,          %rcx
    rep  movsb
    popq %r14
    popq %r13
    popq %r12
    popq %rax
    ret
3:
    movq %rdx,          %rcx
    rep  movsb
    ret
4:
    movq %rdx,          %rcx
    shrq $3,            %rcx
    .p2align            4
    rep  movsq
    movq %rdx,          %rcx
    andq $7,            %rcx
    rep  movsb
    ret
    .size memcpy,       .-memcpy
memset:
    movq %rdi,          %r9
    movq %rsi,          %rax
    movq %rdx,          %rcx
    cmpq $16,           %rdx
    jb  3f
    movq %rdi,          %r8
    andq $7,            %r8
    jz  1f
    movq $8,            %rcx
    subq %r8,           %rcx
    subq %rcx,          %rdx
    rep  stosb
    movq %rdx,          %rcx
1:
    movabs $0x0101010101010101, %r8
    movzbl %sil,                %eax
    imul   %r8,                 %rax
    cmpq   $256,                %rdx
    jb     4f
    shrq   $7,                  %rcx
    .p2align                    4
2:
    movntiq %rax,               (%rdi)
    movntiq %rax,               8(%rdi)
    movntiq %rax,               16(%rdi)
    movntiq %rax,               24(%rdi)
    movntiq %rax,               32(%rdi)
    movntiq %rax,               40(%rdi)
    movntiq %rax,               48(%rdi)
    movntiq %rax,               56(%rdi)
    movntiq %rax,               64(%rdi)
    movntiq %rax,               72(%rdi)
    movntiq %rax,               80(%rdi)
    movntiq %rax,               88(%rdi)
    movntiq %rax,               96(%rdi)
    movntiq %rax,               104(%rdi)
    movntiq %rax,               112(%rdi)
    movntiq %rax,               120(%rdi)
    leaq    128(%rdi),          %rdi
    dec     %rcx
    jnz     2b
    sfence
    movq    %rdx,               %rcx
    andq    $127,               %rcx
    rep     stosb
    movq    %r9,                %rax
    ret
3:
    rep     stosb
    movq    %r9,                %rax
    ret
4:
    shrq    $3,                 %rcx
    .p2align 4
    rep     stosq
    movq    %rdx,               %rcx
    andq    $7,                 %rcx
    rep     stosb
    movq    %r9,                %rax
    ret
    .size memset,               .-memset
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
    .data
	.global	  __atexit_guard
    .type     __atexit_guard, @object
__atexit_guard:
    .byte     0
    .size     __atexit_guard, 1
errno:
	.long	  0
	.size	  errno,		  4
	.type	  errno,		  @object
