#include "asm-defs.inc"
    .code64
    .section    .text
    .global     task_change
    .global     current_active_task
    .global     user_entry
    .global     kernel_reentry
    .global     enable_fs_gs_insns
    .type       task_change,            @function
    .type       current_active_task,    @function
    .type       user_entry,             @function
    .type       kernel_reentry,         @function
    .type       enable_fs_gs_insns,     @function
task_change:
    pushq       %rax
    movq        %cr3,           %rax
    movq        %rax,           __held_cr3
    movq        kernel_cr3,     %rax
    movq        %rax,           %cr3
    popq        %rax
    movq        %rax,           %gs:T_OFFS(rax)
    popq        %rax    
    movq        %rax,           %gs:T_OFFS(rip)
    popq        %rax
    movw        %ax,            %gs:T_OFFS(cs)
    popq        %rax
    movq        %rax,           %gs:T_OFFS(rflags)
    popq        %rax
    movq        %rax,           %gs:T_OFFS(rsp)
    popq        %rax
    movw        %ax,            %gs:T_OFFS(ss)
    movw        %ds,            %ax
    movw        %ax,            %gs:T_OFFS(ds)
    movq        %rbx,           %gs:T_OFFS(rbx)
    movq        %rcx,           %gs:T_OFFS(rcx)
    movq        %rdx,           %gs:T_OFFS(rdx)
    movq        %rdi,           %gs:T_OFFS(rdi)
    movq        %rsi,           %gs:T_OFFS(rsi)
    movq        %r8,            %gs:T_OFFS(r8)
    movq        %r9,            %gs:T_OFFS(r9)
    movq        %r10,           %gs:T_OFFS(r10)
    movq        %r11,           %gs:T_OFFS(r11)
    movq        %r12,           %gs:T_OFFS(r12)
    movq        %r13,           %gs:T_OFFS(r13)
    movq        %r14,           %gs:T_OFFS(r14)
    movq        %r15,           %gs:T_OFFS(r15)
    movq        %rbp,           %gs:T_OFFS(rbp)
    movq        __held_cr3,     %rax
    movq        %rax,           %gs:T_OFFS(cr3)
    fxsave      %gs:T_OFFS(fxsv)
    movq        %gs:T_OFFS(next),      %rax
    wrgsbase    %rax
    movq        %gs:T_OFFS(tls_block), %rax
    wrfsbase    %rax
    fxrstor     %gs:T_OFFS(fxsv)
    movq        %gs:T_OFFS(rbx),      %rbx
    movq        %gs:T_OFFS(rcx),      %rcx
    movq        %gs:T_OFFS(rdx),      %rdx
    movq        %gs:T_OFFS(rdi),      %rdi
    movq        %gs:T_OFFS(rsi),      %rsi
    movq        %gs:T_OFFS(r8),       %r8
    movq        %gs:T_OFFS(r9),       %r9
    movq        %gs:T_OFFS(r10),      %r10
    movq        %gs:T_OFFS(r11),      %r11
    movq        %gs:T_OFFS(r12),      %r12
    movq        %gs:T_OFFS(r13),      %r13
    movq        %gs:T_OFFS(r14),      %r14
    movq        %gs:T_OFFS(r15),      %r15
    movq        %gs:T_OFFS(rbp),      %rbp
    movw        %gs:T_OFFS(ss),       %ax
    pushq       %rax
    movq        %gs:T_OFFS(rsp),      %rax
    pushq       %rax
    movq        %gs:T_OFFS(rflags),   %rax
    pushq       %rax
    movw        %gs:T_OFFS(cs),       %ax
    pushq       %rax
    movq        %gs:T_OFFS(rip),      %rax
    pushq       %rax
    movw        %gs:T_OFFS(ds),       %ax       
    movw        %ax,                  %ds
    movw        %ax,                  %es
    movq        %gs:T_OFFS(rax),      %rax
    pushq       %rax
    movq        %gs:T_OFFS(cr3),      %rax
    movq        %rax,                 %cr3
    popq        %rax
    iretq
    .size       task_change,    .-task_change
