    .global     _start
    .extern     _init
    .extern     _fini
    .extern     module_instance
    .global     __module_frame_tag
    .global     __frame_functions
    .type       _start,                 @function
    .type       _init,                  @function
    .type       _fini,                  @function
    .type       module_instance,        @function
    .type       __module_frame_tag,     @object
    .type       __frame_functions,      @object
    .text
_start:
    pushq   %rbp
    movq    %rsp,   %rbp
    pushq   %rbx
    movq    %rdi,   %rbx
    call    module_instance(%rip)
    movq    %rax,   %rdi
    movq    %rbx,   %rax
    movq    %rdi,   %rbx
    leaq    __module_frame_tag(%rip),   %rsi
    leaq    __frame_functions(%rip),    %rdx
    leaq    _fini(%rip),                %rcx
    call    *%rax
    test    %ax,                        %ax
    jz      1f
    call    _init(%rip)
    movq    %rbx,                       %rax
    jmp     2f
1:
    xorq    %rax,                       %rax
2:
    popq    %rbx
    leave
    ret
    .size   _start,                     .-_start
    .data
__module_frame_tag:
    .quad 0
    .size   __module_frame_tag,         8
__frame_functions:
    .zero 64
    .size   __frame_functions,          64