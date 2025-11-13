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
}
void xdirect_write(std::string const& str);
void xdirect_writeln(std::string const& str);
#endif