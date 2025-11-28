#ifndef __TASK
#define __TASK
#include <sys/types.h>
#if !(defined(__KERNEL__) || defined(__LIBK__))
typedef int32_t spid_t;
typedef void* addr_t;
typedef unsigned long qword;
typedef struct spinlock_t { volatile bool : 8; } *mutex_t;
#define CXX_INI(...)
#define __pack __attribute__((packed))
#define __align(n) __attribute__((aligned(n)))
#define attribute(...) __attribute__((__VA_ARGS__))
#elif defined(__cplusplus)
#include <libk_decls.h>
typedef qword sigset_t;
#else
struct spinlock_t { bool value; };
typedef unsigned long sigset_t;
#endif
#ifdef __cplusplus
constexpr int num_signals = 64;
extern "C"
{
#else
#define num_signals 64
#endif
typedef void (*signal_handler)(int);
enum
#ifdef __cplusplus
class
#endif
priority_val
#ifdef __cplusplus
: int8_t
#endif
{
	PVLOW	= 0,
	PVNORM	= 1,
	PVHIGH	= 2,
	PVEXTRA = 3,
	PVSYS	= 4,
};
enum sigprocmask_action
{
	SIG_BLOCK	= 0,
	SIG_UNBLOCK = 1,
	SIG_SETMASK = 2
};
extern int8_t exception_signals[32];	// translation table for hardware exceptions to signal codes
struct attribute(packed, aligned(16)) fx_state
{
	uint16_t	fcw;
	uint16_t	fsw;
	uint8_t		ftw;
	uint8_t		rsv0;
	uint16_t	fop;
	uint64_t	fip;
	uint64_t	fdp;
	uint32_t	mxcsr;
	uint32_t	mxcsr_mask;
	long double stmm[8];
	int128_t	xmm[16];
	int128_t	rsv1[3];
	int128_t	avail[3];
};
struct attribute(packed, aligned(8)) regstate_t
{
	register_t	rax;
	register_t	rbx;
	register_t	rcx;
	register_t	rdx;
	register_t	rdi;
	register_t	rsi;
	register_t	r8;
	register_t	r9;
	register_t	r10;
	register_t	r11;
	register_t	r12;
	register_t	r13;
	register_t	r14;
	register_t	r15;
	addr_t		rbp;
	addr_t		rsp;
	addr_t		rip;
	addr_t		cr3;
	register_t	rflags;
	uint16_t	ds;
	uint16_t	ss;
	uint16_t	cs;
};
struct attribute(packed, aligned(16)) task_signal_info_t
{
	sigset_t			blocked_signals;
	sigset_t			pending_signals;
	struct fx_state		sigret_fxsave;
	signal_handler		signal_handlers[num_signals];
	struct regstate_t	sigret_frame;
	struct spinlock_t	sigmask_lock;
	int					active_signal;
};
struct __pack tctl_t
{
	volatile struct attribute(packed, aligned(1))
	{
		bool			block			: 1;		// true if the task is in a sleeping or waiting state
		bool			can_interrupt	: 1;		// true if the wait can be interrupted
		bool			should_notify	: 1;		// true if the wait should be interrupted on a child process termination
		bool			killed			: 1;		// true if the process has stopped due to an abnormal termination (e.g. kill signal)
		enum priority_val prio_base		: 4;		// the base priority of the thread/process
	};
	struct attribute(packed, aligned(1))
	{
		uint8_t						skips;				// the number of times the task has been skipped for a higher-priority one. The system will escalate a lower-priority process at the front of its queue with enough skips.
		struct task_signal_info_t*	signal_info;		// points to the signal info struct for the process (handled in the larger, encompassing c++ task_ctx structure)
		uint32_t					wait_ticks_delta;	// for a sleeping task, how many ticks remain in the set time as an offset from the previous waiting task (or from zero if it is the first waiting process)
		spid_t						parent_pid;			// a negative number indicates no parent; a zero here means the task is actually a kernel thread
		pid_t						task_id;			// PID of process; kernel itself is zero
		uid_t						task_uid;			// WIP
		gid_t						task_gid;			// WIP
	};
};
struct thread_t;
struct attribute(aligned(16)) task_t
{
	struct task_t*		self;								// self-pointer
	addr_t				frame_ptr;							// this will be a pointer to a uframe_tag struct for a userspace task. The kernel has its own frame pointer of a different type
	struct regstate_t	saved_regs;							// this stores all the register states when the process is not active
	uint16_t			quantum_val;						// base amount of time allocated per timeslice
	uint16_t			quantum_rem		CXX_INI(0US);		// amount of time remaining in the current timeslice
	struct tctl_t		task_ctl;							// this contains information related to the process' scheduling, as well as PID and signaling info
	struct fx_state		fxsv			CXX_INI();			// stored state for floating-point registers
	uint64_t			run_split		CXX_INI(0UL);		// timer-split of when the task began its most recent timeslice; when it finishes, the delta to the current time is added to the run time counter
	uint64_t			run_time		CXX_INI(0UL);		// total runtime
	uint64_t			sys_time		CXX_INI(0UL);		// approximate time in syscalls (measured via CPU timestamps)
	addr_t				tls_master;							// pointer to the t master TLS template block for the program
	size_t				tls_size;							// size of the TLS template block
	size_t				tls_align;							// alignment requirement for the TLS block
	struct thread_t*	thread_ptr;							// pointer to the current thread's info block
	size_t				num_child_procs;					// how many children are in the array below
	struct task_t**		child_procs;						// array of pointers to child process info structures (for things like process-tree termination)
	struct task_t*		next			CXX_INI(nullptr);	// updated when the scheduling event fires.
};
inline void fx_save(struct task_t* tx) { asm volatile("fxsave %0" : "=m"(tx->fxsv) :: "memory"); }
inline void fx_restore(struct task_t* tx) { asm volatile("fxrstor %0" : "=m"(tx->fxsv) :: "memory"); }
#ifdef __cplusplus
}
#endif
#if defined(__cplusplus) && (defined(__KERNEL__) || defined(__LIBK__))
// Set interrupt flag (bit 9); bit 1 must always be set; all other bits are clear
constexpr register_t ini_flags	= 0x00000202UL;
constexpr priority_val& operator++(priority_val& pv) noexcept
{
	using enum priority_val;
	int8_t qv			= static_cast<int8_t>(pv) + 1SC;
	return (pv			= static_cast<priority_val>(qv));
}
constexpr priority_val& operator--(priority_val& pv)
{
	using enum priority_val;
	int8_t qv			= static_cast<int8_t>(pv) - 1SC;
	return (pv			= static_cast<priority_val>(qv));
}
constexpr priority_val operator+(priority_val pv, int8_t i)
{
	using enum priority_val;
	int8_t qv			= static_cast<int8_t>(pv) + i;
	return static_cast<priority_val>(qv);
}
constexpr priority_val operator-(priority_val pv, int8_t i)
{
	using enum priority_val;
	int8_t qv			= static_cast<int8_t>(pv) - i;
	return static_cast<priority_val>(qv);
}
enum class execution_state : uint8_t
{
	STOPPED		= 0UC,	// not running (yet)
	RUNNING		= 1UC,	// running or runnable but not active (due to scheduling)
	TERMINATED	= 2UC,	// finished execution, normally or otherwise
	IN_DYN_EXIT = 3UC	// for processes with destructors to execute before fully exiting, this state indicates such code is running
};
/**
 * Pointer-like struct that is meant to behave abstractly like a pointer to a fake "kthread" structure.
 * That (exposure-only) object nominally encodes a process and, optionally, a thread within that process.
 * Certain scheduler functionality does not depend on the thread pointer at all.
 * Some other scheduler routines use information from the thread pointer if present or the corresponding process information otherwise.
 * In any case, it uses iterator-like logic for qualifiers, i.e. anything that involves dereferencing one of the pointers is const-qualified.
 */
