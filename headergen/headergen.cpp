#include <iostream>
#ifdef __KERNEL__
#undef __KERNEL__
#endif
typedef long register_t;
typedef __int128_t int128_t;
#include "../common/include/kernel/sched/task.h"
#define DEFS(m, n) std::cout << "#define __TASK_MEMBER_" #m "_OFFSET 0x" << offsetof(task_t, n) << "\n"
#define DEF(n) DEFS(n, n)
#define DEFR(r) DEFS(r, saved_regs.r)
int main()
{
	std::cout << std::hex << std::uppercase;
	DEF(self);
	DEF(frame_ptr);
	DEFR(rax);
	DEFR(rbx);
	DEFR(rcx);
	DEFR(rdx);
	DEFR(rdi);
	DEFR(rsi);
	DEFR(r8);
	DEFR(r9);
	DEFR(r10);
	DEFR(r11);
	DEFR(r12);
	DEFR(r13);
	DEFR(r14);
	DEFR(r15);
	DEFR(rsp);
	DEFR(rbp);
	DEFR(rip);
	DEFR(rflags);
	DEFR(ds);
	DEFR(ss);
	DEFR(cs);
	DEFR(cr3);
	DEF(fxsv);
	DEF(run_split);
	DEF(run_time);
	DEF(sys_time);
	DEF(tls_master);
	DEF(tls_size);
	DEF(thread_ptr);
	DEF(num_child_procs);
	DEF(child_procs);
	DEF(next);
	std::cout << "#define T_OFFS(m) __TASK_MEMBER_##m##_OFFSET" << std::endl;
	std::cout << "#define T_SIZE 0x" << sizeof(task_t) << std::endl;
	return 0;
}
