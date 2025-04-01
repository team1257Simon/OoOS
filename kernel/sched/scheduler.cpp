#include "sched/scheduler.hpp"
#include "isr_table.hpp"
extern "C" { extern std::atomic<bool> task_change_flag; extern task_t kproc; }
scheduler scheduler::__instance{};
bool scheduler::__has_init{ false };
bool scheduler::has_init() noexcept { return __has_init; }
bool scheduler::init_instance() { return has_init() || (__has_init = __instance.init()); }
scheduler &scheduler::get() noexcept { return __instance; }
scheduler::scheduler() = default;
bool scheduler::__set_untimed_wait(task_t* task) { try { __non_timed_sleepers.push_back(task); task->task_ctl.block = true; task->task_ctl.can_interrupt = true; return true; } catch(std::exception& e) { panic(e.what()); return false; } }
void scheduler::register_task(task_t* task) { __my_queues[task->task_ctl.prio_base].push(task); }
bool scheduler::interrupt_wait(task_t* waiting) { if(task_wait_queue::const_iterator i = __my_sleepers.find(waiting); i != __my_sleepers.end()) { return __my_sleepers.interrupt_wait(i); } else return false; }
bool scheduler::__set_wait_time(task_t* task, unsigned int time, bool can_interrupt)
{
    task->task_ctl.block = true;
    task->task_ctl.can_interrupt = can_interrupt;
    unsigned int total = __my_sleepers.cumulative_remaining_ticks();
    if(time < total)
    {
        unsigned int cumulative = 0;
        for(task_wait_queue::const_iterator i = __my_sleepers.current(); i != __my_sleepers.end(); i++)
        {
            unsigned int cwait = (*i)->task_ctl.wait_ticks_delta;
            if(cwait + cumulative > time) { task->task_ctl.wait_ticks_delta = time - cumulative; return __my_sleepers.insert(i, task) != __my_sleepers.end(); }
            cumulative += cwait;
        }
    }
    task->task_ctl.wait_ticks_delta = time - total;
    __my_sleepers.push(task);
    return true;
}
__isrcall void scheduler::__do_task_change(task_t* cur, task_t* next)
{
    next->quantum_rem = next->quantum_val;
    cur->next = next;
    task_change_flag.store(true);
    uint64_t ts = sys_time(nullptr);
    next->run_split = ts;
    cur->run_time += (ts - cur->run_split);
}
bool scheduler::unregister_task(task_t* task) 
{
    bool result = false;
    if(task->task_ctl.prio_base == priority_val::PVSYS) { if(task_pl_queue::const_iterator i = __my_queues[priority_val::PVSYS].find(task, true); i != __my_queues[priority_val::PVSYS].end()) { result = __my_queues[priority_val::PVSYS].erase(i) != 0; asm volatile("mfence" ::: "memory"); } }
    for(priority_val pv = task->task_ctl.prio_base; pv <= priority_val::PVEXTRA; pv = priority_val(int8_t(pv) + 1)) { if(task_pl_queue::const_iterator i = __my_queues[pv].find(task, true); i != __my_queues[pv].end()) { result = __my_queues[pv].erase(i) != 0; asm volatile("mfence" ::: "memory"); } }
    __sync_synchronize();
    return result;
}
bool scheduler::unregister_task_tree(task_t* task)
{
    if(!task->num_child_procs || !task->child_procs) return unregister_task(task);
    bool result = true;
    for(size_t i = 0; i < task->num_child_procs; i++) result &= unregister_task(task->child_procs[i]);
    return result && unregister_task(task);
}
__isrcall task_t *scheduler::select_next()
{
    // first check system priority (I/O drivers and such will go here; most of the time they will be sleeping)
    if(!__my_queues[priority_val::PVSYS].empty()) { if(__my_queues[priority_val::PVSYS].at_end()) __my_queues[priority_val::PVSYS].restart(); return __my_queues[priority_val::PVSYS].pop(); }
    task_t* target = nullptr;
    for(priority_val pv = priority_val::PVEXTRA; pv >= priority_val::PVLOW; pv = priority_val(int8_t(pv) - 1))
    {
        task_pl_queue& queue = __my_queues[pv];
        if(!queue.empty())
        {
            for(priority_val qv = priority_val(int8_t(pv) - 1); qv >= priority_val::PVLOW; qv = priority_val(int8_t(qv) - 1)) { queue.on_skipped(); if(queue.skip_flag()) { queue.transfer_next(__my_queues[escalate(qv)]); __my_queues[escalate(qv)].back()->task_ctl.skips = 0; } }
            if(queue.at_end()) queue.restart();
            asm volatile("mfence" ::: "memory"); 
            task_t* result = queue.pop();
            result->task_ctl.skips = 0;
            if(result->task_ctl.prio_base != pv) { __my_queues[pv].unpop(); __my_queues[pv].transfer_next(__my_queues[deescalate(pv)]); asm volatile("mfence" ::: "memory"); }
            target = result;
            break;
        }
    }
    return target;
}
bool scheduler::set_wait_untimed(task_t *task)
{
    if(task->task_ctl.prio_base == priority_val::PVSYS) { if(task_pl_queue::const_iterator i = __my_queues[priority_val::PVSYS].find(task); i != __my_queues[priority_val::PVSYS].end() && __my_queues[priority_val::PVSYS].erase(i) != 0) { return __set_untimed_wait(task); } }
    for(priority_val pv = task->task_ctl.prio_base; pv <= priority_val::PVEXTRA; pv = priority_val(int8_t(pv) + 1)) { if(task_pl_queue::const_iterator i = __my_queues[pv].find(task); i != __my_queues[pv].end() && __my_queues[pv].erase(i) != 0) { return __set_untimed_wait(task); } }
    return false;
}
bool scheduler::set_wait_timed(task_t *task, unsigned int time, bool can_interrupt)
{
    if(task->task_ctl.prio_base == priority_val::PVSYS) { if(task_pl_queue::const_iterator i = __my_queues[priority_val::PVSYS].find(task); i != __my_queues[priority_val::PVSYS].end() && __my_queues[priority_val::PVSYS].erase(i) != 0) { return __set_wait_time(task, time, can_interrupt); } }
    for(priority_val pv = task->task_ctl.prio_base; pv <= priority_val::PVEXTRA; pv = priority_val(int8_t(pv) + 1)) { if(task_pl_queue::const_iterator i = __my_queues[pv].find(task); i != __my_queues[pv].end() && __my_queues[pv].erase(i) != 0) { return __set_wait_time(task, time, can_interrupt); } }
    return false;
}
__isrcall void scheduler::on_tick()
{
    __my_sleepers.tick_wait();
    if(!__my_sleepers.at_end()) { task_t* front_sleeper = __my_sleepers.next(); while(front_sleeper && front_sleeper->task_ctl.wait_ticks_delta == 0) { __my_queues[front_sleeper->task_ctl.prio_base].push(__my_sleepers.pop()); front_sleeper = __my_sleepers.next(); } }
    task_t* cur = get_gs_base<task_t>();
    if(cur->quantum_rem) { cur->quantum_rem--; }
    if(cur->quantum_rem == 0 || cur->task_ctl.block) { if(task_t* next = select_next()) __do_task_change(cur, next); else { cur->quantum_rem = cur->quantum_val; } }
}
bool scheduler::init()
{
    try
    {
        __my_sleepers.reserve(16);
        __non_timed_sleepers.reserve(16);
        for(prio_level_task_queues::iterator i = __my_queues.begin(); i != __my_queues.end(); i++) i->reserve(16);
    }
    catch(std::exception& e) { panic(e.what()); return false; }
    task_change_flag.store(0);
    init_pit();
    interrupt_table::add_irq_handler(0, std::move(LAMBDA_ISR()
    {
        if(__running)
        {
            __my_subticks++;
            if(__my_subticks >= sub_tick_ratio)
            {
                on_tick();
                __my_subticks = 0;
                __my_tick_cycles++;
                if(__my_tick_cycles >= early_trunc_thresh) { __my_subticks++; if(__my_tick_cycles >= cycle_max) __my_tick_cycles = 0; }
            }
        }
    }));
    return true;
}