    .code64
    .text
    .global __fill_entropy
    .type   __fill_entropy,     @function
__fill_entropy:
    movq    %rsi,               %rcx
    jrcxz   3f
1:
    rdseed  %rax
    jnc     1b
    stosq
    loop    1b
3:
    movq    %rdi,               %rax
    ret
    .size   __fill_entropy,     .-__fill_entropy
