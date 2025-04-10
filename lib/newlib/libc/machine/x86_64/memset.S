
  .global memset
  .type   memset, @function
memset:
  movq %rdi, %r9
  movq %rsi, %rax
  movq %rdx, %rcx
  cmpq $16, %rdx
  jb byte_set
  movq %rdi, %r8
  andq $7, %r8
  jz quadword_aligned
  movq $8, %rcx
  subq %r8, %rcx
  subq %rcx, %rdx
  rep stosb
  movq %rdx, %rcx
quadword_aligned:
  movabs $0x0101010101010101, %r8
  movzbl %sil, %eax
  imul %r8, %rax
  cmpq $256, %rdx
  jb quadword_set
  shrq $7, %rcx
  .p2align 4
loop:
  movntiq %rax,(%rdi)
  movntiq %rax, 8(%rdi)
  movntiq %rax, 16(%rdi)
  movntiq %rax, 24(%rdi)
  movntiq %rax, 32(%rdi)
  movntiq %rax, 40(%rdi)
  movntiq %rax, 48(%rdi)
  movntiq %rax, 56(%rdi)
  movntiq %rax, 64(%rdi)
  movntiq %rax, 72(%rdi)
  movntiq %rax, 80(%rdi)
  movntiq %rax, 88(%rdi)
  movntiq %rax, 96(%rdi)
  movntiq %rax, 104(%rdi)
  movntiq %rax, 112(%rdi)
  movntiq %rax, 120(%rdi)
  leaq 128(%rdi), %rdi
  dec %rcx
  jnz loop
  sfence
  movq %rdx, %rcx
  andq $127, %rcx
  rep stosb
  movq %r9, %rax
  ret
byte_set:
  rep stosb
  movq %r9, %rax
  ret
quadword_set:
  shrq $3, %rcx
  .p2align 4
  rep stosq
  movq %rdx, %rcx
  andq $7, %rcx
  rep stosb
  movq %r9, %rax
  ret
  .size memset, .-memset
