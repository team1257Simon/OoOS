#ifndef __THREAD
#define __THREAD
#if (defined(__KERNEL__) || defined(__LIBK__))
#include <sched/task.h>
#include <util/pool_allocator.hpp>
namespace ooos { typedef std::vector<addr_t, pool_allocator<addr_t>> dynamic_thread_vector; }
typedef execution_state thread_state;
#else
namespace ooos { typedef addr_t dynamic_thread_vector; }
typedef uint8_t thread_state;
#endif
struct thread_ctl
{
	thread_state state;			// current state
	bool park;					// parked threads are waiting, potentially with a timer
	bool non_timed_park;		// indicates a parked thread has no wait timer and will only wake when notified
	spinlock_t thread_lock;		// modifications to the thread's data should lock this first
	uint32_t thread_id;			// unique identifier within a process
	clock_t wait_time_delta;	// amount of ticks remaining in wait time
};
struct __align(16) thread_t
{
	thread_t* self;							// thread self-pointer; this is also the end of the static TLS block
	ooos::dynamic_thread_vector* dtv_ptr;	// pointer to the thread's dynamic TLS vector
	size_t dtv_len;							// length of the thread's dynamic TLS vector in entries; will be at least 1 if there is a DTV at all
	regstate_t saved_regs;					// local register state for the thread
	fx_state fxsv;							// local floating point state for the thread
	thread_ctl ctl_info;					// thread scheduling and control info
	addr_t stack_base;						// pointer to the thread's local stack
	size_t stack_size;						// size of the local stack
};
#if (defined(__KERNEL__) || defined(__LIBK__))
namespace ooos
{
	typedef std::unordered_map<uint32_t, dynamic_thread_vector> dtv_by_thread_id;
	class task_dtv
	{
		dtv_by_thread_id __dtv_map;
		pool_allocator<addr_t> __dtv_alloc;
		void __lthread(thread_t& t);
		void __ulthread(thread_t& t);
	public:
		std::vector<ptrdiff_t> base_offsets;
		task_dtv();
		void instantiate(thread_t& thread);
		bool takedown(thread_t& thread);
	};
}
#endif
#endif