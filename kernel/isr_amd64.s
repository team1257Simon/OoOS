    .code64
    # The one time exponential growth actually comes in handy...
    # Though I suppose that makes this logarithmic complexity, i.e. O(log(n)) macro expansions where n is table size
    .altmacro
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
    .macro isr_enter i
        .global     isr_\i
        .type       isr_\i,     @function
    isr_\i:
        pushq   $\i
        jmp     isr_ramp
        .size   isr_\i,     .-isr_\i
    .endm
    .macro wr_id i
        .quad isr_\i
    .endm
    .section    .text
    .type   isr_ramp,   @function
    xm256   isr_enter
isr_ramp:
    lock xchgq  %rdi,   (%rsp)
    pushq       %rsi
    pushq       %rdx
    pushq       %rcx
    pushq       %r8
    pushq       %r9
    pushq       %r10
    pushq       %r11
    pushq       %rax
    call        isr_dispatch
    popq        %rax
    popq        %r11
    popq        %r10
    popq        %r9
    popq        %r8
    popq        %rcx
    popq        %rdx
    popq        %rsi
    popq        %rdi
    lretq
    .size   isr_ramp,   .-isr_ramp
    .section    .data
    .global     isr_table
    .type       isr_table,      @object
isr_table:
    xm256 wr_id
