#include "def-amd64.inc"
    .code64
    .section .data
    .global syscall_vec
    .type   syscall_vec,    @object
    .type   syscv_end,      @object
    .type   __held_pc,      @object
    .type   __held_rbx,     @object
syscall_vec:
    .quad syscall_exit          // 0
    .quad syscall_sleep         // 1
    .quad syscall_wait          // 2
    .quad syscall_fork          // 3
    .quad syscall_times         // 4
    .quad syscall_gettimeofday  // 5
    .quad syscall_sbrk          // 6
    .quad syscall_open          // 7
    .quad syscall_close         // 8
    .quad syscall_read          // 9
    .quad syscall_write         // 10
    .quad syscall_link          // 11
    .quad syscall_lseek         // 12
    .quad syscall_unlink        // 13
    .quad syscall_getpid        // 14
    .quad syscall_fstat         // 15
    .quad syscall_stat          // 16
    .quad syscall_fchmod        // 17
    .quad syscall_chmod         // 18
    .quad syscall_isatty        // 19
    .quad syscall_execve        // 20
    .quad syscall_kill          // 21
    .quad syscall_mmap          // 22
    .quad syscall_munmap        // 23
    .quad syscall_dlpreinit     // 24; ldso-specific
    .quad syscall_dlinit        // 25; ldso-specific
    .quad syscall_dlopen        // 26
    .quad syscall_dlclose       // 27
    .quad syscall_dlsym         // 28
    .quad syscall_resolve       // 29; ldso-specific
    .quad syscall_dlpath        // 30; ldso-specific
    .quad syscall_dlmap         // 31; ldso-specific
    .quad syscall_depends       // 32; ldso-specific
    .quad syscall_dladdr        // 33
    .quad syscall_dlfini        // 34; ldso-specific
    .quad syscall_sigret        // 35; WIP
syscv_end:
    .quad on_invalid_syscall    // handler for out-of-range syscalls
    .size syscall_vec,      .-syscall_vec    
    .size syscv_end,        .-syscv_end
__held_pc:
    .quad 0
    .size __held_pc,        .-__held_pc
__held_rbx:
    .quad 0
    .size __held_rbx,       .-__held_rbx
    //   OoOS system call ABI:
    //   System calls are performed using the x86-64 fast system call instruction (SYSCALL).
    //   The caller places arguments in registers DI, SI, D, 8, 9, and 10, and the syscall number in register A.
    //   Important note: the argument order above differs from the Linux ABI, which places register 10 before 8 for some ungodly reason.
    //   Registers C and 11 are destroyed by the SYSCALL instruction.
    //   The result of the call is stored in the A register. Error codes are returned as minus values (between -4095 and -1) as per the Linux syscall ABI.
    .section .text
    .global do_syscall
    .type   do_syscall,         @function
do_syscall:
    movq    %rbx,                   __held_rbx
    movq    kernel_cr3,             %rbx
    movq    %rbx,                   %cr3
    movq    __held_rbx,             %rbx
    syscall_state_save
    incq    %gs:T_OFFS(sys_time)
    movq    %gs:T_OFFS(self),       %rcx
    swapgs
    movq    %rcx,                   %gs:T_OFFS(next)
    movq    %gs:T_OFFS(rsp),        %rsp
    movq    %gs:T_OFFS(rbp),        %rbp
    movq    %r8,                    %rcx
    movq    %r9,                    %r8
    movq    %r10,                   %r9
    fxrstor %gs:T_OFFS(fxsv)
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
    fxsave  %gs:T_OFFS(fxsv)
    swapgs
    syscall_state_restore
    movq    %rcx,                   __held_pc
    movq    %gs:T_OFFS(cr3),        %rcx
    movq    %rcx,                   %cr3
    movq    __held_pc,              %rcx
    sysretq
    .size do_syscall, .-do_syscall
