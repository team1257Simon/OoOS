    .code64
    .section    .text
    .global     __copy
    .global     __zero
    .type       __copy,     @function
    .type       __zero,     @function
__copy:
    movq        %rdx,       %rcx
    jrcxz       1f
    rep         movsb
1:  ret
    .size       __copy,     .-__copy
__zero:
    movq        %rsi,       %rcx
    xorl        %eax,       %eax
    jrcxz       2f
    rep         stosb
2:
    ret
    .size       __zero,     .-__zero
