#include "sched/scheduler.hpp"
#include "sched/task_ctx.hpp"
static inline vaddr_t get_applicable_cr3(vaddr_t frame_ptr) { return (*static_cast<uint64_t*>(frame_ptr) == UFRAME_MAGIC) ? *static_cast<paging_table*>(frame_ptr + ptrdiff_t(8L)) : get_cr3(); }
extern "C" int task_wrapper_exec(int argc, char **argv) { if(task_func_pointer ptr = current_active_task()->exec_fn; ptr && ptr->operator bool()) return ptr->operator()(argc, argv); else return -1; }
void task_ctx::__init_task_state(int argc, char **argv, vaddr_t stack_base, ptrdiff_t stack_size, vaddr_t tls_base, vaddr_t frame_ptr)
{
    vaddr_t fnptr{ the_task.target<task_closure>() };
    task_struct.saved_regs.rip = fnptr ? fnptr : vaddr_t{ &task_wrapper_exec };
    task_struct.saved_regs.rbp = stack_base;
    task_struct.saved_regs.rsp = stack_base + stack_size;
    task_struct.saved_regs.rdi = argc;
    task_struct.saved_regs.rsi = std::bit_cast<register_t>(argv);
    task_struct.saved_regs.cr3 = get_applicable_cr3(frame_ptr);
    task_struct.saved_regs.rflags = 0x202UL; // bit 1 is reserved (always 1); bit 9 is interrupt-enable
    asm volatile("fxsave (%0)" :: "r"(&(task_struct.fxsv)) : "memory");
    __builtin_memset(&(task_struct.fxsv.xmm), 0, sizeof(task_struct.fxsv.xmm));
}
task_ctx::task_ctx(std::function<task_closure> &&task, int argc, char **argv, vaddr_t stack_base, ptrdiff_t stack_size, vaddr_t tls_base, vaddr_t frame_ptr, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum) : task_struct { &task_struct, frame_ptr, regstate_t{}, quantum, 0U, tcb_t { { false, false, false, false }, 0U, prio, 0U, 0U, parent_pid, pid, 0UL, 0UL }, fx_state{}, 0UL, nullptr, tls_base, nullptr, &the_task }, the_task{ std::move(task) } { __init_task_state(argc, argv, stack_base, stack_size, tls_base, frame_ptr); }