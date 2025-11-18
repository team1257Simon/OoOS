#ifndef __SCHED_TASKLIST
#define __SCHED_TASKLIST
#include "kernel/sched/task_ctx.hpp"
#include "kernel/elf64.h"
#include "set"
typedef elf64_program_descriptor prog_desc_t;
typedef std::vector<const char*> cstr_vec;
class task_list : std::set<task_ctx>
{
	using __base = std::set<task_ctx>;
	constexpr task_list() : __base{} {}
	pid_t __upid() const noexcept;
	friend class task_ctx;
public:
	using __base::iterator;
	using __base::const_iterator;
	using __base::begin;
	using __base::end;
	using __base::find;
	using __base::contains;
	task_list(task_list const&) = delete;
	task_list& operator=(task_list const&) = delete;
	task_ctx* create_user_task(prog_desc_t const& program_desc, cstr_vec&& args, spid_t parent_pid = -1, priority_val pv = priority_val::PVNORM, uint16_t quantum = 3US, pid_t pid = 0);
	task_ctx* task_vfork(task_ctx const* ctx);
	bool destroy_task(pid_t pid);
	static task_list instance;
};
#define tl task_list::instance
#endif