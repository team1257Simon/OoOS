	.code64
	.text
	.global	sigtramp_enter
	.global	sigtramp_return
    .global signal_exit
    .global sigtramp_code
	.global exception_signals
	.type	sigtramp_enter,     @function
	.type	sigtramp_return,    @function
    .type   signal_exit,        @function
    .type   sigtramp_code,      @object
	.type	exception_signals,	@object
    .align  4096
sigtramp_code:
sigtramp_enter:
	pushq   $sigtramp_return
	jmp     *%rsi
	.size	sigtramp_enter,     .-sigtramp_enter
	.p2align 4
sigtramp_return:
    movq    $35,                %rax
    syscall
	popq    %rcx
	popq    %r11
	ret
	.size	sigtramp_return,    .-sigtramp_return
    .p2align 4
signal_exit:
    xorq    %rax,               %rax
    syscall
	.size	signal_exit,		.-signal_exit
    .align  4096
    .size   sigtramp_code,      .-sigtramp_code
	.data
exception_signals:
	.byte 8, 5, 7, 5, 21, 21, 4, 8, 6, 11, 11, 11, 11, 11, 11, 0, 8, 10, 6, 8, 7, 11, 0, 0, 0, 0, 0, 0, 6, 6, 6, 0
	.size	exception_signals,	.-exception_signals
