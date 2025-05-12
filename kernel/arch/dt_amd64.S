    .code64
    .data
    .global     gdt_setup
    .global     idt_register
    .extern     idt_set_entry
    .global     kernel_stack_base
    .global     kernel_stack_top
    .global     gdt_table
    .global     gdt_descriptor
    .global     system_tss
    .global     idt_table
    .extern     idt_descriptor
    .type       gdt_setup,              @function
    .type       idt_register,           @function
    .type       idt_set_entry,          @function
    .type       kernel_stack_base,      @object
    .type       kernel_stack_top,       @object
    .type       gdt_table,              @object
    .type       gdt_descriptor,         @object
    .type       system_tss,             @object
    .type       local_tss_descr,        @object
    .type       idt_table,              @object
    .type       idt_descriptor,         @object
    .align      4096
kernel_stack_base:
    .zero       65536
kernel_stack_top:
    .size       kernel_stack_base,      .-kernel_stack_base
    .size       kernel_stack_top,       .-kernel_stack_top
    .align      4
system_tss:
    .zero       102
    .word       104
    .size       system_tss,             .-system_tss
gdt_table:                              // The actual GDT
    .quad       0                       // Null Descriptor
    .quad       0x00AF9A000000FFFF      // Code Segment for Ring 0; offset 0x8
    .quad       0x00CF93000000FFFF      // Data Segment for Ring 0; offset 0x10
    .quad       0x00CFF3000000FFFF      // Data Segment for Ring 3; offset 0x18
    .quad       0x00AFFA000000FFFF      // Code Segment for Ring 3; offset 0x20
local_tss_descr:
    .zero       16
    .zero       4040
    .size       local_tss_descr,        .-local_tss_descr
    .size       gdt_table,              .-gdt_table
gdt_descriptor:                         // The GDT pointer struct
    .word       0x0FFF
    .quad       gdt_table
    .size       gdt_descriptor,         .-gdt_descriptor
idt_table:
    .zero       4096
    .size       idt_table,              .-idt_table
    .text
gdt_setup:
    leaq        local_tss_descr,        %rdi
    leaq        system_tss,             %rsi
	movq	    %rsi,                   %rax
	movb	    %sil,                   2(%rdi)
	movw	    $0x68,                  (%rdi)
	movq	    %rsi,                   %rdx
	shrq	    $16,                    %rdx
	movb	    %ah,                    3(%rdi)
	movb	    %dl,                    4(%rdi)
	movq	    %rsi,                   %rdx
	shrq	    $24,                    %rdx
	movw	    $0x4089,                5(%rdi)
	movb	    %dl,                    7(%rdi)
	shrq	    $32,                    %rsi
	movq	    %rsi,                   8(%rdi)
    lgdt        gdt_descriptor
    push        $0x8
    leaq        .Lreload_segments,      %rax
    pushq       %rax
    lretq
.Lreload_segments:
    movw        $0x10,                  %ax
    movw        %ax,                    %ds
    movw        %ax,                    %es
    movw        %ax,                    %fs
    movw        %ax,                    %gs
    movw        %ax,                    %ss
    movw        $0x28,                  %ax
    ltr         %ax
    ret
    .size       gdt_setup,              .-gdt_setup
idt_register:
    lidt        idt_descriptor
    movq        %cr4,                   %rax
    orq         $0x10000,               %rax
    movq        %rax,                   %cr4
    ret
    .size       idt_register,           .-idt_register
