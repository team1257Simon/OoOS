#ifndef __SCHED
#define __SCHED
#include "sched/task_queue.hpp"
#include "atomic"
#include "vector"
class scheduler
{
    prio_level_task_queues __my_queues{};
    task_wait_queue __my_sleepers{};
    std::vector<task_t*> __non_timed_sleepers{};
    std::atomic<unsigned> __my_subticks{};
    std::atomic<unsigned char> __my_tick_cycles{};
    static scheduler __instance;
    static bool __has_init;
    bool __set_wait_time(task_t* task, unsigned int time, bool can_interrupt);
    bool __set_untimed_wait(task_t* task);
protected:
    __isrcall task_t* select_next();
    __isrcall void on_tick();
    bool init();
    constexpr scheduler() = default;
public:
    scheduler(scheduler const&) = delete;
    scheduler& operator=(scheduler const&) = delete;
    void register_task(task_t* task);
    bool unregister_task(task_t* task);
    bool set_wait_untimed(task_t* task);
    bool set_wait_timed(task_t* task, unsigned int time, bool can_interrupt = true);
    static bool init_instance();
    static bool has_init() noexcept;
    static scheduler& get() noexcept;
};
#endif