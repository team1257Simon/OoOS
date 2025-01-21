    .code64
    .altmacro
    .extern isr_dispatch
    .type   isr_dispatch,   @function
    .section    .data
    .global     ecode
    .global     errinst
    .global     svinst
    .global     taskchg_flag
    .type       taskchg_flag,   @object
    .type       ecode,          @object
    .type       errinst,        @object
    .type       svinst,         @object
ecode:
    .quad 0
    .size       ecode,          .-ecode
errinst:
    .quad 0
    .size       errinst,        .-errinst
svinst:
    .quad 0
    .size       svinst,         .-svinst
taskchg_flag:
    .byte 0
    .size       taskchg_flag,   .-taskchg_flag
    .section    .text
    # The one time exponential growth actually comes in handy...
    # Though I suppose that makes this logarithmic complexity, i.e. O(log(n)) macro expansions where n is table size
    .macro xm_1 macro at
        \macro \at
        \macro %(at+1)
    .endm
    .macro xm_2 macro at
        xm_1 \macro \at
        xm_1 \macro %(at+2)
    .endm
    .macro xm_3 macro at
        xm_2 \macro \at
        xm_2 \macro %(at+4)
    .endm
    .macro xm_4 macro at
        xm_3 \macro \at
        xm_3 \macro %(at+8)
    .endm
    .macro xm_5 macro at
        xm_4 \macro \at
        xm_4 \macro %(at+16)
    .endm
    .macro xm_6 macro at
        xm_5 \macro \at
        xm_5 \macro %(at+32)
    .endm
    .macro xm_7 macro at
        xm_6 \macro \at
        xm_6 \macro %(at+64)
    .endm
    .macro xm_8 macro at
        xm_7 \macro \at
        xm_7 \macro %(at+128)
    .endm
    .macro xm256 macro
        xm_8 \macro 0
    .endm
    .macro wrapper i
        .global     isr_\i
        .type       isr_\i,     @function
        isr_\i:
        # First, clear interrupts. Then check if we're in ring 0 (segment selector 8). If not, swap the gs base to ensure we're able to access kernel stuff if needed
            cli
            cmpw    $0x08,  8(%rsp)
            je      .LKB\i
            swapgs
        .LKB\i:
            pushq   %rax
            .ifge 32-\i
            .ifeq (\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
            movq    8(%rsp),    %rax
            movq    %rax,       ecode
            movq    16(%rsp),   %rax
            movq    %rax,       errinst
            .else
            movq    8(%rsp),    %rax
            movq    %rax,       svinst
            .endif
            .else
            movq    8(%rsp),    %rax
            movq    %rax,       svinst
            .endif
            pushq   %rdi
            movq    $\i,        %rdi
            call    isr_dispatch
            popq    %rdi
            popq    %rax
        .L\i:
            cmpw    $0x08, 8(%rsp)
            je      .LKN\i
            swapgs
            .ifge 32-\i
            .ifeq (\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
            addq    $8,         %rsp
            .endif
            .endif
            cmpb    $0,         taskchg_flag
            jz      .LKF\i
            movb    $0,         taskchg_flag
            jmp     task_change
        .LKN\i:
            .ifge 32-\i
            .ifeq (\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
            addq    $8,         %rsp
            .endif
            .endif
            cmpb    $0,         taskchg_flag
            jz      .LKF\i
            movb    $0,         taskchg_flag
            jmp     ktask_change
        .LKF\i:
            sti
            iretq
        .size       isr_\i,     .-isr_\i
    .endm
    .macro wr_id i
        .quad isr_\i
    .endm
    xm256 wrapper
    .section    .data
    .global     isr_table
    .type       isr_table,      @object    
isr_table:
    xm256 wr_id
    .size       isr_table,      .-isr_table
