    .code64
    .text
    .global read_tsc
    .global diff_tsc
    .global read_diff_tsc
    .type   read_tsc,       @function
    .type   diff_tsc,       @function
    .type   read_diff_tsc,  @function
read_tsc:
    rdtsc
    movl    %eax,           (%rdi)
    movl    %edx,           4(%rdi)
    ret
    .size   read_tsc,       .-read_tsc
diff_tsc:
    rdtsc
    shlq    $32,            %rdx
    orq     %rdx,           %rax
    subq    (%rdi),         %rax
    ret
    .size   diff_tsc,       .-diff_tsc
read_diff_tsc:
    movq    (%rsi),         %rcx
    rdtsc
    movl    %eax,           (%rdi)
    movl    %edx,           4(%rdi)
    subq    %rcx,           (%rdi)
    ret
    .size   read_diff_tsc,  .-read_diff_tsc
