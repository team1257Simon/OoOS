#ifndef __TASK_CTX
#define __TASK_CTX
#include "sched/task.h"
#include "compare"
#include "vector"
#include "heap_allocator.hpp"
struct task_ctx
{
    task_t task_struct;                     //  The c-style struct from task.h; gs base will point here when the task is active
    std::vector<task_ctx*> child_tasks{};   //  The array in task_struct will be redirected here.
    std::vector<const char*> arg_vec;
    vaddr_t allocated_stack;
    size_t stack_allocated_size;
    vaddr_t tls;
    size_t tls_size;
    task_ctx(task_functor task, std::vector<const char*>&& args, vaddr_t stack_base, ptrdiff_t stack_size, vaddr_t tls_base, size_t tls_len, vaddr_t frame_ptr, uint64_t pid, int64_t parent_pid, priority_val prio = priority_val::PVNORM, uint16_t quantum = 3);
    constexpr uint64_t get_pid() const noexcept { return task_struct.task_ctl.task_id; }
    constexpr int64_t get_parent_pid() const noexcept { return task_struct.task_ctl.parent_pid; }
    constexpr bool is_system() const noexcept { return *static_cast<uint64_t*>(task_struct.frame_ptr) == KFRAME_MAGIC; }
    friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.get_pid() <=> __that.get_pid(); }
    friend constexpr std::strong_ordering operator<=>(task_ctx const& __this, uint64_t __that) noexcept { return __this.get_pid() <=> __that; }
    friend constexpr std::strong_ordering operator<=>(uint64_t __this, task_ctx const& __that) noexcept { return __this <=> __that.get_pid(); }
    friend constexpr bool operator==(task_ctx const& __this, task_ctx const& __that) noexcept { return __this.task_struct.self == __that.task_struct.self; }
    void add_child(task_ctx* that);
    bool remove_child(task_ctx* that);
    void start_task(vaddr_t exit_fn);
private:
    void __init_task_state(task_functor task, vaddr_t stack_base, ptrdiff_t stack_size, vaddr_t tls_base, vaddr_t frame_ptr);
} __align(16);
#endif