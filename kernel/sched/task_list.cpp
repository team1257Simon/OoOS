#include "sched/task_list.hpp"
#include "sched/scheduler.hpp"
#include "kernel_mm.hpp"
#include "frame_manager.hpp"
#include "stdlib.h" // rand()
using namespace std;
extern "C" kframe_tag* __kernel_frame_tag;
task_list task_list::__instance{};
uint64_t task_list::__upid() const noexcept { uint64_t pid; do { pid = static_cast<unsigned long>(rand()); } while(pid == 0 || contains(pid)); return pid; }
task_list& task_list::get() { return __instance; }
task_ctx* task_list::create_system_task(task_functor task, cstr_vec&& args, size_t st_sz, size_t tls_sz, priority_val pv, uint16_t qv)
{
    kframe_tag* ft  = __kernel_frame_tag;
    addr_t stk      = ft->allocate(st_sz, page_size);
    addr_t tls      = ft->allocate(tls_sz, st_sz);
    if(__unlikely(!stk || !tls)) throw std::bad_alloc();
    iterator result = emplace(task, move(args), stk, static_cast<ptrdiff_t>(st_sz), tls, tls_sz, ft, __upid(), 0L, pv, qv).first;
    result->init_task_state();
    return result.base();
}
task_ctx* task_list::create_user_task(prog_desc_t const& program_desc, cstr_vec&& args, int64_t parent_pid, priority_val pv, uint16_t qv, pid_t pid) 
{
    iterator result = emplace(program_desc, move(args), pid ? pid : __upid(), parent_pid, pv, qv).first; 
    result->init_task_state(); 
    return result.base(); 
}
bool task_list::destroy_task(uint64_t pid)
{
    iterator i = find(pid);
    if(__unlikely(i == end())) return false;
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