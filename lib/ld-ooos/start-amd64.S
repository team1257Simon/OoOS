    .code64
    .section    .text
    .global     _start
    .type       _start,     @function
    .type       _end,       @function
_start:
    pushq       %rbp
    movq        %rsp,       %rbp
    pushq       %r8
    pushq       %rsi
    pushq       %rdx
    call        dlbegin
    testq       %rax,       %rax
    jnz         1f
    popq        %rdx
    popq        %rsi
    popq        %rdi
    popq        %rbp
    movq        %rbx,       %rax
    leaq        _end(%rip), %rbx
    jmp         *%rax
    .size       _start,     .-_start
_end:
    call        dlend
1:
    movq        %rax,       %rdi
    xorq        %rax,       %rax
    syscall
    .size       _end,       .-_end    
