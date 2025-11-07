#include "def-amd64.inc"
//      OoOS x86-64 system call ABI:
//      System calls are performed using the x86-64 fast system call instruction (SYSCALL).
//      The caller places arguments in registers DI, SI, D, 8, 9, and 10, and the system call number in register A.
//      Important note: the argument order above differs from the Linux system call ABI, which places register 10 before 8 for some ungodly reason.
//      Registers C and 11 are destroyed by the SYSCALL instruction. The return value is stored in the A register.
//      The kernel restores ALL registers (including scratch and argument registers) to their pre-call state before returning to the user process.
//      As per the Linux system call ABI result between -4095 and -1 (inclusive) indicates an error. 
//      In these cases, the returned value is the error code's additive inverse (i.e. errno = -1 * %rax).
//      A negative number with an absolute value GREATER than 4095 is a non-error result (for instance, a pointer to a value in a read-only system segment).
//      Other notes:
//      The header files which declare the functions that implement the system calls contain comments describing the C-style function declaration equivalent.
//      System calls 24-34 are intended for use by the dynamic linker (ld-ooos.so). 
//      They perform input validation, but will likely return an error if used directly. 
//      The dynamic linker exports wrappers to calls 26-28 (dlopen, dlclose, dlsym) and 33 (dladdr) which perform some additional bookkeeping as well.
//      Other libdl functions, such as dlinfo, are implemented in the dynamic linker using information from system call 31 (dlmap).
//      System call 35 (sigret) is used by the signal trampoline to resume execution and will return an EINVAL error if used without such context.
//      Syscalls 49, 50, and 51 access the user data in sysfs and return them in a format consistent with the unix /etc/pwd file entries.
	.code64
	.data
	.global                         do_syscall
	.global                         user_reentry
	.extern                         syscall_exit
	.extern                         syscall_sleep
	.extern                         syscall_wait
	.extern                         syscall_fork
	.extern                         syscall_times
	.extern                         syscall_gettimeofday
	.extern                         syscall_sbrk
	.extern                         syscall_open
	.extern                         syscall_close
	.extern                         syscall_read
	.extern                         syscall_write
	.extern                         syscall_link
	.extern                         syscall_lseek
	.extern                         syscall_unlink
	.extern                         syscall_getpid
	.extern                         syscall_fstat
	.extern                         syscall_stat
	.extern                         syscall_fchmod
	.extern                         syscall_chmod
	.extern                         syscall_isatty
	.extern                         syscall_execve
	.extern                         syscall_kill
	.extern                         syscall_mmap
	.extern                         syscall_munmap
	.extern                         syscall_dlpreinit
	.extern                         syscall_dlinit
	.extern                         syscall_dlopen
	.extern                         syscall_dlclose
	.extern                         syscall_dlsym
	.extern                         syscall_resolve
	.extern                         syscall_dlpath
	.extern                         syscall_dlmap
	.extern                         syscall_depends
	.extern                         syscall_dladdr
	.extern                         syscall_dlfini
	.extern                         syscall_sigret
	.extern                         syscall_signal
	.extern                         syscall_raise
	.extern                         syscall_sigprocmask
	.extern                         syscall_mkdir
	.extern                         syscall_opendir
	.extern                         syscall_fdopendir
	.extern                         syscall_closedir
	.extern                         syscall_lstat
	.extern                         syscall_mknod
	.extern                         syscall_mknodat
	.extern                         syscall_vfork
	.extern                         syscall_spawn
	.extern                         syscall_pipe
	.extern                         syscall_getvpwent
	.extern                         syscall_getvpwuid
	.extern                         syscall_getvpwnam
	.extern                         on_invalid_syscall
	.global                         syscall_vec
	.extern                         kernel_cr3
	.type   do_syscall,             @function
	.type   syscall_exit,			@function
	.type   syscall_sleep,			@function
	.type   syscall_wait,			@function
	.type   syscall_fork,			@function
	.type   syscall_times,			@function
	.type   syscall_gettimeofday,	@function
	.type   syscall_sbrk,			@function
	.type   syscall_open,			@function
	.type   syscall_close,			@function
	.type   syscall_read,			@function
	.type   syscall_write,			@function
	.type   syscall_link,			@function
	.type   syscall_lseek,			@function
	.type   syscall_unlink,			@function
	.type   syscall_getpid,			@function
	.type   syscall_fstat,			@function
	.type   syscall_stat,			@function
	.type   syscall_fchmod,			@function
	.type   syscall_chmod,			@function
	.type   syscall_isatty,			@function
	.type   syscall_execve,			@function
	.type   syscall_kill,			@function
	.type   syscall_mmap,			@function
	.type   syscall_munmap,			@function
	.type   syscall_dlpreinit,		@function
	.type   syscall_dlinit,			@function
	.type   syscall_dlopen,			@function
	.type   syscall_dlclose,		@function
	.type   syscall_dlsym,			@function
	.type   syscall_resolve,		@function
	.type   syscall_dlpath,			@function
	.type   syscall_dlmap,			@function
	.type   syscall_depends,		@function
	.type   syscall_dladdr,			@function
	.type   syscall_dlfini,			@function
	.type   syscall_sigret,			@function
	.type   syscall_signal,			@function
	.type   syscall_raise,			@function
	.type   syscall_sigprocmask,	@function
	.type   syscall_mkdir,          @function
	.type   syscall_opendir,        @function
	.type   syscall_fdopendir,      @function
	.type   syscall_closedir,       @function
	.type   syscall_lstat,          @function
	.type   syscall_mknod,          @function
	.type   syscall_mknodat,        @function
	.type   syscall_vfork,          @function
	.type   syscall_spawn,          @function
	.type   syscall_pipe,           @function
	.type   syscall_getvpwent,      @function
	.type   syscall_getvpwuid,      @function
	.type   syscall_getvpwuid,      @function
	.type   on_invalid_syscall,     @function
	.type   syscall_vec,            @object
	.type   syscv_end,              @object
	.type   kernel_cr3,             @object
	.type   __held_pc,              @object
	.type   __held_rbx,             @object
