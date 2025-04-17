#include "def-amd64.inc"
    .code64
    .global         task_change
    .global         user_entry
    .global         kernel_reentry
    .global         enable_fs_gs_insns
    .global         set_task_signal
    .extern         force_signal
    .extern         task_signal_code
    .type           task_change,         @function
    .type           user_entry,          @function
    .type           kernel_reentry,      @function
    .type           enable_fs_gs_insns,  @function
    .type           set_task_signal,     @function
    .type           force_signal,        @function
    .type           task_signal_code,    @object
    .text
    .macro          interrupt_stack_save
        pushq       %rax
        movq        kernel_cr3,           %rax
        movq        %rax,                 %cr3
        popq        %rax
        movq        %rax,                 %gs:T_OFFS(rax)
        popq        %rax    
        movq        %rax,                 %gs:T_OFFS(rip)
        popq        %rax
        movw        %ax,                  %gs:T_OFFS(cs)
        popq        %rax
        movq        %rax,                 %gs:T_OFFS(rflags)
        popq        %rax
        movq        %rax,                 %gs:T_OFFS(rsp)
        popq        %rax
        movw        %ax,                  %gs:T_OFFS(ss)
        movw        %ds,                  %ax
        movw        %ax,                  %gs:T_OFFS(ds)
    .endm
    .macro          interrupt_stack_restore
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
    .endm
task_change:
    interrupt_stack_save
    base_state_save
    movq        %gs:T_OFFS(next),       %rax
    wrgsbase    %rax
    movq        %gs:T_OFFS(tls_block),  %rax
    wrfsbase    %rax
    base_state_restore
    interrupt_stack_restore
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
    base_state_save
    movq        %rsp,           %gs:T_OFFS(rsp)
    movw        %ds,            %ax
    movw        %ax,            %gs:T_OFFS(ds)
    movw        %ss,            %ax
    movw        %ax,            %gs:T_OFFS(ss)
    movw        %cs,            %ax
    movw        %ax,            %gs:T_OFFS(cs)
    swapgs   
    movw        T_OFFS(ds)(%rdi),   %ax
    movw        %ax,                %ds
    movw        %ax,                %es
    movw        %ax,                %fs
    movw        %ax,                %gs
    wrgsbase    %rdi
    movq        %gs:T_OFFS(tls_block),  %rax
    wrfsbase    %rax
    syscall_state_restore
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
    base_state_restore
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
set_task_signal:
    interrupt_stack_save
    base_state_save
    movq        %gs:T_OFFS(self),       %rdi
    xorl        %esi,                   %esi
    lock xchgb  %sil,                   task_signal_code
    call        force_signal
    base_state_restore
    interrupt_stack_restore
    iretq
    .size       set_task_signal,        .-set_task_signal
