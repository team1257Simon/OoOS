    .code64
    .section .data
    .global syscall_vec
    .type   syscall_vec,    @object
syscall_vec:
    .quad syscall_exit          # 0
    .quad syscall_sleep         # 1
    .quad syscall_wait          # 2
    .quad syscall_fork          # 3
    .quad syscall_times         # 4
    .quad syscall_gettimeofday  # 5
    .quad syscall_sbrk          # 6
    .quad syscall_open          # 7
    .quad syscall_close         # 8
    .quad syscall_read          # 9
    .quad syscall_write         # 10
    .quad syscall_link          # 11
    .quad syscall_lseek         # 12
    .quad syscall_unlink        # 13
    .quad syscall_getpid        # 14
    .quad syscall_fstat         # 15
    .quad syscall_stat          # 16
    .quad syscall_fchmod        # 17
    .quad syscall_chmod         # 18
    .quad syscall_isatty        # 19
    .quad syscall_execve        # 20
    .quad syscall_kill          # 21
    .size   syscall_vec,    .-syscall_vec
    .section .text
    .global do_syscall
    .type   do_syscall,     @function
do_syscall:
    cli
    movq    %rsp,                   %gs:0x088
    movq    %rbp,                   %gs:0x080
    movq    %rcx,                   %gs:0x090
    movq    %rbx,                   %gs:0x018
    incq    %gs:0x2F0     
    movq    %gs:0x000,              %rcx
    swapgs
    movq    %gs:0x0A6,              %rbx
    movq    %rbx,                   %cr3
    movq    %rcx,                   %gs:0x300
    movq    %gs:0x088,              %rsp
    movq    %gs:0x080,              %rbp
    movq    %r8,                    %rcx
    movq    %r9,                    %r8
    movq    %r10,                   %r9
    leaq    syscall_vec,            %r10
    movq    (%r10, %rax, 8),        %rax
    movq    %r11,                   %gs:0x098
    # subject to change, but for now just count all syscalls as 1
    sti
    call    *%rax
    cli
    movq    %gs:0x098,              %r11
    xorq    %rdi,                   %rdi
    xorq    %rsi,                   %rsi
    xorq    %rdx,                   %rdx
    xorq    %r8,                    %r8
    xorq    %r9,                    %r9
    xorq    %r10,                   %r10
    swapgs
    movq    %gs:0x080,              %rbp
    movq    %gs:0x088,              %rsp
    movq    %gs:0x018,              %rbx
    movq    %gs:0x0A6,              %rcx
    movq    %rcx,                   %cr3    
    movq    %gs:0x090,              %rcx
    sysretq
    .size do_syscall, .-do_syscall
