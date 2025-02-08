	.section .init
	# GCC will nicely put the contents of crtend.o's .init section here.
	popq %rbp
	ret
	.section .fini
	# GCC will nicely put the contents of crtend.o's .fini section here.
	popq %rbp
	ret
