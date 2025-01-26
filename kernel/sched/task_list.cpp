#include "sched/task_list.hpp"
#include "sched/scheduler.hpp"
#include "heap_allocator.hpp"
#include "stdlib.h" // rand()
extern "C" kframe_tag* __kernel_frame_tag;
task_list task_list::__inst{};
uint64_t task_list::__mk_pid() const noexcept { uint64_t pid; do { pid = static_cast<unsigned long>(rand()); } while(pid == 0 || this->contains(pid)); return pid; }
task_list::iterator task_list::create_system_task(task_functor task, std::vector<const char*>&& args, size_t stack_size, size_t tls_size, priority_val pv, uint16_t quantum) { return this->emplace(task, std::move(args), heap_allocator::get().allocate_kernel_block(stack_size), static_cast<ptrdiff_t>(stack_size), heap_allocator::get().allocate_kernel_block(tls_size), tls_size, __kernel_frame_tag, __mk_pid(), 0, pv, quantum).first; }
bool task_list::destroy_task(uint64_t pid)
{
    iterator i = this->find(pid);
    if(i == this->end()) return false;
    scheduler::get().unregister_task_tree(i->task_struct.self);
    this->erase(i);
    return true;
}
task_list &task_list::get() { return __inst; }