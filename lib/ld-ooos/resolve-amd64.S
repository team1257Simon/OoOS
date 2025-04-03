    .code64
    .section    .text
    .global     resolve
    .type       resolve,        @function
resolve:
    xchgq   8(%rsp),        %rdi
    xchgq   (%rsp),         %rsi
    pushq   %rcx
    movq    $28,            %rax
    syscall
    cmpq    $0,             %rax
    jge     1f
    cmpq    $-4096,         %rax
    jng     1f
    negq    %rax
    movq    %rax,           %rdi
    xorq    %rax,           %rax
    syscall
1:
    popq    %rcx
    popq    %rsi
    popq    %rdi
    jmp     *%rax
    .size       resolve,        .-resolve