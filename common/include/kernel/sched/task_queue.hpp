#ifndef __TASK_QUEUE
#define __TASK_QUEUE
/**
 * Each priority level will have its own queue. When a scheduling event fires (e.g. the current process' timeslice expires),
 * if there are waiting processes, its "next" pointer is updated to the next process "in line" based on the following:
 *  1: If the queue with priority "system" has any active tasks, that queue will be accessed first.
 *  2: Otherwise, the highest numeric priority takes precedence. The "times skipped" value of the chosen non-system process is reset to zero at this point.
 *  3: If the process was selected from a queue of higher priority than its base value, it moves to the back of the queue one loweer than its current queue.
 *  4: Otherwise, the task queue element is "popped" and the pointer moves to the next element (or to the beginning if it is at the end). 
 *  5: The "times skipped" value of the front process in each queue of lower priority than the one accessed is incremented.
 *  6: If the above value exceeds the threshold (tbd), the process is moved to the back of the queue above its current queue.
 */
#include "sched/task_ctx.hpp"
#include "bits/stl_queue.hpp"
#include "array"
typedef std::ext::resettable_queue<kthread_ptr, std::allocator<kthread_ptr>> task_ptr_queue_base;
class task_pl_queue : public task_ptr_queue_base
{
	typedef task_ptr_queue_base __base;
	uint8_t __skips_threshold = 5UC;
public:
	using __base::value_type;
	using __base::allocator_type;
	using __base::size_type;
	using __base::difference_type;
	using __base::reference;
	using __base::const_reference;
	using __base::pointer;
	using __base::const_pointer;
	using __base::iterator;
	using __base::const_iterator;
	using __base::reverse_iterator;
	using __base::const_reverse_iterator;
	constexpr task_pl_queue() noexcept : __base{} {}
	constexpr task_pl_queue(size_type start_cap) : __base(start_cap) {}
	constexpr void set_skips_threshold(uint8_t value) noexcept { this->__skips_threshold = value; }
	void on_skipped() noexcept;
	bool skip_flag() noexcept;
	bool transfer_next(task_ptr_queue_base& to_whom);
};
class task_wait_queue : public task_ptr_queue_base
{
	typedef task_ptr_queue_base __base;
public:
	using __base::value_type;
	using __base::allocator_type;
	using __base::size_type;
	using __base::difference_type;
	using __base::reference;
	using __base::const_reference;
	using __base::pointer;
	using __base::const_pointer;
	using __base::iterator;
	using __base::const_iterator;
	using __base::reverse_iterator;
	using __base::const_reverse_iterator;
	constexpr task_wait_queue() noexcept : __base() { this->set_trim_stale(true); }
	constexpr task_wait_queue(size_type start_cap) : __base(start_cap) { this->set_trim_stale(true); }
	void tick_wait() noexcept;
	unsigned int next_remaining_wait_ticks() const noexcept;
	unsigned int cumulative_remaining_ticks() const noexcept;
	bool interrupt_wait(const_iterator where);
};
class prio_level_task_queues : public std::array<task_pl_queue, 5UZ>
{
	typedef std::array<task_pl_queue, 5UZ> __base;
	constexpr static __base::size_type __idx_by_prio(priority_val pv) { return static_cast<__base::size_type>(static_cast<int8_t>(pv)); }
public:
	task_pl_queue& operator[](priority_val pv) noexcept;
	task_pl_queue const& operator[](priority_val pv) const noexcept;
};
#endif