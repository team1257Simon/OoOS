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
    defglobal   abset
    movq    %rsi,   %rax
    movq    %rdx,   %rcx
    jrcxz   .L1
    rep     stosb
.L1:
    ret
    .size   abset, .-abset
    defglobal    awset
    movq    %rsi,   %rax
    movq    %rdx,   %rcx
    jrcxz   .L2
    rep     stosw
.L2:
    ret
    .size   awset, .-awset
    defglobal   alset
    movq    %rsi,   %rax
    movq    %rdx,   %rcx
    jrcxz   .L3
    rep     stosl
.L3:
    ret
    .size   alset, .-alset
    defglobal   aqset
    movq    %rsi,   %rax
    movq    %rdx,   %rcx
    jrcxz   .L4
    rep     stosq
.L4:
    ret
    .size   aqset, .-aqset
    defglobal   sbcopy
    movq    %rdx,   %rcx
    jrcxz   .L5
    rep     movsb
.L5:
    ret
    .size   sbcopy, .-sbcopy
    defglobal   swcopy
    movq    %rdx,   %rcx
    jrcxz   .L6
    rep     movsw
.L6:
    ret
    .size   swcopy, .-swcopy
    defglobal   slcopy
    movq    %rdx,   %rcx
    jrcxz   .L7
    rep     movsl
.L7:
    ret
    .size   slcopy, .-slcopy
    defglobal   sqcopy
    movq    %rdx,   %rcx
    jrcxz   .L8
    rep     movsq
.L8:
    ret
    .size   sqcopy, .-sqcopy
    defglobal   strlen
    xorq    %rsi,   %rsi
    defglobal   strnlen
    movq    %rsi,   %rcx
    xorq    %rax,   %rax
    movq    %rdi,   %rdx
    repnz   scasb
    movq    %rdi,   %rax
    subq    %rdx,   %rax
    ret
    .size   strlen,  .-strlen
    .size   strnlen, .-strnlen
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
	jrcxz	.L9
	movq	%rsi, 		%rax
	repne	scasb
	jne		.L9
	leaq	-1(%rdi),	%rax
	jmp		.L10
.L9:
	xorq	%rax,	%rax
.L10:
	ret
	.size	strchr, 	.-strchr
    .size   strnchr,    .-strnchr
	.size	memchr,		.-memchr
    defglobal   memcpy
    movq    %rdi,   %rax
    jmp     sbcopy
    .size   memcpy,     .-memcpy
    defglobal   stpcpy
    pushq   $1
    jmp     .L11
    defglobal   strcpy
    pushq   $0
.L11:
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
    jrcxz   .L13
    movq    %rdi,   %r8
.L12:
    lodsb
    stosb
    testq   %rax,   %rax
    loopnz  .L12
    testq   %r9,    %r9
    cmovnz  %rdi,   %r8
    movq    %r8,    %rax
.L13:
    ret
    .size   strcpy,     .-strcpy
    .size   strncpy,    .-strncpy
    .size   stpcpy,     .-stpcpy
