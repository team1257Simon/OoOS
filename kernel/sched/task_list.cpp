#include "sched/task_list.hpp"
#include "heap_allocator.hpp"
#include "stdlib.h" // rand()
extern "C" kframe_tag* __kernel_frame_tag;
uint64_t task_list::__mk_pid() const noexcept { uint64_t pid; do { pid = rand() * rand(); } while(pid == 0 || this->contains(pid)); return pid; }
task_list::iterator task_list::create_system_task(task_functor task, std::vector<const char*>&& args, size_t stack_size, size_t tls_size, priority_val pv, uint16_t quantum) { return this->emplace(task, std::move(args), heap_allocator::get().allocate_kernel_block(stack_size), static_cast<ptrdiff_t>(stack_size), heap_allocator::get().allocate_kernel_block(tls_size), tls_size, __kernel_frame_tag, __mk_pid(), 0, pv, quantum).first; }