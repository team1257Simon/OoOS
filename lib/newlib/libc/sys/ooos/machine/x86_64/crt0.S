#ifdef DYNAMIC
    .code64
    .extern     environ
    .type       environ,                    @object
    .extern     main
    .type       main,                       @function
    .section    .text
    .global     _start
    .type       _start,                     @function
_start:
    leaq        environ@GOTPCREL(%rip),     %rax
    movq        %rdx,                       (%rax)
    pushq       %rbp
    movq        %rsp,                       %rbp
    call        main
    movq        %rax,                       %rdi
    jmp         exit
    .size       _start,                     .-_start
#else
    .code64
    .extern     environ
    .type       environ,        @object
    .extern     main
    .type       main,           @function
    .section    .text
    .global     _start
    .type       _start,     @function
_start:
    movq        %rdx,       environ
    pushq       %rbp
    movq        %rsp,       %rbp
    pushq       %rdi
    pushq       %rsi
    pushq       %rdx
    call        _init
    popq        %rdx    
    popq        %rsi
    popq        %rdi
    call        main
    movq        %rax,       %rdi
    jmp         exit
    .size       _start,     .-_start
#endif
