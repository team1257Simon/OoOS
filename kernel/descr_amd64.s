    .code64
    .section    .data
    .global     gdt_table
    .global     gdt_descriptor
    .global     system_tss
    .global     idt_table
    .global     idt_descriptor
    .type       gdt_table,          @object
    .type       gdt_descriptor,     @object
    .type       system_tss,         @object
    .type       local_tss_descr,    @object
    .type       idt_table,          @object
    .type       idt_descriptor,     @object
    .size       gdt_table,          4096
    .size       gdt_descriptor,     10
    .size       system_tss,         104
    .size       local_tss_descr,    16
    .size       idt_table,          4096
    .size       idt_descriptor,     10
system_tss:
    .zero       104
    # The actual GDT
gdt_table:
    .quad       0                     # Null Descriptor
    .quad       0x00AF9A000000FFFF    # Code Segment for Ring 0; offset 0x8
    .quad       0x00CF92000000FFFF    # Data Segment for Ring 0; offset 0x10
    .quad       0x00AFFA000000FFFF    # Code Segment for Ring 3; offset 0x18
    .quad       0x00CFF2000000FFFF    # Data Segment for Ring 3; offset 0x20
local_tss_descr:
    .zero       16
    .zero       4040
    # The GDT pointer struct
gdt_descriptor:
    .word       0x0FFF
    .quad       gdt_table
idt_table:
    .zero       4096
idt_descriptor:
    .word       0x0FFF
    .quad       idt_table
    .section    .text
    .global     gdt_setup
    .global     idt_register
    .global     idt_set_entry
    .type       gdt_setup,              @function
    .type       idt_register,           @function
    .type       idt_set_entry,          @function
    .type       fill_tss_descriptor,    @function
fill_tss_descriptor:
	movl	$104,       %edx
	movq	%rsi,       %rax
	movl	$16521,     %ecx
	movb	%sil,       2(%rdi)
	movw	%dx,        (%rdi)
	movq	%rsi,       %rdx
	shrq	$16,        %rdx
	movb	%ah,        3(%rdi)
	movb	%dl,        4(%rdi)
	movq	%rsi,       %rdx
	shrq	$24,        %rdx
	movw	%cx,        5(%rdi)
	movb	%dl,        7(%rdi)
	movq	%rsi,       %rdx
	movl	$104,       %esi
	shrq	$32,        %rdx
	movq	%rdx,       8(%rdi)
	movq	4(%rax),    %rdx
	movw	%si,        102(%rax)
	movq	%rdx,       44(%rax)
	ret
    .size   fill_tss_descriptor,  .-fill_tss_descriptor
gdt_setup:
    leaq    local_tss_descr,        %rdi
    leaq    system_tss,             %rsi
    movq    %rsp,                   4(%rsi)
    call    fill_tss_descriptor
    leaq    gdt_descriptor,         %rax
    lgdt    (%rax)
    push    $0x8
    leaq    local_reload_segments,  %rax
    pushq   %rax
    lretq
local_reload_segments:
    movw    $0x10,  %ax
    movw    %ax,    %ds
    movw    %ax,    %es
    movw    %ax,    %fs
    movw    %ax,    %gs
    movw    %ax,    %ss
    movw    $0x28,  %ax
    ltr     %ax
    ret
    .size       gdt_setup,    .-gdt_setup
idt_register:
    leaq        idt_descriptor, %rax
    lidt        (%rax)
    movq        %cr4,       %rax
    orq         $0x10000,   %rax
    movq        %rax,       %cr4
    ret
    .size       idt_register,   .-idt_register
