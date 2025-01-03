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
    .macro wrapper i
        .global     isr_\i
        .type       isr_\i,     @function
        isr_\i:
            cli
            pushq   %rax
            addq    $8,         %rsp
            popq    %rax
            movq    %rax,       ecode
            popq    %rax
            movq    %rax,       errinst
            subq    $0x18,      %rsp
            pushq   %rdi
            movq    $\i,        %rdi
            call    isr_dispatch
            popq    %rdi
            testq   %rax,       %rax
            popq    %rax
            jz      .L\i
            addq    $0x8,       %rsp
        .L\i:
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
    .global     ecode
    .global     errinst
    .type       isr_table,      @object
    .type       ecode,          @object
    .type       errinst,        @object
isr_table:
    xm256 wr_id
    .size       isr_table,      .-isr_table
ecode:
    .quad 0
    .size       ecode,          .-ecode
errinst:
    .quad 0
    .size       errinst,        .-errinst
