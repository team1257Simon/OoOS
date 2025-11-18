#include "def-amd64.inc"
// task_amd64.S
// Most of the low-level task and scheduling related code (especially state save/load for task changes) lives here.
// The syscall dispatcher entry point is in syscall_amd64.S, under syscalls.
	.code64
	.text
	.global	fallthrough_reentry
	.global	task_change
	.global	user_entry
	.global	kernel_reentry
	.global	enable_fs_gs_insns
	.global	set_task_signal
	.extern	force_signal
	.global	worker_exit
	.extern worker_finish
	.global	start_worker
	.extern worker_add
	.extern	task_signal_code
	.extern	kernel_isr_stack_top
	.extern	kernel_cr3
	.extern kproc
	.type	fallthrough_reentry,	@function
	.type	task_change,			@function
	.type	user_entry,				@function
	.type	kernel_reentry,			@function
	.type	enable_fs_gs_insns,		@function
	.type	set_task_signal,		@function
	.type	force_signal,			@function
	.type	worker_exit,			@function
	.type	start_worker,			@function
	.type	worker_add,				@function
	.type	worker_finish,			@function
	.type	task_signal_code,		@object
	.type	kernel_isr_stack_top,	@object
	.type	kernel_cr3,				@object
fallthrough_reentry:
	swapgs
	movq	%rdi,					%gs:T_OFFS(next)
	movq	$kernel_isr_stack_top,	%rsp
	jmp	1f
task_change:
	isr_stack_save
	base_state_save
1:
	movq		%gs:T_OFFS(next),		%rax
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
	movq		%gs:T_OFFS(self),		%rdi
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
enable_fs_gs_insns:
	movq		%cr4,		%rax
	orl			$0x10000,	%eax
	movq		%rax,		%cr4
	ret
	.size		enable_fs_gs_insns,		.-enable_fs_gs_insns
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
