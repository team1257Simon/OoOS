    .code64
    .altmacro
    .extern     isr_dispatch
    .type       isr_dispatch,           @function
    .section    .data
    .global     ecode
    .global     errinst
    .global     svinst
    .global     task_change_flag
    .global     kernel_isr_stack_top
    .global     kernel_isr_stack_base
    .global     debug_stop_flag
    .type       ecode,                  @object
    .type       errinst,                @object
    .type       svinst,                 @object
    .type       task_change_flag,       @object
    .type       kernel_isr_stack_top,   @object
    .type       kernel_isr_stack_base,  @object
    .type       debug_stop_flag,        @object
    .align  4096
kernel_isr_stack_base:
    .zero   16384
kernel_isr_stack_top:
    .size       kernel_isr_stack_base,  .-kernel_isr_stack_base
    .size       kernel_isr_stack_top,   .-kernel_isr_stack_base
ecode:
    .quad 0
    .size       ecode,          .-ecode
errinst:
    .quad 0
    .size       errinst,        .-errinst
svinst:
    .quad 0
    .size       svinst,         .-svinst
task_change_flag:
    .byte 0
    .size       task_change_flag,   .-task_change_flag
debug_stop_flag:
    .byte 0
    .size       debug_stop_flag,    .-debug_stop_flag
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
            pushq   %rax
            .ifeq (\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
            movq    8(%rsp),    %rax
            movq    %rax,       ecode
            movq    16(%rsp),   %rax
            movq    %rax,       errinst
            .else
            movq    8(%rsp),    %rax
            movq    %rax,       svinst
            .endif
            pushq   %rdi
            movq    $\i,        %rdi
            call    isr_dispatch
            popq    %rdi
            popq    %rax
            .ifeq (\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
            addq    $8,         %rsp
            .endif
            cmpb    $0,         debug_stop_flag
            jnz     debug_stop
            cmpb    $0,         task_change_flag
            jz      1f
            movb    $0,         task_change_flag
            jmp     task_change
        1:
            sti
            iretq
        .size       isr_\i,     .-isr_\i
    .endm
    .macro wr_id i
        .quad isr_\i
    .endm
    xm256 wrapper
    .global test_fault
    .type test_fault,   @function
test_fault:
    # Classic "divide by zero" fault to test the handler established in kmain
    xorq    %rcx,   %rcx
    movq    $1,     %rax
    divq    %rcx
    ret
    .size   test_fault, .-test_fault
    .type   debug_stop, @function
debug_stop:
    # stop here in case of a fault; register state will be mostly intact, and the faulting instruction's address will have been displayed
1:  jmp 1b
    .size   debug_stop, .-debug_stop
    .section    .data
    .global     isr_table
    .type       isr_table,      @object    
isr_table:
    xm256 wr_id
    .size       isr_table,      .-isr_table
