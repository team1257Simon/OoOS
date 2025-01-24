#ifndef __SCHED_TASKLIST
#define __SCHED_TASKLIST
#include "sched/task_ctx.hpp"
#include "set"
class task_list : public std::set<task_ctx>
{
    using __base = std::set<task_ctx>;
    uint64_t __mk_pid() const noexcept;
public:
    using __base::key_type;
    using __base::value_type;
    using __base::size_type;
    using __base::difference_type;
    using __base::allocator_type;
    using __base::reference;
    using __base::const_reference;
    using __base::pointer;
    using __base::const_pointer;
    using __base::iterator;
    using __base::const_iterator;
    constexpr task_list() : __base{} {}
    task_list(task_list const&) = delete;
    task_list& operator=(task_list const&) = delete;
    iterator create_system_task(task_functor task, std::vector<const char*>&& args, size_t stack_size, size_t tls_size, priority_val pv = priority_val::PVSYS, uint16_t quantum = 3);
};
#endif