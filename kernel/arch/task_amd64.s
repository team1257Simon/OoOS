    .code64
    .section    .text
    .global     task_change
    .global     ktask_change
    .global     current_active_task
    .global     user_entry
    .type       task_change,            @function
    .type       ktask_change,           @function
    .type       current_active_task,    @function
    .type       user_entry,             @function    
task_change:
    movq        %rax,           %gs:0x010
    movw        32(%rsp),       %ax
    movw        %ax,            %gs:0x0A4
    movq        24(%rsp),       %rax
    movq        %rax,           %gs:0x088
    movq        16(%rsp),       %rax
    movq        %rax,           %gs:0x098
    movw        8(%rsp),        %ax
    movw        %ax,            %gs:0x0AA
    movq        (%rsp),         %rax    
    movq        %rax,           %gs:0x090
    movw        %ax,            %gs:0x0A0
    movw        %ax,            %gs:0x0A2
    movw        %fs,            %ax
    movw        %ax,            %gs:0x0A6
    movw        %gs,            %ax
    movw        %ax,            %gs:0x0A8
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
    pushfq
    popq        %rax
    movq        %rax,           %gs:0x098
    movq        %gs:0x000,      %rax
    fxsave      0x0E0(%rax)
    movq        0x300(%rax),    %rax
    wrgsbase    %rax
    movq        %gs:0x000,      %rax
    fxrstor     0x0E0(%rax)
    movq        %gs:0x098,      %rax
    pushq       %rax
    popfq
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
    movw        %gs:0x0A4,      %ax
    movw        %ax,            32(%rsp)
    movq        %gs:0x088,      %rax
    movq        %rax,           24(%rsp)
    movq        %gs:0x098,      %rax
    movq        %rax,           16(%rsp)
    movw        %gs:0x0AA,      %ax
    movw        %ax,            8(%rsp)
    movq        %gs:0x090,      %rax
    movq        %rax,           (%rsp)
    movw        %gs:0x0A0,      %ax
    movw        %ax,            %ds
    movw        %gs:0x0A6,      %ax
    movw        %ax,            %fs
    movw        %gs:0x0A8,      %ax
    movw        %ax,            %gs
    movq        %gs:0x0AC,      %rax
    movq        %rax,           %cr3
    movq        %gs:0x010,      %rax
    sti
    iretq
    .size       task_change,    .-task_change
ktask_change:
    movq        %rax,           %gs:0x010
    movq        %rsp,           %gs:0x088
    movw        %ss,            %ax
    movw        %ax,            %gs:0x0A4
    movq        16(%rsp),       %rax
    movq        %rax,           %gs:0x098
    movw        8(%rsp),        %ax
    movw        %ax,            %gs:0x0AA
    movq        (%rsp),         %rax    
    movq        %rax,           %gs:0x090
    movw        %ds,            %ax
    movw        %ax,            %gs:0x0A0
    movw        %ax,            %gs:0x0A2
    movw        %fs,            %ax
    movw        %ax,            %gs:0x0A6
    movw        %gs,            %ax
    movw        %ax,            %gs:0x0A8
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
    movq        %gs:0x000,      %rax
    fxsave      0x0E0(%rax)
    movq        0x2F8(%rax),    %rax
    wrgsbase    %rax
    movq        %gs:0x000,      %rax
    fxrstor     0x0E0(%rax)
    movq        %gs:0x2F0,      %rax
    wrfsbase    %rax
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
    movw        %gs:0x0A4,      %ax
    movw        %ax,            %ss
    movq        %gs:0x098,      %rax
    movq        %rax,           16(%rsp)
    movw        %gs:0x0AA,      %ax
    movw        %ax,            8(%rsp)
    movq        %gs:0x090,      %rax
    movq        %rax,           (%rsp)
    movw        %gs:0x0A0,      %ax
    movw        %ax,            %ds
    movw        %gs:0x0A6,      %ax
    movw        %ax,            %fs
    movw        %gs:0x0A8,      %ax
    movw        %ax,            %gs
    movq        %gs:0x0AC,      %rax
    movq        %rax,           %cr3
    movq        %gs:0x010,      %rax
    sti
    iretq
    .size       ktask_change,           .-ktask_change
current_active_task:
    movq        %gs:0x000,      %rax
    ret
    .size       current_active_task,    .-current_active_task
user_entry:
    cli
    movq    %gs:0xAC,           %rax
    movq    %rax,               %cr3
    movq    %gs:0x010,          %rax    
    movq    %gs:0x018,          %rbx
    movq    %gs:0x090,          %rcx    # instruction pointer goes in the C register for a sysret
    movq    %gs:0x028,          %rdx
    movq    %gs:0x030,          %rdi
    movq    %gs:0x038,          %rsi    
    movq    %gs:0x098,          %r11    # flags must be in r11
    movq    %gs:0x060,          %r12
    movq    %gs:0x068,          %r13
    movq    %gs:0x070,          %r14
    movq    %gs:0x078,          %r15
    movq    %gs:0x080,          %rbp
    movq    %gs:0x088,          %rsp
    sti
    sysretq
    .size       user_entry,             .-user_entry
