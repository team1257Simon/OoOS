    .global     _module_load
    .extern     start
    .type       _module_load,   @function
    .type       start,          @function
    .text
_module_load:
    pushq   %rbp
    movq    %rsp,                       %rbp
    movq    %rdi,                       %rax
    leaq    __module_frame_tag(%rip),   %rdi
    leaq    __frame_functions(%rip),    %rsi
    call    *%rax
    leave
    jmp     start
    .size   _module_load,               .-_module_load