struct kthread_ptr
{
	task_t* task_ptr;
	thread_t* thread_ptr;
	void activate() const noexcept;
	void set_blocking(bool can_interrupt) const noexcept;
	void clear_blocking() const noexcept;
	void set_wait_delta(uint32_t ticks) const noexcept;
	void add_wait_ticks(uint32_t ticks) const noexcept;
	bool is_blocking() const noexcept;
	bool is_interruptible() const noexcept;
	unsigned int get_wait_delta() const noexcept;
	void wait_tick() const noexcept;
	constexpr task_t* operator->() const noexcept { return task_ptr; }
	constexpr task_t& operator*() const noexcept { return *task_ptr; }
	constexpr operator bool() const noexcept { return static_cast<bool>(task_ptr); }
	constexpr bool operator==(kthread_ptr const& that) const noexcept { return this->task_ptr == that.task_ptr && this->thread_ptr == that.thread_ptr; }
	friend constexpr bool operator==(task_t* const& __this, kthread_ptr const& __that) noexcept { return __this == __that.task_ptr; }
	friend constexpr bool operator==(kthread_ptr const& __this, task_t* const& __that) noexcept { return __this.task_ptr == __that; }
	constexpr std::strong_ordering operator<=>(kthread_ptr const& that) const noexcept { return this->task_ptr == that.task_ptr ? this->thread_ptr <=> that.thread_ptr : this->task_ptr <=> that.task_ptr; }
	friend constexpr std::strong_ordering operator<=>(task_t* const& __this, kthread_ptr const& __that) noexcept { return __this <=> __that.task_ptr; }
	friend constexpr std::strong_ordering operator<=>(kthread_ptr const& __this, task_t* const& __that) noexcept { return __this.task_ptr <=> __that; }
};
#endif
#endif