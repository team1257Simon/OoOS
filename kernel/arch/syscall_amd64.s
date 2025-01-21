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
    pushq   %rax
    movq    %gs:0x00,               %rax
    swapgs
    movq    %rax,                   %gs:0x30
    popq    %rax
    movq    %rsp,                   %gs:0x18
    movq    %rbp,                   %gs:0x20
    movq    %rcx,                   %gs:0x28
    movq    %gs:0x08,               %rsp
    movq    %gs:0x10,               %rbp
    leaq    syscall_vec(,%rax,8),   %rax
    pushq   %r11
    sti
    call    *%rax
    cli
    popq    %r11
    xorq    %rdi,                   %rdi
    xorq    %rsi,                   %rsi
    xorq    %rdx,                   %rdx
    xorq    %r8,                    %r8
    xorq    %r9,                    %r9
    xorq    %r10,                   %r10
    movq    %gs:0x20,               %rbp
    movq    %gs:0x18,               %rsp
    movq    %gs:0x28,               %rcx
    swapgs
    sti
    sysretq
    .size do_syscall, .-do_syscall
