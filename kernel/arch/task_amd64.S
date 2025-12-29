#include "def-amd64.inc"
// task_amd64.S
// Most of the low-level task and scheduling related code (especially state save/load for task changes) lives here.
// The syscall dispatcher entry point is in syscall_amd64.S, under syscalls.
	.code64
	.global		fallthrough_reentry
	.global		task_change
	.global		user_entry
	.global		kernel_reentry
	.global		set_task_signal
	.global		worker_exit
	.global		worker_return
	.global		start_worker
	.extern		force_signal
	.extern		worker_add
	.extern		worker_finish
	.extern		task_signal_code
	.extern		kernel_isr_stack_top
	.extern		kernel_cr3
	.extern		kproc
	.type		fallthrough_reentry,	@function
	.type		task_change,			@function
	.type		user_entry,				@function
	.type		kernel_reentry,			@function
	.type		set_task_signal,		@function
	.type		worker_exit,			@function
	.type		worker_return,			@function
	.type		start_worker,			@function
	.type		force_signal,			@function
	.type		worker_add,				@function
	.type		worker_finish,			@function
	.type		task_signal_code,		@object
	.type		kernel_isr_stack_top,	@object
	.type		kernel_cr3,				@object
	.type		kproc,					@object
	.text
fallthrough_reentry:
	movq		%rdi,					%gs:T_OFFS(next)
	movq		$kernel_isr_stack_top,	%rsp
	jmp			1f
task_change:
	isr_stack_save
	base_state_save
1:
	movq		%gs:T_OFFS(next),		%rax
	//	As kproc is a global kernel symbol, residing in the data segment (the lowest one), it will always fit into 32 bits.
	//	Since this is an immediate operand, we can't encode this instruction to more than 32 bits.
	//	This is hot code, so every instruction counts. I also want to avoid clobbering another register for convenience's sake.
	//	Therefore we use the immediate to save us from having to perform an additional load into another register.
	cmpl		$kproc,					%gs:T_OFFS(self)
	jne			2f
	swapgs
2:
	wrgsbase	%rax
	movq		%gs:T_OFFS(thread_ptr),	%rax
	wrfsbase	%rax
	base_state_restore
	isr_stack_restore
	iretq
	.size		task_change,			.-task_change
	.size		fallthrough_reentry,	.-1b+task_change-fallthrough_reentry
user_entry:
	cli
	movq		%rax,					%gs:T_OFFS(rax)
	popq		%rax
	movq		%rax,					%gs:T_OFFS(rip)
	pushfq
	popq		%rax
	movq		%rax,					%gs:T_OFFS(rflags)
	base_state_save
	movq		%rsp,					%gs:T_OFFS(rsp)
	movw		%ds,					%ax
	movw		%ax,					%gs:T_OFFS(ds)
	movw		%ss,					%ax
	movw		%ax,					%gs:T_OFFS(ss)
	movw		%cs,					%ax
	movw		%ax,					%gs:T_OFFS(cs)
	swapgs
	movw		T_OFFS(ds)(%rdi),		%ax
	movw		%ax,					%ds
	movw		%ax,					%es
	movw		%ax,					%fs
	movw		%ax,					%gs
	wrgsbase	%rdi
	movq		%gs:T_OFFS(thread_ptr),	%rax
	wrfsbase	%rax
	syscall_state_restore
	movq		%gs:T_OFFS(cr3),		%rax
	movq		%rax,					%cr3
	xorq		%rax,					%rax
	sysretq
	.size		user_entry,			 .-user_entry
kernel_reentry:
	leaq		kproc,					%rdi
	movw		T_OFFS(ds)(%rdi),		%ax
	movw		%ax,					%ds
	movw		%ax,					%es
	movw		%ax,					%fs
	movw		%ax,					%gs
	wrgsbase	%rdi
	movq		%gs:T_OFFS(thread_ptr),	%rax
	wrfsbase	%rax
	base_state_restore
	movq		%gs:T_OFFS(rsp),		%rsp
	movq		%gs:T_OFFS(rflags),		%rax
	pushq		%rax
	popfq
	movq		%gs:T_OFFS(rip),		%rax
	pushq		%rax
	movq		%gs:T_OFFS(rax),		%rax
	sti
	ret
	.size		kernel_reentry,			.-kernel_reentry
set_task_signal:
	isr_stack_save
	base_state_save
	movq		%gs:T_OFFS(self),		%rdi
	xorl		%esi,					%esi
	lock xchgb	%sil,					task_signal_code
	call		force_signal
	base_state_restore
	isr_stack_restore
	iretq
	.size		set_task_signal,		.-set_task_signal
worker_exit:
	cli
	movq		%gs:T_OFFS(self),	%rdi
	movq		%rax,				%rsi
	jmp			worker_finish
	.size		worker_exit,			.-worker_exit
worker_return:
	movq		%rsi,		%rax
	movq		8(%rdi),	%rbp
	movq		48(%rdi),	%rsp
	pushq		56(%rdi)
	movq		0(%rdi),	%rbx
	movq		16(%rdi),	%r12
	movq		24(%rdi),	%r13
	movq		32(%rdi),	%r14
	movq		40(%rdi),	%r15
	ret
	.size		worker_return,			.-worker_return
start_worker:
	leaq		T_SIZE(%rdi),			%rsi
	movq		%rbx,					0(%rsi)
	movq		%rbp,					8(%rsi)
	movq		%r12,					16(%rsi)
	movq		%r13,					24(%rsi)
	movq		%r14,					32(%rsi)
	movq		%r15,					40(%rsi)
	leaq		8(%rsp),				%rax
	movq		%rax,					48(%rsi)
	movq		(%rsp),					%rax
	movq		%rax,					56(%rsi)
	jmp			worker_add
	.size		start_worker,			.-start_worker
