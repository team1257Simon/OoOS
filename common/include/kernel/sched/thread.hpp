#ifndef __THREAD
#define __THREAD
#if (defined(__KERNEL__) || defined(__LIBK__))
#include <sched/task.h>
#include <util/pool_allocator.hpp>
namespace ooos { typedef std::vector<addr_t, pool_allocator<addr_t>> dynamic_thread_vector; struct user_callback_base; }
enum class thread_state : uint8_t
{
	STOPPED		= 0UC,	// not running (yet)
	RUNNING		= 1UC,	// running or runnable but not active (due to scheduling)
	CANCELLED	= 2UC,	// cancelled and due to exit depending on cancel type
	TERMINATED	= 3UC,	// finished execution, normally or otherwise
};
#else
namespace ooos { typedef addr_t dynamic_thread_vector; struct user_callback_base; }
enum class thread_state : uint8_t
{
	STOPPED		= 0,	// not running (yet)
	RUNNING		= 1,	// running or runnable but not active (due to scheduling)
	CANCELLED	= 2,	// cancelled and due to exit depending on cancel type
	TERMINATED	= 3,	// finished execution, normally or otherwise
};
#endif
typedef void (*callback_reset_fn)(void*);
struct thread_ctl
{
	volatile struct
	{
		thread_state state		: 2;		// current state
		bool park				: 1;		// parked threads are waiting, potentially with a timer
		bool non_timed_park		: 1;		// indicates a parked thread has no wait timer and will only wake when notified
		bool detached			: 1;		// indicates the thread should immediately terminate upon exit rather than wait for a join
		bool cancel_disable		: 1;		// if true, the thread ignores cancellation
		bool cancel_async		: 1;		// if true, cancellation occurs as soon as possible; if false, it waits until a cancel point function is called
		bool retrigger_capable	: 1;		// if true, the thread is a repeatable callback with a reset function, which is pointed to by reset
	};
	struct
	{
		spinlock_t thread_lock;							// modifications to the thread's data should lock this first
		pid_t thread_id;								// unique identifier within a process
		clock_t wait_time_delta;						// amount of ticks remaining in wait time
		callback_reset_fn reset;						// called to reset the thread's state if applicable
		ooos::user_callback_base* callback_handle;		// if this thread is a callback, this is a handle to the callback's data structure
	};
};
struct __align(16) thread_t
{
	thread_t* self;							// thread self-pointer; this is also the end of the static TLS block
	ooos::dynamic_thread_vector* dtv_ptr;	// pointer to the thread's dynamic TLS vector
	regstate_t saved_regs;					// local register state for the thread
	fx_state fxsv;							// local floating point state for the thread
	thread_ctl ctl_info;					// thread scheduling and control info
	size_t stack_size;						// size of the local stack
	addr_t stack_base;						// pointer to the thread's local stack
	addr_t tls_start;						// start of the TLS block, accounting for alignup operations
};
enum class join_result
{
	IMMEDIATE,
	DEFER,
	NXTHREAD
};
#if (defined(__KERNEL__) || defined(__LIBK__))
namespace ooos
{
	typedef std::unordered_map<pid_t, dynamic_thread_vector> dtv_by_thread_id;
	class task_dtv
	{
		dtv_by_thread_id __dtv_map;
		pool_allocator<addr_t> __dtv_alloc;
	public:
		std::vector<ptrdiff_t> base_offsets;
		task_dtv();
		task_dtv(task_dtv&&) = default;
		task_dtv(task_dtv const& that);		// copy-constructing only copies the base offset vector (i.e. the thread pool remains separate)
		void instantiate(thread_t& thread);
		bool takedown(thread_t& thread);
	};
	bool test_thread_mutex(thread_t const& t);
	void lock_thread_mutex(thread_t& t);
	void unlock_thread_mutex(thread_t& t);
	void update_thread_state(thread_t& thread, task_t& task_struct);
}
extern "C"
{
	[[noreturn]] void syscall_threadexit(register_t retval);
	spid_t syscall_tfork();
	spid_t syscall_threadcreate(addr_t entry_pt, addr_t exit_pt, size_t stack_sz, bool start_detached, register_t arg);
	ssize_t syscall_getthreadcount();
	register_t syscall_threadjoin(pid_t thread);
	int syscall_threaddetach(pid_t thread);
}
#endif
#endif