#ifndef __SCHED
#define __SCHED
#include "arch/cpu_time.hpp"
#include "sched/task_queue.hpp"
#include "sched/deferred_action.hpp"
#include "atomic"
#include "vector"
#include "kdebug.hpp"
class scheduler
{
	prio_level_task_queues __queues;
	task_wait_queue __sleepers;
	std::vector<task_t*> __non_timed_sleepers;
	unsigned int __tick_rate;
	unsigned int __cycle_divisor;
	std::atomic<unsigned> __tick_cycles;
	cpu_timer_stopwatch __timestamp_stopwatch;
	bool __running;
	size_t __total_tasks;
	ooos::deferred_action_queue __deferred_actions;
	static scheduler __instance;
	static bool __has_init;
	bool __set_wait_time(task_t* task, unsigned int time, bool can_interrupt);
	bool __set_untimed_wait(task_t* task);
	void __do_task_change(task_t* cur, task_t* next);
protected:
	task_t* select_next();
	void on_tick();
	__nointerrupts bool init() noexcept;
	scheduler();
public:
	constexpr bool is_running() const noexcept  { return __running; }
	constexpr void start() noexcept             { __running = true; }
	constexpr void stop() noexcept              { __running = false; }
	scheduler(scheduler const&) = delete;
	scheduler& operator=(scheduler const&) = delete;
	void register_task(task_t* task);
	bool unregister_task(task_t* task);
	bool unregister_task_tree(task_t* task);
	bool set_wait_untimed(task_t* task);
	bool interrupt_wait(task_t* waiting);
	bool set_wait_timed(task_t* task, unsigned int time, bool can_interrupt = true);
	task_t* yield();
	task_t* fallthrough_yield();
	static bool init_instance() noexcept;
	static bool has_init() noexcept;
	static scheduler& get() noexcept;
	static void add_worker_task(task_t* worker);
	static void remove_worker_task(task_t* worker);
	static inline time_t ms_to_ticks(time_t ms) noexcept { return __instance.__deferred_actions.compute_ticks(ms); }
	template<ooos::__internal::__extended_runnable FT> static inline void defer_millis(time_t delay_ms, FT&& action) { __instance.__deferred_actions.add(delay_ms, std::forward<FT>(action)); }
	template<ooos::__internal::__extended_runnable FT> static inline void defer_sec(time_t delay_sec, FT&& action) { defer_millis(delay_sec * 1000UL, std::forward<FT>(action)); }
};
#define sch scheduler::get()
#endif