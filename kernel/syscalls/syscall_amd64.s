    .code64
    .section .data
    .global syscall_vec
    .type   syscall_vec,    @object
    .type   syscv_end,      @object
    .type   __held_pc,      @object
    .type   __held_rbx,     @object
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
    .quad syscall_mmap          # 22
    .quad syscall_munmap        # 23
    .quad syscall_dlpreinit     # 24; WIP / ldso-specific
    .quad syscall_dlinit        # 25; WIP / ldso-specific
    .quad syscall_dlopen        # 26; WIP
    .quad syscall_dlclose       # 27; WIP
    .quad syscall_dlsym         # 28; WIP
    .quad syscall_resolve       # 29; WIP / ldso-specific
    .quad syscall_dlpath        # 30; WIP / ldso-specific
    .quad syscall_dlmap         # 31; WIP / ldso-specific
    .quad syscall_depends       # 32; WIP / ldso-specific
    .quad syscall_dladdr        # 33; WIP
    .quad syscall_dlfini        # 34; WIP / ldso-specific
syscv_end:
    .quad on_invalid_syscall    # handler for out-of-range syscalls
    .size syscall_vec,      .-syscall_vec    
    .size syscv_end,        .-syscv_end
__held_pc:
    .quad 0
    .size __held_pc,        .-__held_pc
__held_rbx:
    .quad 0
    .size __held_rbx,       .-__held_rbx
    #   OoOS system call ABI:
    #   System calls are performed using the x86-64 fast system call instruction (SYSCALL).
    #   The caller places arguments in registers DI, SI, D, 8, 9, and 10, and the syscall number in register A.
    #   Important note: the argument order above differs from the Linux ABI, which places register 10 before 8 for some ungodly reason.
    #   Registers C and 11 are destroyed by the SYSCALL instruction.
    #   The result of the call is stored in the A register. Error codes are returned as minus values (between -4095 and -1) as per the Linux syscall ABI.
    .section .text
    .global do_syscall
    .type   do_syscall,         @function
do_syscall:
    cli
    movq    %rbx,                   __held_rbx
    movq    kernel_cr3,             %rbx
    movq    %rbx,                   %cr3
    movq    __held_rbx,             %rbx
    movq    %rsp,                   %gs:0x088
    movq    %rbp,                   %gs:0x080
    movq    %rcx,                   %gs:0x090
    movq    %rbx,                   %gs:0x018
    movq    %rdx,                   %gs:0x028
    movq    %rdi,                   %gs:0x030
    movq    %rsi,                   %gs:0x038
    movq    %r8,                    %gs:0x040
    movq    %r9,                    %gs:0x048
    movq    %r10,                   %gs:0x050
    movq    %r11,                   %gs:0x098    
    movq    %r12,                   %gs:0x060
    movq    %r13,                   %gs:0x068
    movq    %r14,                   %gs:0x070
    movq    %r15,                   %gs:0x078
    incq    %gs:0x2E0
    movq    %gs:0x000,              %rcx
    fxsave  %gs:0x0D0    
    swapgs
    movq    %rcx,                   %gs:0x300
    movq    %gs:0x088,              %rsp
    movq    %gs:0x080,              %rbp
    movq    %r8,                    %rcx
    movq    %r9,                    %r8
    movq    %r10,                   %r9
    fxrstor %gs:0x0D0
    pushq   %rbp
    movq    %rsp,                   %rbp
    cmpl    $0,                     %eax
    jl      on_invalid_syscall
    leaq    syscall_vec,            %r10
    leaq    (%r10, %rax, 8),        %rax
    cmpq    $syscv_end,             %rax
    jg      on_invalid_syscall
    movq    (%rax),                 %rax
    sti
    call    *%rax
    cli
    fxsave  %gs:0x0D0
    swapgs
    fxrstor %gs:0x0D0
    movq    %gs:0x028,              %rdx
    movq    %gs:0x030,              %rdi
    movq    %gs:0x038,              %rsi
    movq    %gs:0x040,              %r8
    movq    %gs:0x048,              %r9
    movq    %gs:0x050,              %r10
    movq    %gs:0x098,              %r11    
    movq    %gs:0x060,              %r12
    movq    %gs:0x068,              %r13
    movq    %gs:0x070,              %r14
    movq    %gs:0x078,              %r15
    movq    %gs:0x080,              %rbp
    movq    %gs:0x088,              %rsp
    movq    %gs:0x018,              %rbx
    movq    %gs:0x090,              %rcx
    movq    %rcx,                   __held_pc
    movq    %gs:0x0A6,              %rcx
    movq    %rcx,                   %cr3
    movq    __held_pc,              %rcx
    sysretq
    .size do_syscall, .-do_syscall