syscall_vec:
	.quad syscall_exit              // 0
	.quad syscall_sleep             // 1
	.quad syscall_wait              // 2
	.quad syscall_fork              // 3
	.quad syscall_times             // 4
	.quad syscall_gettimeofday      // 5
	.quad syscall_sbrk              // 6
	.quad syscall_open              // 7
	.quad syscall_close             // 8
	.quad syscall_read              // 9
	.quad syscall_write             // 10
	.quad syscall_link              // 11
	.quad syscall_lseek             // 12
	.quad syscall_unlink            // 13
	.quad syscall_getpid            // 14
	.quad syscall_fstat             // 15
	.quad syscall_stat              // 16
	.quad syscall_fchmod            // 17
	.quad syscall_chmod             // 18
	.quad syscall_isatty            // 19
	.quad syscall_execve            // 20
	.quad syscall_kill              // 21
	.quad syscall_mmap              // 22
	.quad syscall_munmap            // 23
	.quad syscall_dlpreinit         // 24; ldso-specific
	.quad syscall_dlinit            // 25; ldso-specific
	.quad syscall_dlopen            // 26; ldso-specific (exported)
	.quad syscall_dlclose           // 27; ldso-specific (exported)
	.quad syscall_dlsym             // 28; ldso-specific (exported)
	.quad syscall_resolve           // 29; ldso-specific
	.quad syscall_dlpath            // 30; ldso-specific
	.quad syscall_dlmap             // 31; ldso-specific
	.quad syscall_depends           // 32; ldso-specific
	.quad syscall_dladdr            // 33; ldso-specific (exported)
	.quad syscall_dlfini            // 34; ldso-specific
	.quad syscall_sigret            // 35
	.quad syscall_signal            // 36
	.quad syscall_raise             // 37
	.quad syscall_sigprocmask       // 38
	.quad syscall_mkdir             // 39
	.quad syscall_opendir           // 40
	.quad syscall_fdopendir         // 41
	.quad syscall_closedir          // 42
	.quad syscall_lstat             // 43
	.quad syscall_mknod             // 44
	.quad syscall_mknodat           // 45
	.quad syscall_vfork             // 46
	.quad syscall_spawn             // 47
	.quad syscall_pipe              // 48
	.quad syscall_getvpwent         // 49
	.quad syscall_getvpwuid         // 50
	.quad syscall_getvpwnam         // 51
syscv_end:
	.quad on_invalid_syscall        // handler for out-of-range syscalls
	.size syscall_vec,              .-syscall_vec    
	.size syscv_end,                .-syscv_end
__held_pc:
	.quad 0
	.size __held_pc,                .-__held_pc
__held_rbx:
	.quad 0
	.size __held_rbx,               .-__held_rbx
	.text
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
	jl      2f
	leaq    syscall_vec,            %r10
	leaq    (%r10, %rax, 8),        %rax
	cmpq    $syscv_end,             %rax
	jg      2f
	movq    (%rax),                 %rax
1:
	call    *%rax
user_reentry:
	fxsave  %gs:T_OFFS(fxsv)
	swapgs
	syscall_state_restore
	movq    %rcx,                   __held_pc
	movq    %gs:T_OFFS(cr3),        %rcx
	movq    %rcx,                   %cr3
	movq    __held_pc,              %rcx
	sysretq
	.size   user_reentry,           .-user_reentry
2:
	leaq    on_invalid_syscall,     %rax
	jmp 1b
	.size do_syscall,               .-do_syscall
