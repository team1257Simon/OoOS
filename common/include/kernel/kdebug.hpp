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
}
void xdirect_write(std::string const& str);
void xdirect_writeln(std::string const& str);
void xklog(std::string const& str);
#define debug_hold() direct_write("|"); dbg_hold = true; do { asm volatile ("pause" ::: "memory"); } while(dbg_hold.load())
#endif