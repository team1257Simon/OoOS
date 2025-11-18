#ifndef __THREAD
#define __THREAD
#include <sched/task.h>
#ifdef __cplusplus
extern "C" 
{
#endif
typedef struct {
	size_t gen_count;
	addr_t tls_ptrs[];
} dtv_t;
typedef struct {
	size_t vec_len;
	dtv_t dtv[];
} dtv_vec;
typedef struct
{
	bool park				CXX_INI(false);	// parked threads are waiting, potentially with a timer
	bool non_timed_park		CXX_INI(false);	// indicates a parked thread has no wait timer and will only wake when notified
	bool relaunch_on_exit;					// indicates the thread is a worker that should attempt to restart if it exits
	spinlock_t thread_lock	CXX_INI();		// modifications to the thread's data should lock this first
	uint32_t thread_id;						// unique identifier
	clock_t wait_time_delta CXX_INI(0UZ);	// amount of ticks remaining in wait time
} thread_ctl;
typedef struct __align(16) __thread_info
{
	addr_t self;					// thread self-pointer; this is also the end of the static TLS block
	task_t* proc_ptr;				// process back-pointer; should equal %gs:0
	thread_ctl ctl_info;			// thread scheduling and control info
	fx_state thread_fp_state;		// local floating point state for the thread
	regstate_t thread_cpu_state;	// local register state for the thread
	addr_t thread_stack_base;		// pointer to the thread's local stack
	size_t thread_stack_size;		// size of the local stack
	dtv_t* dtv_ptr;					// pointer to the thread's dynamic TLS vector
} thread_t;
#ifdef __cplusplus
}
#endif
#endif