#ifndef __KDBG
#define __KDBG
#include "libk_decls.h"
#include "atomic"
extern std::atomic<bool> dbg_hold;
[[gnu::always_inline]] inline void debug_hold() { direct_write("[HOLD]"); dbg_hold = true; bool b; do { b = dbg_hold; } while(b); }
#endif