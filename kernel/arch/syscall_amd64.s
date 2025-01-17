    .code64
    .section .data
    .global syscall_vec
    .type   syscall_vec,    @object
syscall_vec:
    .quad syscall_open
    .quad syscall_close
    .quad syscall_read
    .quad syscall_write
    .quad syscall_link
    .quad syscall_unlink
    .quad syscall_isatty
    # more to come...
    .size   syscall_vec,    .-syscall_vec
    .section .text
    .global do_syscall
    .type   do_syscall,     @function
do_syscall:
    cli
    swapgs
    movq    %rsp,                   %gs:0x20
    movq    %rbp,                   %gs:0x28
    movq    %rcx,                   %gs:0x30
    movq    %cr3,                   %rcx
    movq    %rcx,                   %gs:0x38
    movq    %gs:0x08,               %rcx
    movq    %rcx,                   %cr3
    movq    %gs:0x10,               %rsp
    movq    %gs:0x18,               %rbp
    leaq    syscall_vec(,%rax,8),   %rax
    sti
    call    *%rax
    cli
    movq    %gs:0x28,               %rbp
    movq    %gs:0x20,               %rsp
    movq    %gs:0x30,               %rdi 
    movq    %gs:0x38,               %rcx
    movq    %rcx,                   %cr3
    movq    %rdi,                   %rcx
    swapgs
    sti
    sysretq
    .size do_syscall, .-do_syscall
