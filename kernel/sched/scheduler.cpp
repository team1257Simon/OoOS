#include "sched/scheduler.hpp"
#include "isr_table.hpp"
#include "arch/arch_amd64.h"
extern "C" 
{
    extern std::atomic<bool> task_change_flag;
    extern std::atomic<int8_t> task_signal_code;
    extern task_t kproc;
}
scheduler scheduler::__instance{};
bool scheduler::__has_init{ false };
constexpr static priority_val incr_pv(priority_val v) { return static_cast<priority_val>(static_cast<int8_t>(v) + 1); }
constexpr static priority_val decr_pv(priority_val v) { return static_cast<priority_val>(static_cast<int8_t>(v) - 1); }
bool scheduler::has_init() noexcept { return __has_init; }
bool scheduler::init_instance() { return has_init() || (__has_init = __instance.init()); }
scheduler& scheduler::get() noexcept { return __instance; }
void scheduler::register_task(task_t* task) { __queues[task->task_ctl.prio_base].push(task); __total_tasks++; }
using priority_val::PVLOW;
using priority_val::PVEXTRA;
using priority_val::PVSYS;
scheduler::scheduler()      :
    __queues                {},
    __sleepers              {},
    __non_timed_sleepers    {},
    __tick_rate             {},
    __cycle_divisor         {},
    __tick_cycles           {},
    __running               { false },
    __total_tasks           { 0UZ }
                            {}
bool scheduler::__set_untimed_wait(task_t* task)
{
    try 
    {
        __non_timed_sleepers.push_back(task); 
        task->task_ctl.block            = true;
        task->task_ctl.can_interrupt    = true;
        return true;
    }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}

