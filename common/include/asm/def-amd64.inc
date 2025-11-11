#include "x86_64-defs.inc"
.altmacro
.macro      syscall_state_save
	movq    %rsp,                   %gs:T_OFFS(rsp)
	movq    %rbp,                   %gs:T_OFFS(rbp)
	movq    %rcx,                   %gs:T_OFFS(rip)
	movq    %rbx,                   %gs:T_OFFS(rbx)
	movq    %rdx,                   %gs:T_OFFS(rdx)
	movq    %rdi,                   %gs:T_OFFS(rdi)
	movq    %rsi,                   %gs:T_OFFS(rsi)
	movq    %r8,                    %gs:T_OFFS(r8)
	movq    %r9,                    %gs:T_OFFS(r9)
	movq    %r10,                   %gs:T_OFFS(r10)
	movq    %r11,                   %gs:T_OFFS(rflags)
	movq    %r12,                   %gs:T_OFFS(r12)
	movq    %r13,                   %gs:T_OFFS(r13)
	movq    %r14,                   %gs:T_OFFS(r14)
	movq    %r15,                   %gs:T_OFFS(r15)
	fxsave  %gs:T_OFFS(fxsv)
.endm
.macro      syscall_state_restore
	fxrstor %gs:T_OFFS(fxsv)
	movq    %gs:T_OFFS(rdx),        %rdx
	movq    %gs:T_OFFS(rdi),        %rdi
	movq    %gs:T_OFFS(rsi),        %rsi
	movq    %gs:T_OFFS(r8),         %r8
	movq    %gs:T_OFFS(r9),         %r9
	movq    %gs:T_OFFS(r10),        %r10
	movq    %gs:T_OFFS(rflags),     %r11    
	movq    %gs:T_OFFS(r12),        %r12
	movq    %gs:T_OFFS(r13),        %r13
	movq    %gs:T_OFFS(r14),        %r14
	movq    %gs:T_OFFS(r15),        %r15
	movq    %gs:T_OFFS(rbp),        %rbp
	movq    %gs:T_OFFS(rsp),        %rsp
	movq    %gs:T_OFFS(rbx),        %rbx
	movq    %gs:T_OFFS(rip),        %rcx
.endm
.macro      base_state_save
	movq    %rbx,                   %gs:T_OFFS(rbx)
	movq    %rcx,                   %gs:T_OFFS(rcx)
	movq    %rdx,                   %gs:T_OFFS(rdx)
	movq    %rdi,                   %gs:T_OFFS(rdi)
	movq    %rsi,                   %gs:T_OFFS(rsi)
	movq    %r8,                    %gs:T_OFFS(r8)
	movq    %r9,                    %gs:T_OFFS(r9)
	movq    %r10,                   %gs:T_OFFS(r10)
	movq    %r11,                   %gs:T_OFFS(r11)
	movq    %r12,                   %gs:T_OFFS(r12)
	movq    %r13,                   %gs:T_OFFS(r13)
	movq    %r14,                   %gs:T_OFFS(r14)
	movq    %r15,                   %gs:T_OFFS(r15)
	movq    %rbp,                   %gs:T_OFFS(rbp)
	fxsave  %gs:T_OFFS(fxsv)
.endm
.macro      base_state_restore
	fxrstor %gs:T_OFFS(fxsv)
	movq    %gs:T_OFFS(rbx),        %rbx
	movq    %gs:T_OFFS(rcx),        %rcx
	movq    %gs:T_OFFS(rdx),        %rdx
	movq    %gs:T_OFFS(rdi),        %rdi
	movq    %gs:T_OFFS(rsi),        %rsi
	movq    %gs:T_OFFS(r8),         %r8
	movq    %gs:T_OFFS(r9),         %r9
	movq    %gs:T_OFFS(r10),        %r10
	movq    %gs:T_OFFS(r11),        %r11
	movq    %gs:T_OFFS(r12),        %r12
	movq    %gs:T_OFFS(r13),        %r13
	movq    %gs:T_OFFS(r14),        %r14
	movq    %gs:T_OFFS(r15),        %r15
	movq    %gs:T_OFFS(rbp),        %rbp
.endm
.macro      isr_stack_save
	pushq   %rax
	movq    kernel_cr3,             %rax
	movq    %rax,                   %cr3
	popq    %rax
	movq    %rax,                   %gs:T_OFFS(rax)
	popq    %rax    
	movq    %rax,                   %gs:T_OFFS(rip)
	popq    %rax
	movw    %ax,                    %gs:T_OFFS(cs)
	popq    %rax
	movq    %rax,                   %gs:T_OFFS(rflags)
	popq    %rax
	movq    %rax,                   %gs:T_OFFS(rsp)
	popq    %rax
	movw    %ax,                    %gs:T_OFFS(ss)
	movw    %ds,                    %ax
	movw    %ax,                    %gs:T_OFFS(ds)
.endm
.macro      isr_stack_restore
	movw    %gs:T_OFFS(ss),         %ax
	pushq   %rax
	movq    %gs:T_OFFS(rsp),        %rax
	pushq   %rax
	movq    %gs:T_OFFS(rflags),     %rax
	pushq   %rax
	movw    %gs:T_OFFS(cs),         %ax
	pushq   %rax
	movq    %gs:T_OFFS(rip),        %rax
	pushq   %rax
	movw    %gs:T_OFFS(ds),         %ax       
	movw    %ax,                    %ds
	movw    %ax,                    %es
	movq    %gs:T_OFFS(rax),        %rax
	pushq   %rax
	movq    %gs:T_OFFS(cr3),        %rax
	movq    %rax,                   %cr3
	popq    %rax
.endm