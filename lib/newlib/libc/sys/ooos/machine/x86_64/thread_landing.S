	.extern exit_thread
	.global thread_landing
	.type	exit_thread,	@function
	.type	thread_landing,	@function
	.text
thread_landing:
	movq	%rax,			%rdi
	jmp		exit_thread
	.size	thread_landing,	.-thread_landing
