#ifndef __SCHED
#define __SCHED
#include "kernel/sched/task_queue.hpp"
#include "atomic"
#include "vector"
class scheduler
{
	prio_level_task_queues __queues{};
	task_wait_queue __sleepers{};
	std::vector<task_t*> __non_timed_sleepers{};
	unsigned int __tick_rate{};
	unsigned int __cycle_divisor{};
	std::atomic<unsigned> __tick_cycles{};
	bool __running{};
	size_t __total_tasks{};
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
};
#define sch scheduler::get()
#endif