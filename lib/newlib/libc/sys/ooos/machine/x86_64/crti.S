	.global 	_init
	.global 	_fini
	.type 		_init,	@function
	.type 		_fini, 	@function
	.section 	.init
_init:
	push 		%rbp
	movq 		%rsp, 	%rbp
	.section 	.fini
_fini:
	push 		%rbp
	movq 		%rsp, 	%rbp
