	.code64
	.altmacro
	.global	kernel_isr_stack_top
	.global	kernel_isr_stack_base
	.global	ecode
	.global	errinst
	.global	svinst
	.global callback_8
	.global	interrupted_state
	.global	task_change_flag
	.global	task_signal_code
	.global	delay_flag
	.global	debug_stop_flag
	.global	task_lock_flag
	.extern	isr_dispatch
	.extern	set_task_signal
	.extern	task_change
	.global	test_fault
	.global	no_waiting_op
	.local	isr_trampoline
	.type	isr_dispatch,			@function
	.type	set_task_signal,		@function
	.type	task_change,			@function
	.type	test_fault,				@function
	.type	no_waiting_op,			@function
	.type	isr_trampoline,			@function
	.type	kernel_isr_stack_top,	@object
	.type	kernel_isr_stack_base,	@object
	.type	ecode,					@object
	.type	errinst,				@object
	.type	svinst,					@object
	.type	callback_8,				@object
	.type	interrupted_state,		@object
	.type	task_change_flag,		@object
	.type	task_signal_code,		@object
	.type	delay_flag,				@object
	.type	debug_stop_flag,		@object
	.type	task_lock_flag,			@object
	//		If we ever get around to using SMP, this will allow us to have separate versions of the local storage here for each processor.
	.section	.data.plocal
kernel_isr_stack_base:
	.zero	16384
kernel_isr_stack_top:
	.size	kernel_isr_stack_base,	.-kernel_isr_stack_base
	.size	kernel_isr_stack_top,	.-kernel_isr_stack_base
ecode:
	.quad	0
	.size	ecode,				.-ecode
errinst:
	.quad	0
	.size	errinst,			.-errinst
svinst:
	.quad	0
	.size	svinst,				.-svinst
callback_8:
	.quad		no_waiting_op
	.size		callback_8,		.-callback_8
interrupted_state:
	.quad	0
	.size	interrupted_state,	.-interrupted_state
task_change_flag:
	.byte	0
	.size	task_change_flag,	.-task_change_flag
task_signal_code:
	.byte	0
	.size	task_signal_code,	.-task_signal_code
debug_stop_flag:
	.byte	0
	.size	debug_stop_flag,	.-debug_stop_flag
delay_flag:
	.byte	0
	.size	delay_flag,			.-delay_flag
task_lock_flag:
	.byte		0
	.size		task_lock_flag,			.-task_lock_flag
	.align	8
.Ltrampoline_return:
	.quad	0
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
	.macro			wrapper		i
		.global		isr_\i
		.type		isr_\i,		@function
		.p2align	4
		isr_\i:
			pushq	%rax
			movq	$0f,		.Ltrampoline_return
	//	The vectors 8, 10, 11, 12, 13, 14, 17, 21, 29, and 30 correspond to hardware exceptions that push an error code to the stack.
			.ifeq	(\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
			movq	8(%rsp),	%rax
			movq	%rax,		ecode
			movq	16(%rsp),	%rax
			movq	%rax,		errinst
			.else
			movq	8(%rsp),	%rax
			movq	%rax,		svinst
			.endif
			movq	$\i,		%rax
			jmp		isr_trampoline
		0:
			.ifeq	(\i-8)*(\i-10)*(\i-11)*(\i-12)*(\i-13)*(\i-14)*(\i-17)*(\i-21)*(\i-29)*(\i-30)
	//	If there is an error code on the stack, attempting to return will cause another fault. Therefore "pop" it by adding 8 to the stack pointer.
			addq	$8,			%rsp
			.endif
			cmpb	$0,			task_signal_code
	//	In usermode, a hardware exception translates to a signal (the scheduler will register an ISR callback that handles this).
			jnz		set_task_signal
	//	If we encountered a hardware exception in kernel code, that's a bug and we need to stop.
	//	This flag will be set by the kernel's ISR callback in such a case, after information regarding the error is logged.
			cmpb	$0,			debug_stop_flag
	//	Other than the instruction and stack pointers, the processor state will be the same as it was when the error was encountered once the stop is entered.
	//	The stack pointer will be underneath the faulting instruction's address on the kernel's ISR stack.
			jnz		debug_stop
			cmpb	$0,			task_change_flag
	//	The scheduler will set this flag when a task switch is set to occur. The code for task_change lives in task_amd64.S and handles that procedure.
			movb	$0,			task_change_flag
			jnz		task_change
			sti
			iretq
		.size		isr_\i,		.-isr_\i
	.endm
	.macro			wr_id		i
		.quad		isr_\i
	.endm
	xm256			wrapper
isr_trampoline:
	//	At this point, the A register from the interrupted code is on top of the stack, and the interrupt vector number is now in that register.
	//	The order in which these pushes occur is meant to group registers by their purpose.
	pushq		%rdi
	pushq		%rsi
	pushq		%rdx
	pushq		%rcx
	pushq		%r8
	pushq		%r9
	pushq		%r10
	pushq		%r11
	pushq		%rbx
	pushq		%r12
	pushq		%r13
	pushq		%r14
	pushq		%r15
	pushq		%rbp
	movq		%rsp,			interrupted_state
	movq		%rax,			%rdi
	call		isr_dispatch
	popq		%rbp
	popq		%r15
	popq		%r14
	popq		%r13
	popq		%r12
	popq		%rbx
	popq		%r11
	popq		%r10
	popq		%r9
	popq		%r8
	popq		%rcx
	popq		%rdx
	popq		%rsi
	popq		%rdi
	popq		%rax
	jmp			*.Ltrampoline_return(%rip)
	.size		isr_trampoline,		.-isr_trampoline
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
	.section	.rodata
	.global		isr_table
	.type		isr_table,		@object
isr_table:
	xm256		wr_id
	.size		isr_table,		.-isr_table
