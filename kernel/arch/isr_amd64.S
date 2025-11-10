	.code64
	.altmacro
	.global	ecode
	.global	errinst
	.global	svinst
	.global	task_change_flag
	.global	task_signal_code
	.global	kernel_isr_stack_top
	.global	kernel_isr_stack_base
	.global	delay_flag
	.global	debug_stop_flag
	.global	test_fault
	.extern	isr_dispatch
	.extern	set_task_signal
	.global	no_waiting_op
	.type	test_fault,				@function
	.type	isr_dispatch,			@function
	.type	set_task_signal,		@function
	.type	no_waiting_op,			@function
	.type	ecode,					@object
	.type	errinst,				@object
	.type	svinst,					@object
	.type	task_change_flag,		@object
	.type	task_signal_code,		@object
	.type	kernel_isr_stack_top,	@object
	.type	kernel_isr_stack_base,	@object
	.type	delay_flag,				@object
	.type	debug_stop_flag,		@object
	.data
	.align	4096
kernel_isr_stack_base:
	.zero	16384
kernel_isr_stack_top:
	.size	kernel_isr_stack_base,	.-kernel_isr_stack_base
	.size	kernel_isr_stack_top,	.-kernel_isr_stack_base
ecode:
	.quad		0
	.size		ecode,				.-ecode
errinst:
	.quad		0
	.size		errinst,			.-errinst
svinst:
	.quad		0
	.size		svinst,				.-svinst
task_change_flag:
	.byte		0
	.size		task_change_flag,	.-task_change_flag
task_signal_code:
	.byte		0
	.size		task_signal_code,	.-task_signal_code
debug_stop_flag:
	.byte		0
	.size		debug_stop_flag,	.-debug_stop_flag
delay_flag:
	.byte		0
	.size		delay_flag,			.-delay_flag
	.text
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
		.global	isr_\i
		.type	isr_\i,	@function
		isr_\i:
			pushq	%rax
			.ifeq	(\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
			movq	8(%rsp),	%rax
			movq	%rax,		ecode
			movq	16(%rsp),	%rax
			movq	%rax,		errinst
			.else
			movq	8(%rsp),	%rax
			movq	%rax,		svinst
			.endif
			pushq	%rbx
			pushq	%rdi
			movq	$\i,		%rdi
			pushq	%rsi
			pushq	%rdx
			pushq	%rcx
			pushq	%r8
			pushq	%r9
			pushq	%r10
			pushq	%r11
			pushq	%r12
			pushq	%r13
			pushq	%r14
			pushq	%r15
			call	isr_dispatch
			popq	%r15
			popq	%r14
			popq	%r13
			popq	%r12
			popq	%r11
			popq	%r10
			popq	%r9
			popq	%r8
			popq	%rcx
			popq	%rdx
			popq	%rsi
			popq	%rdi
			popq	%rbx
			popq	%rax
			.ifeq	(\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
			addq	$8,		%rsp
			.endif
			cmpb	$0,		task_signal_code
			jnz	set_task_signal
			cmpb	$0,		debug_stop_flag
			jnz	debug_stop
			cmpb	$0,		task_change_flag
			movb	$0,		task_change_flag
			jnz	task_change
			sti
			iretq
		.size		isr_\i,	.-isr_\i
	.endm
	.macro wr_id i
		.quad isr_\i
	.endm
	xm256 wrapper
test_fault:
	// Classic "divide by zero" fault to test the handler established in kmain
	xorq		%rcx,			%rcx
	movq		$1,				%rax
	divq		%rcx
	ret
	.size		test_fault,		.-test_fault
	.type		debug_stop,		@function
debug_stop:
	// stop here in case of a fault; register state will be mostly intact, and the faulting instruction's address will have been displayed
1:	jmp 1b
	.size		debug_stop,		.-debug_stop
no_waiting_op:
	ret
	.size		no_waiting_op,	.-no_waiting_op
	.data
	.global	isr_table
	.type		isr_table,		@object	
isr_table:
	xm256		wr_id
	.size		isr_table,		.-isr_table
