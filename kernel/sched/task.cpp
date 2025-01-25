#include "sched/scheduler.hpp"
#include "sched/task_ctx.hpp"
#include "errno.h"
static fx_state __init_fx_state{};
static bool __fx_initialized{ false };
static inline vaddr_t get_applicable_cr3(vaddr_t frame_ptr) { return (*static_cast<uint64_t*>(frame_ptr) == UFRAME_MAGIC) ? *static_cast<paging_table*>(frame_ptr + ptrdiff_t(8L)) : get_cr3(); }
void task_ctx::__init_task_state(task_functor task, vaddr_t stack_base, ptrdiff_t stack_size, vaddr_t tls_base, vaddr_t frame_ptr)
{
    task_struct.saved_regs.rip = vaddr_t{ task };
    task_struct.saved_regs.rbp = stack_base;
    task_struct.saved_regs.rsp = stack_base + stack_size;
    task_struct.saved_regs.rdi = arg_vec.size();
    task_struct.saved_regs.rsi = std::bit_cast<register_t>(arg_vec.data());
    task_struct.saved_regs.cr3 = get_applicable_cr3(frame_ptr);
    task_struct.saved_regs.rflags = 0x202UL; // bit 1 is reserved (always 1); bit 9 is interrupt-enable
    if(!__fx_initialized)
    {
        asm volatile("fxsave (%0)" :: "r"(&__init_fx_state) : "memory");
        __builtin_memset(__init_fx_state.xmm, 0, sizeof(__init_fx_state.xmm));
        __fx_initialized = true;
    }
    __builtin_memcpy(&task_struct.fxsv, &__init_fx_state, sizeof(fx_state));
    if(is_system())
    {
        task_struct.saved_regs.cs = 0x08;
        task_struct.saved_regs.ds = 0x10;
    }
    else
    {
        task_struct.saved_regs.cs = 0x20;
        task_struct.saved_regs.ds = 0x18;
    }
}
task_ctx::task_ctx(task_functor task, std::vector<const char*>&& args, vaddr_t stack_base, ptrdiff_t stack_size, vaddr_t tls_base, size_t tls_len, vaddr_t frame_ptr, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum) : task_struct { &task_struct, frame_ptr, regstate_t{}, quantum, 0U, tcb_t { { false, false, false, false, prio }, 0U, 0U, 0U, parent_pid, pid}, fx_state{},  0UL, 0UL, 0UL, nullptr, 0UL, tls_base, nullptr }, arg_vec{ std::move(args) }, allocated_stack{ stack_base }, stack_allocated_size{ static_cast<size_t>(stack_size) }, tls{ tls_base }, tls_size{ tls_len } { __init_task_state(task, stack_base, stack_size, tls_base, frame_ptr); }
void task_ctx::add_child(task_ctx *that) { that->task_struct.task_ctl.parent_pid = this->task_struct.task_ctl.task_id; child_tasks.push_back(that); task_struct.num_child_procs = child_tasks.size(); task_struct.child_procs = reinterpret_cast<vaddr_t*>(child_tasks.data()); }
bool task_ctx::remove_child(task_ctx *that) { if(std::vector<task_ctx*>::const_iterator i = child_tasks.find(that); i != child_tasks.end()) { child_tasks.erase(i); return true; } return false; }
void task_ctx::start_task(vaddr_t exit_fn)
{
    *static_cast<uintptr_t*>(task_struct.saved_regs.rsp) = exit_fn; // put the return address onto the stack
    scheduler::get().register_task(this->task_struct.self);
}
tms task_ctx::get_times() const noexcept
{
    tms result{ task_struct.run_time, task_struct.sys_time, 0UL, 0UL };
    for(task_ctx* child : child_tasks)
    {
        tms add_ct = child->get_times();
        result.tms_cstime += (add_ct.tms_cstime + add_ct.tms_stime);
        result.tms_cutime += (add_ct.tms_cutime + add_ct.tms_utime);
    }
    return result;
}
extern "C"
{
    clock_t syscall_times(tms *out) { if(task_ctx* task = static_cast<task_ctx*>(current_active_task()->self); task->is_user() && out) { new (out) tms{ static_cast<task_ctx*>(current_active_task()->self)->get_times() }; return syscall_time(nullptr); } else return -EINVAL; }
    long syscall_getpid() { if(task_ctx* task = static_cast<task_ctx*>(current_active_task()->self); task->is_user()) return static_cast<long>(task->get_pid()); else return 0L; }
}