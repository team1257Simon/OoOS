#include <sched/task_list.hpp>
#include <sched/scheduler.hpp>
#include <frame_manager.hpp>
#include <stdlib.h> // rand()
using namespace std;
extern "C" kframe_tag* __kernel_frame_tag;
task_list task_list::instance{};
pid_t task_list::__upid() const noexcept
{
	pid_t pid{};
	do pid = static_cast<pid_t>(rand()); while(!pid || contains(pid));
	return pid;
}
task_ctx* task_list::create_user_task(prog_desc_t const& program_desc, cstr_vec&& args, spid_t parent_pid, priority_val pv, uint16_t qv, pid_t pid)
{
    iterator result	= emplace(program_desc, move(args), pid ? pid : __upid(), parent_pid, pv, static_cast<uint16_t>(scheduler::ms_to_ticks(qv))).first;
    result->init_task_state();
    return result.base();
}
bool task_list::destroy_task(pid_t pid)
{
    iterator i		= find(pid);
    if(__unlikely(i	== end())) return false;
    erase(i);
    return true;
}
task_ctx* task_list::task_vfork(task_ctx const* ctx)
{
    // TODO check/enforce process limit
    if(!ctx) return nullptr;
    task_ctx* result;
    try { result = emplace(*ctx).first.base(); }
    catch(...) { return nullptr; }
    return result;
}