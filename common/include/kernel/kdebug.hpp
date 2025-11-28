#ifndef __KDBG
#define __KDBG
#include "libk_decls.h"
#include "atomic"
#include "string"
extern std::atomic<bool> dbg_hold;
extern "C"
{
    void debug_print_num(uintptr_t num, int lenmax = 16);
    void debug_print_addr(addr_t addr);
    void direct_write(const char* str);
    void direct_writeln(const char* str);
	void dwendl();
	void dwclear();
	void direct_putch(wchar_t ch);
	extern struct __pack interrupt_stack
	{
		register_t r15;
		register_t r14;
		register_t r13;
		register_t r12;
		register_t rbx;
		register_t r11;
		register_t r10;
		register_t r9;
		register_t r8;
		register_t rcx;
		register_t rdx;
		register_t rsi;
		register_t rdi;
		register_t rax;
		register_t ecode;
		register_t rip;
		register_t cs;
		register_t rflags;
		register_t rsp;
		register_t ss;
	} *interrupted_state;
}
void xdirect_write(std::string const& str);
void xdirect_writeln(std::string const& str);
#endif