#include "sched/task_list.hpp"
#include "sched/scheduler.hpp"
#include "heap_allocator.hpp"
#include "stdlib.h" // rand()
extern "C" kframe_tag* __kernel_frame_tag;
task_list task_list::__inst{};
uint64_t task_list::__mk_pid() const noexcept { uint64_t pid; do { pid = static_cast<unsigned long>(rand()); } while(pid == 0 || this->contains(pid)); return pid; }
task_ctx* task_list::create_system_task(task_functor task, std::vector<const char*>&& args, size_t stack_size, size_t tls_size, priority_val pv, uint16_t quantum) { iterator result = this->emplace(task, std::move(args), heap_allocator::get().allocate_kernel_block(stack_size), static_cast<ptrdiff_t>(stack_size), heap_allocator::get().allocate_kernel_block(tls_size), tls_size, __kernel_frame_tag, this->__mk_pid(), 0L, pv, quantum).first; result->init_task_state(); return result.base(); }
task_ctx* task_list::create_user_task(task_functor task, std::vector<const char *> &&args, addr_t frame_ptr, addr_t stack_base, addr_t tls_base, int64_t parent_pid, size_t stack_size, size_t tls_size, priority_val pv, uint16_t quantum) { iterator result = this->emplace(task, std::move(args), stack_base, static_cast<ptrdiff_t>(stack_size), tls_base, tls_size, frame_ptr, this->__mk_pid(), parent_pid, pv, quantum).first; result->init_task_state(); return result.base(); }
task_ctx* task_list::create_user_task(elf64_program_descriptor const &program_desc, std::vector<const char *> &&args, int64_t parent_pid, priority_val pv, uint16_t quantum) { return create_user_task(addr_t(program_desc.entry), std::move(args), program_desc.frame_ptr, program_desc.prg_stack, program_desc.prg_tls, parent_pid, program_desc.stack_size, program_desc.tls_size, pv, quantum); }
bool task_list::destroy_task(uint64_t pid) { iterator i = this->find(pid); if(i == this->end()) return false; scheduler::get().unregister_task_tree(i->task_struct.self); this->erase(i); return true; }
task_list &task_list::get() { return __inst; }