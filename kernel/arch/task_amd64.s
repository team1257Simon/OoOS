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
    movq        %rax,           %gs:0x010
    popq        %rax    
    movq        %rax,           %gs:0x090
    popq        %rax
    movw        %ax,            %gs:0x0A4
    popq        %rax
    movq        %rax,           %gs:0x098
    popq        %rax
    movq        %rax,           %gs:0x088
    popq        %rax
    movw        %ax,            %gs:0x0A2
    movw        %ds,            %ax
    movw        %ax,            %gs:0x0A0
    movq        %rbx,           %gs:0x018
    movq        %rcx,           %gs:0x020
    movq        %rdx,           %gs:0x028
    movq        %rdi,           %gs:0x030
    movq        %rsi,           %gs:0x038
    movq        %r8,            %gs:0x040
    movq        %r9,            %gs:0x048
    movq        %r10,           %gs:0x050
    movq        %r11,           %gs:0x058
    movq        %r12,           %gs:0x060
    movq        %r13,           %gs:0x068
    movq        %r14,           %gs:0x070
    movq        %r15,           %gs:0x078
    movq        %rbp,           %gs:0x080
    movq        __held_cr3,     %rax
    movq        %rax,           %gs:0x0A6
    fxsave      %gs:0x0D0
    movq        %gs:0x300,      %rax
    wrgsbase    %rax
    movq        %gs:0x2F8,      %rax
    wrfsbase    %rax
    fxrstor     %gs:0x0D0
    movq        %gs:0x018,      %rbx
    movq        %gs:0x020,      %rcx
    movq        %gs:0x028,      %rdx
    movq        %gs:0x030,      %rdi
    movq        %gs:0x038,      %rsi
    movq        %gs:0x040,      %r8
    movq        %gs:0x048,      %r9
    movq        %gs:0x050,      %r10
    movq        %gs:0x058,      %r11
    movq        %gs:0x060,      %r12
    movq        %gs:0x068,      %r13
    movq        %gs:0x070,      %r14
    movq        %gs:0x078,      %r15
    movq        %gs:0x080,      %rbp
    movw        %gs:0x0A2,      %ax
    pushq       %rax
    movq        %gs:0x088,      %rax
    pushq       %rax
    movq        %gs:0x098,      %rax
    pushq       %rax
    movw        %gs:0x0A4,      %ax
    pushq       %rax
    movq        %gs:0x090,      %rax
    pushq       %rax
    movw        %gs:0x0A0,      %ax       
    movw        %ax,            %ds
    movw        %ax,            %es
    movq        %gs:0x010,      %rax
    pushq       %rax
    movq        %gs:0x0A6,      %rax
    movq        %rax,           %cr3
    popq        %rax
    iretq
    .size       task_change,    .-task_change
user_entry:
    cli
    movq        %rax,           %gs:0x010
    popq        %rax
    movq        %rax,           %gs:0x090
    pushfq
    popq        %rax
    movq        %rax,           %gs:0x098
    movq        %rbx,           %gs:0x018
    movq        %rcx,           %gs:0x020
    movq        %rdx,           %gs:0x028
    movq        %rdi,           %gs:0x030
    movq        %rsi,           %gs:0x038
    movq        %r8,            %gs:0x040
    movq        %r9,            %gs:0x048
    movq        %r10,           %gs:0x050
    movq        %r11,           %gs:0x058
    movq        %r12,           %gs:0x060
    movq        %r13,           %gs:0x068
    movq        %r14,           %gs:0x070
    movq        %r15,           %gs:0x078
    movq        %rbp,           %gs:0x080
    movq        %rsp,           %gs:0x088
    movw        %ds,            %ax
    movw        %ax,            %gs:0x0A0
    movw        %ss,            %ax
    movw        %ax,            %gs:0x0A2
    movw        %cs,            %ax
    movw        %ax,            %gs:0x0A4
    fxsave      %gs:0x0D0
    swapgs   
    movw        0x0A0(%rdi),    %ax
    movw        %ax,            %ds
    movw        %ax,            %es
    movw        %ax,            %fs
    movw        %ax,            %gs
    wrgsbase    %rdi
    movq        %gs:0x2F8,      %rax
    wrfsbase    %rax
    fxrstor     %gs:0x0D0
    movq        %gs:0x018,      %rbx
    movq        %gs:0x090,      %rcx    # instruction pointer goes in the C register for a sysret
    movq        %gs:0x028,      %rdx
    movq        %gs:0x030,      %rdi
    movq        %gs:0x038,      %rsi
    movq        %gs:0x040,      %r8
    movq        %gs:0x048,      %r9
    movq        %gs:0x050,      %r10
    movq        %gs:0x098,      %r11    # flags go in R11 for a sysret
    movq        %gs:0x060,      %r12
    movq        %gs:0x068,      %r13
    movq        %gs:0x080,      %rbp
    movq        %gs:0x088,      %rsp
    movq        %gs:0x070,      %r14
    movq        %gs:0x078,      %r15
    movq        %gs:0x010,      %rax
    movq        %gs:0x0A6,      %rax
    movq        %rax,           %cr3
    sysretq
    .size       user_entry,     .-user_entry
kernel_reentry:
    movq        %gs:0x000,      %rax
    movq        %rax,           %rdi
    movw        0x0A0(%rdi),    %ax
    movw        %ax,            %ds
    movw        %ax,            %es
    movw        %ax,            %fs
    movw        %ax,            %gs
    wrgsbase    %rdi
    movq        %gs:0x2F8,      %rax
    wrfsbase    %rax
    fxrstor     %gs:0x0D0
    movq        %gs:0x018,      %rbx
    movq        %gs:0x020,      %rcx
    movq        %gs:0x028,      %rdx
    movq        %gs:0x030,      %rdi
    movq        %gs:0x038,      %rsi
    movq        %gs:0x040,      %r8
    movq        %gs:0x048,      %r9
    movq        %gs:0x050,      %r10
    movq        %gs:0x058,      %r11
    movq        %gs:0x060,      %r12
    movq        %gs:0x068,      %r13
    movq        %gs:0x070,      %r14
    movq        %gs:0x078,      %r15
    movq        %gs:0x080,      %rbp
    movq        %gs:0x088,      %rsp
    movq        %gs:0x098,      %rax
    pushq       %rax
    popfq
    movq        %gs:0x090,      %rax
    pushq       %rax
    movq        %gs:0x010,      %rax
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