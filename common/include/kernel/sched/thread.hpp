#ifndef __THREAD
#define __THREAD
#include <sched/task.h>
extern "C" 
{
	struct dtv_t {
		size_t gen_count;
		addr_t tls_ptrs[];
	};
	struct dtv_vec {
		size_t vec_len;
		dtv_t dtv[];
	};
	struct thread_ctl
	{
		bool started;				// set to true once the thread is started
		bool park;					// parked threads are waiting, potentially with a timer
		bool non_timed_park;		// indicates a parked thread has no wait timer and will only wake when notified
		spinlock_t thread_lock;		// modifications to the thread's data should lock this first
		uint32_t thread_id;			// unique identifier
		clock_t wait_time_delta;	// amount of ticks remaining in wait time
	};
	struct __align(16) thread_t
	{
		addr_t self;					// thread self-pointer; this is also the end of the static TLS block
		task_t* proc_ptr;				// process back-pointer; should equal %gs:0
		thread_ctl ctl_info;			// thread scheduling and control info
		fx_state thread_fp_state;		// local floating point state for the thread
		regstate_t thread_cpu_state;	// local register state for the thread
		addr_t thread_stack_base;		// pointer to the thread's local stack
		size_t thread_stack_size;		// size of the local stack
		dtv_t* dtv_ptr;					// pointer to the thread's dynamic TLS vector
	};
}
#endif