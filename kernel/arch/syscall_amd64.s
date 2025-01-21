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
    movq    %rsp,                   %gs:0x88
    movq    %rbp,                   %gs:0x80
    movq    %rcx,                   %gs:0x90
    swapgs
    movq    %gs:0x88,               %rsp
    movq    %gs:0x80,               %rbp
    leaq    syscall_vec(,%rax,8),   %rax
    movq    %r11,                   %gs:0x98
    sti
    call    *%rax
    cli
    movq    %gs:0x98,               %r11
    xorq    %rdi,                   %rdi
    xorq    %rsi,                   %rsi
    xorq    %rdx,                   %rdx
    xorq    %r8,                    %r8
    xorq    %r9,                    %r9
    xorq    %r10,                   %r10
    swapgs
    movq    %gs:0x80,               %rbp
    movq    %gs:0x88,               %rsp
    movq    %gs:0x90,               %rcx
    sti
    sysretq
    .size do_syscall, .-do_syscall