bool scheduler::interrupt_wait(task_t* waiting)
{
    if(task_wait_queue::const_iterator i = __sleepers.find(waiting); i != __sleepers.end() && (*i)->task_ctl.can_interrupt) { return __sleepers.interrupt_wait(i); } 
    else if(std::vector<task_t*>::iterator i = __non_timed_sleepers.find(waiting); i != __non_timed_sleepers.end()) 
    {
        task_t* task            = *i;
        task->task_ctl.block    = false;
        __non_timed_sleepers.erase(i);
        return true;
    }
    return false;
}
bool scheduler::__set_wait_time(task_t* task, unsigned int time, bool can_interrupt)
{
    task->task_ctl.block            = true;
    task->task_ctl.can_interrupt    = can_interrupt;
    unsigned int total              = __sleepers.cumulative_remaining_ticks();
    if(time < total)
    {
        unsigned int cumulative = 0;
        for(task_wait_queue::const_iterator i = __sleepers.current(); i != __sleepers.end(); i++)
        {
            unsigned int cwait = (*i)->task_ctl.wait_ticks_delta;
            if(cwait + cumulative > time) { task->task_ctl.wait_ticks_delta = time - cumulative; return __sleepers.insert(i, task) != __sleepers.end(); }
            cumulative += cwait;
        }
    }
    task->task_ctl.wait_ticks_delta = time - total;
    __sleepers.push(task);
    return true;
}
void scheduler::__do_task_change(task_t* cur, task_t* next)
{
    next->quantum_rem   = next->quantum_val;
    cur->next           = next;
    task_change_flag.store(true);
    uint64_t ts         = sys_time(nullptr);
    next->run_split     = ts;
    cur->run_time       += (ts - cur->run_split);
}
bool scheduler::unregister_task(task_t* task) 
{
    bool result = false;
    if(task->task_ctl.prio_base == PVSYS) { if(task_pl_queue::const_iterator i = __queues[PVSYS].find(task, true); i != __queues[PVSYS].end()) { result = __queues[PVSYS].erase(i) != 0; asm volatile("mfence" ::: "memory"); } }
    for(priority_val pv = task->task_ctl.prio_base; pv <= PVEXTRA; pv = incr_pv(pv)) { if(task_pl_queue::const_iterator i = __queues[pv].find(task, true); i != __queues[pv].end()) { result = __queues[pv].erase(i) != 0; asm volatile("mfence" ::: "memory"); } }
    __sync_synchronize();
    if(__total_tasks) __total_tasks--;
    return result;
}
bool scheduler::unregister_task_tree(task_t* task)
{
    if(!task->num_child_procs || !task->child_procs) return unregister_task(task);
    bool result     = true;
    task_ctx* xtask = task->self;
    for(task_ctx* c : xtask->child_tasks) { result &= unregister_task_tree(c->task_struct.self); }
    return result && unregister_task(task);
}
task_t* scheduler::select_next()
{
    // first check system priority (I/O drivers and such will go here; most of the time they will be sleeping)
    if(!__queues[PVSYS].empty()) { if(__queues[PVSYS].at_end()) __queues[PVSYS].restart(); return __queues[PVSYS].pop(); }
    task_t* target = nullptr;
    for(priority_val pv = PVEXTRA; pv >= PVLOW; pv = decr_pv(pv))
    {
        task_pl_queue& queue = __queues[pv];
        if(!queue.empty())
        {
            task_pl_queue::iterator i = queue.current(), j;
            task_t* result;
            do {
                if(queue.at_end()) queue.restart();
                fence();
                result  = queue.pop();
                fence();
                j       = queue.current();
            } while(result->task_ctl.block && i != j);
            if(result->task_ctl.block) continue;
            result->task_ctl.skips = 0;
            if(result->task_ctl.prio_base != pv)
            { 
                __queues[pv].unpop();
                __queues[pv].transfer_next(__queues[deescalate(pv)]);
                fence();
            }
            target = result;
            for(priority_val qv = decr_pv(pv); qv >= PVLOW; qv = decr_pv(qv)) 
            { 
                queue.on_skipped(); 
                if(queue.skip_flag()) 
                { 
                    priority_val rv = escalate(qv);
                    queue.transfer_next(__queues[rv]);
                    __queues[rv].back()->task_ctl.skips = 0;
                }
            }
            break;
        }
    }
    return target;
}
bool scheduler::set_wait_untimed(task_t* task)
{
    if(!task->task_ctl.block)
        return __set_untimed_wait(task);
    return false;
}
bool scheduler::set_wait_timed(task_t* task, unsigned int time, bool can_interrupt)
{
    if(!task->task_ctl.block)
        return __set_wait_time(task, time, can_interrupt);
    return false;
}
void scheduler::on_tick()
{
    __sleepers.tick_wait();
    if(!__sleepers.at_end())
    { 
        task_t* front_sleeper = __sleepers.next();
        while(front_sleeper && front_sleeper->task_ctl.wait_ticks_delta == 0)
        {
            task_t* wakee           = __sleepers.pop();
            wakee->task_ctl.block   = false;
            front_sleeper           = __sleepers.next();
        } 
    }
    task_t* cur = get_task_base();
    if(cur->quantum_rem) { cur->quantum_rem--; }
    if(cur->quantum_rem == 0 || cur->task_ctl.block) { if(task_t* next = select_next()) __do_task_change(cur, next); else { cur->quantum_rem = cur->quantum_val; } }
}
bool scheduler::init()
{
    try
    {
        __sleepers.reserve(16UZ);
        __non_timed_sleepers.reserve(16UZ);
        for(prio_level_task_queues::iterator i = __queues.begin(); i != __queues.end(); i++) i->reserve(16);
    }
    catch(std::exception& e) { panic(e.what()); return false; }
    task_change_flag.store(false);
    uint32_t timer_frequency    = cpuid(0x15U, 0).ecx;
    if(!timer_frequency)
        timer_frequency         = cpuid(0x16U, 0).ecx;
    __cycle_divisor             = timer_frequency;
    __tick_rate                 = magnitude(timer_frequency);
    interrupt_table::add_irq_handler(0, std::move([this]() -> void
    {
        if(__running)
        {
            __tick_cycles += __tick_rate;
            if(__tick_cycles >= __cycle_divisor) on_tick();
            __tick_cycles = __tick_cycles % __cycle_divisor;
        }
    }));
    interrupt_table::add_interrupt_callback([this](byte idx, qword) -> void
    {
        if(idx < 0x20UC && get_task_base() != std::addressof(kproc))
            if(task_ctx* task = get_task_base()->self; task->is_user())
                task_signal_code = exception_signals[idx];
    });
    return true;
}
task_t* scheduler::yield()
{
    task_t* cur         = current_active_task();
    cur->quantum_rem    = 0;
    task_t* next        = select_next();
    if(!next) next      = cur; 
    else asm volatile("swapgs; wrgsbase %0; swapgs" :: "r"(next) : "memory"); 
    next->quantum_rem   = next->quantum_val;
    return next;
}
task_t* scheduler::fallthrough_yield()
{
    if(__total_tasks == 0) return nullptr;
    task_t* next            = select_next();
    if(next)
        next->quantum_rem   = next->quantum_val;
    return next;
}