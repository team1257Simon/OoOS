#ifndef __SCHED_TASKLIST
#define __SCHED_TASKLIST
#include "sched/task_ctx.hpp"
#include "elf64.h"
#include "set"
class task_list : std::set<task_ctx>
{
    using __base = std::set<task_ctx>;
    static task_list __inst;
    uint64_t __mk_pid() const noexcept;
    constexpr task_list() : __base{} {}
    friend class task_ctx;
    friend iterator ctx_fork(task_ctx const&);
public:
    using __base::iterator;
    using __base::const_iterator;
    using __base::begin;
    using __base::end;
    using __base::find;
    using __base::contains;
    task_list(task_list const&) = delete;
    task_list& operator=(task_list const&) = delete;
    iterator create_system_task(task_functor task, std::vector<const char*>&& args, size_t stack_size, size_t tls_size, priority_val pv = priority_val::PVSYS, uint16_t quantum = 3);
    iterator create_user_task(task_functor task, std::vector<const char*>&& args, vaddr_t frame_ptr, vaddr_t stack_base, vaddr_t tls_base, int64_t parent_pid = -1L, size_t stack_size = S04, size_t tls_size = S04, priority_val pv = priority_val::PVNORM, uint16_t quantum = 3);
    iterator create_user_task(elf64_desc const& program_desc, std::vector<const char*>&& args, int64_t parent_pid = -1L, priority_val pv = priority_val::PVNORM, uint16_t quantum = 3);
    bool destroy_task(uint64_t pid);
    static task_list& get();
};
#endif