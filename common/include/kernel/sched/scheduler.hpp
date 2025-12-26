#ifndef __SCHED
#define __SCHED
#include <arch/cpu_time.hpp>
#include <sched/task_queue.hpp>
#include <sched/deferred_action.hpp>
#include <atomic>
#include <vector>
class scheduler
{
	prio_level_task_queues __queues;
	task_wait_queue __sleepers;
	std::vector<kthread_ptr> __non_timed_sleepers;
	bool __running;
	size_t __total_tasks;
	ooos::deferred_action_queue __deferred_actions;
	static scheduler __instance;
	static bool __has_init;
	bool __set_wait_time(kthread_ptr& task, clock_t time, bool can_interrupt);
	bool __set_untimed_wait(kthread_ptr& task);
	void __do_task_change(kthread_ptr& cur, kthread_ptr& next);
protected:
	kthread_ptr select_next();
	void on_tick();
	__nointerrupts bool init() noexcept;
	scheduler();
public:
	constexpr bool is_running() const noexcept  { return __running; }
	constexpr void start() noexcept             { __running = true; }
	constexpr void stop() noexcept              { __running = false; }
	scheduler(scheduler const&) = delete;
	scheduler& operator=(scheduler const&) = delete;
	void register_task(kthread_ptr const& task);
	bool unregister_task(kthread_ptr const& task);
	bool unregister_task(task_t* task);
	bool set_wait_untimed(kthread_ptr& task);
	bool interrupt_wait(kthread_ptr& waiting);
	bool interrupt_wait(task_t* task);
	bool set_wait_timed(kthread_ptr& task, clock_t time, bool can_interrupt = true);
	void retrothread(task_t* task, thread_t* thread);
	kthread_ptr yield();
	kthread_ptr fallthrough_yield();
	static bool init_instance() noexcept;
	static bool has_init() noexcept;
	static scheduler& get() noexcept;
	static void add_worker_task(kthread_ptr const& worker);
	static void remove_worker_task(kthread_ptr const& worker);
	static pid_t active_pid() noexcept;
	static inline time_t ms_to_ticks(time_t ms) noexcept { return __instance.__deferred_actions.compute_ticks(ms); }
	template<ooos::__internal::__extended_runnable FT> static inline void defer_millis(time_t delay_ms, FT&& action) { __instance.__deferred_actions.add(delay_ms, std::forward<FT>(action)); }
	template<ooos::__internal::__extended_runnable FT> static inline void defer_sec(time_t delay_sec, FT&& action) { defer_millis(delay_sec * 1000UL, std::forward<FT>(action)); }
};
void write_task_base(task_t const& next);
#define sch scheduler::get()
#endif