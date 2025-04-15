	.code64
	.text
	.global	sigtramp_enter
	.global	sigtramp_return
    .global signal_exit
    .global sigtramp_code
	.type	sigtramp_enter,     @function
	.type	sigtramp_return,    @function
    .type   signal_exit,        @function
    .type   sigtramp_code,      @object
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
    orl     $0x80000000,        %edi
    syscall
    .align  4096
    .size   sigtramp_code,      .-sigtramp_code
