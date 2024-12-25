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
    defglobal   memset
    movq    %rsi,   %rax
    movq    %rdx,   %rcx
    jrcxz   .L0
    rep     stosb
    ret
    .size   memset, .-memset
    defglobal   memcpy
    movq    %rdi,   %rax
    defglobal   sbcopy
    movq    %rdx,   %rcx
    jrcxz   .L0
    rep     movsb
    ret
    .size   sbcopy, .-sbcopy
    .size   memcpy, .-memcpy
    defglobal   swcopy
    movq    %rdx,   %rcx
    jrcxz   .L0
    rep     movsw
    ret
    .size   swcopy, .-swcopy
    defglobal   slcopy
    movq    %rdx,   %rcx
    jrcxz   .L0
    rep     movsl
    ret
    .size   slcopy, .-slcopy
    defglobal   sqcopy
    movq    %rdx,   %rcx
    jrcxz   .L0
    rep     movsq
    ret
    .size   sqcopy, .-sqcopy
    defglobal   strcmp
    xorq    %rdx,   %rdx
    defglobal   strncmp
    pushq   %rdi
    pushq   %rsi
    movq    %rdx,   %rsi
    call    strnlen
    movq    %rax,   %rdx
    popq    %rsi
    popq    %rdi
    jmp     memcmp
    .size   strcmp,     .-strcmp
    .size   strncmp,    .-strncmp
    defglobal   strchr
    xorq    %rdx,   %rdx
    defglobal   strnchr
	pushq	%rsi
    pushq	%rdi
    movq    %rdx,   %rsi
	call	strnlen
	movq	%rax, 	%rdx
	popq	%rdi
	popq	%rsi
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
	.size	strchr, 	.-strchr
    .size   strnchr,    .-strnchr
	.size	memchr,		.-memchr
    defglobal   stpcpy
    pushq   $1
    jmp     .L3
    defglobal   strcpy
    pushq   $0
.L3:
    pushq   %rdi
    pushq   %rsi
    movq    %rdi,   %rsi
    call    strlen
    movq    %rax,   %rdx
    popq    %rsi
    popq    %rdi
    popq    %r9
    defglobal   strncpy
    movq    %rdx,   %rcx
    jrcxz   .L2
    movq    %rdi,   %r8
.L4:
    lodsb
    stosb
    testq   %rax,   %rax
    loopnz  .L4
    testq   %r9,    %r9
    cmovnz  %rdi,   %r8
    movq    %r8,    %rax
    ret
    .size   strcpy,     .-strcpy
    .size   strncpy,    .-strncpy
    .size   stpcpy,     .-stpcpy
    defglobal   strlen
    xorq    %rsi,   %rsi
    defglobal   strnlen
    movq    %rsi,   %rcx
    xorq    %rax,   %rax
    xorq    %rdx,   %rdx
.L5:
    scasb
    jz      .L6
    incq    %rdx
    loop    .L5
.L6:
    movq    %rdx,   %rax
    ret
    .size   strlen,  .-strlen
    .size   strnlen, .-strnlen
    defglobal acquire
    orb         $1,     %al
    lock xchgb  (%rdi), %al
    movzbq      %al,    %rax
    ret
    .size   acquire,    .-acquire
    defglobal release
    xorq        %rax,   %rax
    lock xchgb  (%rdi), %al
    ret
    .size   release,    .-release
    defglobal testlock
    xorq    %rax,  %rax
    orb    (%rdi), %al
    ret
    .size   testlock,   .-testlock
    defglobal __sysinternal_tlb_flush
    movq    %cr3,   %rax
    movq    %rax,   %cr3
    ret
    .size   __sysinternal_tlb_flush,    .-__sysinternal_tlb_flush
    defglobal __sysinternal_set_cr3
    movq    %rdi,   %rax
    movq    %rax,   %cr3
    ret
    .size     __sysinternal_set_cr3,     .-__sysinternal_set_cr3
    defglobal __sysinternal_get_cr3
    movq    %cr3,   %rax
    ret
    .size     __sysinternal_get_cr3,     .-__sysinternal_get_cr3
