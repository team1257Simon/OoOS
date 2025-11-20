#include <iostream>
#ifdef __KERNEL__
#undef __KERNEL__
#endif
typedef long register_t;
typedef __int128_t int128_t;
#include "../common/include/kernel/sched/task.h"
#include "../common/include/kernel/sched/thread.hpp"
#define DEFS(m, n) std::cout << "#define __TASK_MEMBER_" #m "_OFFSET 0x" << offsetof(task_t, n) << "\n"
#define DEFLS(m, n) std::cout << "#define __THREAD_MEMBER_" #m "_OFFSET 0x" << offsetof(thread_t, n) << "\n"
#define DEF(n) DEFS(n, n)
#define DEFL(n) DEFLS(n, n)
#define DEFLC(n) DEFLS(n, ctl_info.n)
#define DEFR(r) DEFS(r, saved_regs.r)
#define DEFLR(r) DEFLS(r, saved_regs.r)
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
	std::cout << "#define T_OFFS(m) __TASK_MEMBER_##m##_OFFSET\n";
	std::cout << "#define T_SIZE 0x" << sizeof(task_t) << "\n";
	DEFL(self);
	DEFL(dtv_ptr);
	DEFLR(rax);
	DEFLR(rbx);
	DEFLR(rcx);
	DEFLR(rdx);
	DEFLR(rdi);
	DEFLR(rsi);
	DEFLR(r8);
	DEFLR(r9);
	DEFLR(r10);
	DEFLR(r11);
	DEFLR(r12);
	DEFLR(r13);
	DEFLR(r14);
	DEFLR(r15);
	DEFLR(rsp);
	DEFLR(rbp);
	DEFLR(rip);
	DEFLR(rflags);
	DEFLR(ds);
	DEFLR(ss);
	DEFLR(cs);
	DEFLR(cr3);
	DEFL(fxsv);
	DEFLC(state);
	DEFLC(park);
	DEFLC(non_timed_park);
	DEFLC(thread_lock);
	DEFLC(thread_id);
	DEFLC(wait_time_delta);
	DEFL(stack_base);
	DEFL(stack_size);
	std::cout << "#define L_OFFS __THREAD_MEMBER_##m##_OFFSET\n";
	std::cout << "#define L_SIZE 0x" << sizeof(thread_t) << std::endl;
	return 0;
}
