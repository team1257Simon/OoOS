
  .global setjmp
  .global longjmp
  .type   setjmp,   @function
  .type   longjmp,  @function
setjmp:
  movq %rbx, 0(%rdi)
  movq %rbp, 8(%rdi)
  movq %r12, 16(%rdi)
  movq %r13, 24(%rdi)
  movq %r14, 32(%rdi)
  movq %r15, 40(%rdi)
  leaq 8(%rsp), %rax
  movq %rax, 48(%rdi)
  movq (%rsp), %rax
  movq %rax, 56(%rdi)
  movq $0, %rax
  ret
  .size setjmp, .-setjmp
longjmp:
  movq %rsi, %rax
  movq 8(%rdi), %rbp
  movq 48(%rdi), %rsp
  pushq 56(%rdi)
  movq 0(%rdi), %rbx
  movq 16(%rdi), %r12
  movq 24(%rdi), %r13
  movq 32(%rdi), %r14
  movq 40(%rdi), %r15
  ret
  .size longjmp, .-longjmp