user_entry:
    cli
    movq        %rax,           %gs:T_OFFS(rax)
    popq        %rax
    movq        %rax,           %gs:T_OFFS(rip)
    pushfq
    popq        %rax
    movq        %rax,           %gs:T_OFFS(rflags)
    movq        %rbx,           %gs:T_OFFS(rbx)
    movq        %rcx,           %gs:T_OFFS(rcx)
    movq        %rdx,           %gs:T_OFFS(rdx)
    movq        %rdi,           %gs:T_OFFS(rdi)
    movq        %rsi,           %gs:T_OFFS(rsi)
    movq        %r8,            %gs:T_OFFS(r8)
    movq        %r9,            %gs:T_OFFS(r9)
    movq        %r10,           %gs:T_OFFS(r10)
    movq        %r11,           %gs:T_OFFS(r11)
    movq        %r12,           %gs:T_OFFS(r12)
    movq        %r13,           %gs:T_OFFS(r13)
    movq        %r14,           %gs:T_OFFS(r14)
    movq        %r15,           %gs:T_OFFS(r15)
    movq        %rbp,           %gs:T_OFFS(rbp)
    movq        %rsp,           %gs:T_OFFS(rsp)
    movw        %ds,            %ax
    movw        %ax,            %gs:T_OFFS(ds)
    movw        %ss,            %ax
    movw        %ax,            %gs:T_OFFS(ss)
    movw        %cs,            %ax
    movw        %ax,            %gs:T_OFFS(cs)
    fxsave      %gs:T_OFFS(fxsv)
    swapgs   
    movw        T_OFFS(ds)(%rdi),   %ax
    movw        %ax,                %ds
    movw        %ax,                %es
    movw        %ax,                %fs
    movw        %ax,                %gs
    wrgsbase    %rdi
    movq        %gs:T_OFFS(tls_block),  %rax
    wrfsbase    %rax
    fxrstor     %gs:T_OFFS(fxsv)
    movq        %gs:T_OFFS(rbx),        %rbx
    movq        %gs:T_OFFS(rip),        %rcx    // instruction pointer goes in the C register for a sysret
    movq        %gs:T_OFFS(rdx),        %rdx
    movq        %gs:T_OFFS(rdi),        %rdi
    movq        %gs:T_OFFS(rsi),        %rsi
    movq        %gs:T_OFFS(r8),         %r8
    movq        %gs:T_OFFS(r9),         %r9
    movq        %gs:T_OFFS(r10),        %r10
    movq        %gs:T_OFFS(rflags),     %r11    // flags go in R11 for a sysret
    movq        %gs:T_OFFS(r12),        %r12
    movq        %gs:T_OFFS(r13),        %r13
    movq        %gs:T_OFFS(rbp),        %rbp
    movq        %gs:T_OFFS(rsp),        %rsp
    movq        %gs:T_OFFS(r14),        %r14
    movq        %gs:T_OFFS(r15),        %r15
    movq        %gs:T_OFFS(rax),        %rax
    movq        %gs:T_OFFS(cr3),        %rax
    movq        %rax,                   %cr3
    xorq        %rax,                   %rax
    sysretq
    .size       user_entry,     .-user_entry
kernel_reentry:
    movq        %gs:T_OFFS(self),   %rax
    movq        %rax,               %rdi
    movw        T_OFFS(ds)(%rdi),   %ax
    movw        %ax,                %ds
    movw        %ax,                %es
    movw        %ax,                %fs
    movw        %ax,                %gs
    wrgsbase    %rdi
    movq        %gs:T_OFFS(tls_block),  %rax
    wrfsbase    %rax
    fxrstor     %gs:T_OFFS(fxsv)
    movq        %gs:T_OFFS(rbx),        %rbx
    movq        %gs:T_OFFS(rcx),        %rcx
    movq        %gs:T_OFFS(rdx),        %rdx
    movq        %gs:T_OFFS(rdi),        %rdi
    movq        %gs:T_OFFS(rsi),        %rsi
    movq        %gs:T_OFFS(r8),         %r8
    movq        %gs:T_OFFS(r9),         %r9
    movq        %gs:T_OFFS(r10),        %r10
    movq        %gs:T_OFFS(r11),        %r11
    movq        %gs:T_OFFS(r12),        %r12
    movq        %gs:T_OFFS(r13),        %r13
    movq        %gs:T_OFFS(r14),        %r14
    movq        %gs:T_OFFS(r15),        %r15
    movq        %gs:T_OFFS(rbp),        %rbp
    movq        %gs:T_OFFS(rsp),        %rsp
    movq        %gs:T_OFFS(rflags),     %rax
    pushq       %rax
    popfq
    movq        %gs:T_OFFS(rip),        %rax
    pushq       %rax
    movq        %gs:T_OFFS(rax),        %rax
    sti
    ret
    .size       kernel_reentry,         .-kernel_reentry
enable_fs_gs_insns:
    movq        %cr4,       %rax
    orl         $0x10000,   %eax
    movq        %rax,       %cr4
    ret
    .size       enable_fs_gs_insns,     .-enable_fs_gs_insns
    .section    .data
    .type       __held_cr3,             @object
__held_cr3:
    .quad 0
    .size       __held_cr3,             .-__held_cr3