#define INST_TL
#include "sched/task_list.hpp"
#include "sched/scheduler.hpp"
#include "kernel_mm.hpp"
#include "frame_manager.hpp"
#include "stdlib.h" // rand()
using namespace std;
extern "C" kframe_tag* __kernel_frame_tag;
task_list task_list::__instance{};
task_list& task_list::get() { return __instance; }
uint64_t task_list::__mk_pid() const noexcept { uint64_t pid; do { pid = static_cast<unsigned long>(rand()); } while(pid == 0 || contains(pid)); return pid; }
task_ctx* task_list::create_system_task(task_functor task, cstr_vec&& args, size_t stack_size, size_t tls_size, priority_val pv, uint16_t quantum)
{
    kframe_tag* ft = __kernel_frame_tag;
    iterator result = emplace
    (
        task,
        std::move(args), 
        ft->allocate(stack_size, page_size),
        static_cast<ptrdiff_t>(stack_size),
        ft->allocate(tls_size, stack_size),
        tls_size,
        ft,
        __mk_pid(),
        0L,
        pv,
        quantum
    ).first;
    result->init_task_state();
    return result.base();
}
task_ctx* task_list::create_user_task(prog_desc_t const& program_desc, cstr_vec&& args, int64_t parent_pid, priority_val pv, uint16_t quantum, pid_t pid) 
{
    iterator result = emplace(program_desc, std::move(args), pid ? pid : __mk_pid(), parent_pid, pv, quantum).first; 
    result->init_task_state(); 
    return result.base(); 
}
bool task_list::destroy_task(uint64_t pid)
{
    iterator i = find(pid);
    if(i == end()) return false;
    erase(i);
    return true;
}
task_ctx* task_list::task_vfork(task_ctx const* ctx)
{
    // TODO check/enforce thread limit
    if(!ctx) return nullptr;
    task_ctx* result;
    try { result = emplace(*ctx).first.base(); }
    catch(...) { return nullptr; }
    return result;
}