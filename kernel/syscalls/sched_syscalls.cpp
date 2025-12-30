#include <sched/scheduler.hpp>
#include <sched/task_list.hpp>
#include <frame_manager.hpp>
#include <prog_manager.hpp>
#include <kernel_mm.hpp>
#include <errno.h>
#include <elf64_exec.hpp>
#include <arch/arch_amd64.h>
#include <rtc.h>
#include <users.hpp>
extern "C"
{
	extern task_t* kproc;
	extern std::atomic<bool> task_lock_flag;
	static char* empty_env	= nullptr;
	struct exec_fail_guard
	{
		task_ctx* task;
		constexpr exec_fail_guard(task_ctx* c) noexcept : task(c) {}
		constexpr void release() noexcept { task = nullptr; }
		constexpr ~exec_fail_guard() { if(task) tl.destroy_task(task->get_pid()); }
	};
	int prg_execve(file_vnode* restrict n, task_ctx* restrict task, char** restrict argv, char** restrict env, bool noparent)
	{
		if(user_accounts_manager::is_initialized())
		{
			user_handle user	= user_accounts_manager::get_instance()->get_user(task->euid());
			if(__unlikely(!n->check_permissions(*user, CHK_EXECUTE)))
				return -EPERM;
		}
		elf64_executable* ex = prog_manager::get_instance().add(n);
		if(__unlikely(!ex)) return -ENOEXEC;
		cstrvec argv_v{}, env_v{};
		for(size_t i	= 0UZ; argv[i]; ++i) argv_v.push_back(argv[i]);
		for(size_t i	= 0UZ; env[i]; ++i) env_v.push_back(env[i]);
		try
		{
			if(!noparent)
				if(task_list::iterator parent = tl.find(task->get_parent_pid()); parent != tl.end())
					sch.interrupt_wait(std::addressof(parent->task_struct));
			if(task->subsume(ex->describe(), std::move(argv_v), std::move(env_v)))
				return 0;
			else return -ENOMEM;
		}
		catch(std::invalid_argument& e)	{ return panic(e.what()), -ECANCELED; }
		catch(std::out_of_range& e)	 	{ return panic(e.what()), -EFAULT; }
		catch(std::bad_alloc&)			{ panic("[PRG/EXEC] no memory for argument vectors"); }
		return -ENOMEM;
	}
	spid_t prg_spawn(file_vnode* restrict n, task_ctx* restrict task, char** restrict argv, char** restrict renv)
	{
		if(user_accounts_manager::is_initialized())
		{
			user_handle user	= user_accounts_manager::get_instance()->get_user(task->euid());
			if(__unlikely(!n->check_permissions(*user, CHK_EXECUTE)))
				return -EPERM;
		}
		if(task_ctx* clone	= tl.task_vfork(task))
		{
			exec_fail_guard guard(clone);
			try { clone->start_task(task->exit_target); }
			catch(...) { return -ENOMEM; }
			task->add_child(clone);
			int ecode		= prg_execve(n, clone, argv, renv, true);
			if(__unlikely(ecode)) return ecode;
			guard.release();
			return clone->get_pid();
		}
		return -EAGAIN;
	}
	clock_t syscall_times(tms* out)
	{
		task_ctx* task	= active_task_context();
		if(__unlikely(!task)) return -ENOSYS;
		out				= translate_user_pointer(out);
		if(!out) return -EFAULT;
		new(out) tms(task->get_times());
		return sys_time(nullptr);
	}
	int syscall_gettimeofday(timeval* restrict tm, void* restrict tz)
	{
		tm = translate_user_pointer(tm);
		if(!tm) return -EFAULT;
		std::construct_at<timeval>(tm, timestamp_to_timeval(rtc::get_instance().get_timestamp()));
		return 0;
	}
	spid_t syscall_getpid()
	{
		if(task_ctx* task = active_task_context())
			return static_cast<long>(task->get_pid());
		else return 0; /* Not an error technically; system tasks are PID 0 */
	}
	void syscall_exit(int n) {
		if(task_ctx* task = active_task_context())
			task->set_exit(n);
	}
	[[noreturn]] void syscall_threadexit(register_t retval)
	{
		task_t* task		= current_active_task();
		if(__unlikely(!task->thread_ptr)) handle_exit();
		else handle_thread_exit(task->thread_ptr, retval);
		__builtin_unreachable();
	}
	[[noreturn]] void handle_thread_exit(thread_t* thread_ptr, register_t retval)
	{
		pid_t thread_id		= thread_ptr->ctl_info.thread_id;
		task_ctx* task		= active_task_context();
		cli();
		try { task->thread_exit(thread_id, retval); }
		catch(std::out_of_range& e)	{ panic(e.what()); force_signal(task, 11UC); }
		catch(std::exception& e)	{ panic(e.what()); force_signal(task, 12UC); }
		task_lock_flag		= false;
		kthread_ptr next	= sch.fallthrough_yield();
		if(__unlikely(!next))
		{
			task->terminate();
			tl.destroy_task(task->get_pid());
			kernel_reentry();
		}
		else
		{
			if(next.thread_ptr)
				next.activate();
			fallthrough_reentry(next.task_ptr);
		}
		__builtin_unreachable();
	}
	[[noreturn]] void handle_exit()
	{
		cli();
		task_ctx* task		= active_task_context();
		task->terminate();
		tl.destroy_task(task->get_pid());
		task_lock_flag		= false;
		kthread_ptr next	= sch.fallthrough_yield();
		if(next)
		{
			if(next.thread_ptr)
				next.activate();
			fallthrough_reentry(next.task_ptr);
		}
		else kernel_reentry();
		__builtin_unreachable();
	}
	int syscall_sleep(unsigned long seconds)
	{
		task_ctx* task		= active_task_context();
		kthread_ptr thr		= kthread_of(task);
		kthread_ptr next	= sch.yield();
		if(__unlikely(next == thr)) return -ECHILD;
		if(__unlikely(!sch.set_wait_timed(thr, seconds * 1000, false))) return -ENOMEM;
		task->task_struct.saved_regs.rax = 0;
		return next->saved_regs.rax;
	}
	pid_t syscall_wait(int* sc_out)
	{
		task_ctx* task	= active_task_context();
		kthread_ptr thr	= kthread_of(task);
		sc_out			= translate_user_pointer(sc_out);
		if(task->last_notified)
		{
			if(sc_out)
				*sc_out = task->last_notified->exit_code;
			return task->last_notified->get_pid();
		}
		else
		{
			kthread_ptr next							= sch.yield();
			if(__unlikely(next == thr)) return -ECHILD;
			if(sch.set_wait_untimed(thr))
			{
				task->notif_target							= sc_out;
				task->task_struct.task_ctl.should_notify	= true;
				return next->saved_regs.rax;
			}
		}
		return -ENOMEM;
	}
	spid_t syscall_fork() try
	{
		task_ctx* task	= active_task_context();
		task_ctx* clone	= tl.task_vfork(task);
		if(clone && clone->set_fork())
		{
			clone->start_task(task->exit_target);
			task->add_child(clone);
			clone->task_struct.saved_regs.rax	= 0UL;
			return clone->get_pid();
		}
		else return -EAGAIN;
	}
	catch(...) { return -ENOMEM; }
	spid_t syscall_vfork() try
	{
		task_ctx* task	= active_task_context();
		task_ctx* clone	= tl.task_vfork(task);
		kthread_ptr thr(task->header(), task->current_thread_ptr());
		if(clone && sch.set_wait_untimed(thr))
		{
			clone->start_task(task->exit_target);
			task->add_child(clone);
			task->task_struct.task_ctl.should_notify	= true;
			clone->task_struct.saved_regs.rax			= 0UL;
			task->task_struct.saved_regs.rax			= clone->get_pid();
			kthread_ptr next							= sch.yield();
			return next->saved_regs.rax;
		}
		else return -EAGAIN;
	}
	catch(...) { return -ENOMEM; }
	int syscall_execve(char* restrict name, char** restrict argv, char** restrict env)
	{
		task_ctx* task		= active_task_context();
		filesystem* fs_ptr	= get_task_vfs();
		if(__unlikely(!fs_ptr || !task)) return -ENOSYS;
		name				= translate_user_pointer(name);
		argv				= translate_user_pointer(argv);
		char** renv			= env ? translate_user_pointer(env).as<char*>() : std::addressof(empty_env);
		if(__unlikely(!name || !argv || !renv)) return -EFAULT;
		// TODO: check execute permissions
		try
		{
			file_vnode* n	= fs_ptr->open_file(name, std::ios_base::in);
			int ecode		= prg_execve(n, task, argv, renv, false);
			fs_ptr->close_file(n);
			if(__unlikely(ecode)) return ecode;
			return task->task_struct.saved_regs.rax;
		}
		catch(std::exception& e) { return panic(e.what()), -ENOENT; }
	}
	int syscall_fexecve(int fd, char** restrict argv, char** restrict env)
	{
		task_ctx* task		= active_task_context();
		filesystem* fs_ptr	= get_task_vfs();
		if(__unlikely(!fs_ptr || !task)) return -ENOSYS;
		argv				= translate_user_pointer(argv);
		char** renv			= env ? translate_user_pointer(env).as<char*>() : std::addressof(empty_env);
		if(__unlikely(!argv || !renv)) return -EFAULT;
		// TODO: check execute permissions
		try
		{
			file_vnode* n	= get_by_fd(fs_ptr, task, fd);
			if(n)
			{
				if(!n->current_mode.in) return -EACCES;
				int ecode	= prg_execve(n, task, argv, env, false);
				if(__unlikely(ecode)) return ecode;
				return task->task_struct.saved_regs.rax;
			}
			else return -EBADF;
		}
		catch(std::logic_error& e)
		{
			panic(e.what());
			force_signal(task, 13);	// SIGPIPE
			return -EPIPE;
		}
		catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	spid_t syscall_spawn(char* restrict name, char** restrict argv, char** restrict env)
	{
		task_ctx* task			= active_task_context();
		filesystem* fs_ptr		= get_task_vfs();
		if(__unlikely(!fs_ptr || !task)) return -ENOSYS;
		name					= translate_user_pointer(name);
		argv					= translate_user_pointer(argv);
		char** renv				= env ? translate_user_pointer(env).as<char*>() : std::addressof(empty_env);
		if(__unlikely(!name || !argv || !renv)) return -EFAULT;
		// TODO: check execute permissions
		try
		{
			file_vnode* n		= fs_ptr->open_file(name, std::ios_base::in);
			spid_t result		= prg_spawn(n, task, argv, renv);
			fs_ptr->close_file(n);
			return result;
		}
		catch(std::exception& e) { panic(e.what()); }
		return -ENOENT;
	}
	spid_t syscall_fspawn(int fd, char** restrict argv, char** restrict env)
	{
		task_ctx* task		= active_task_context();
		filesystem* fs_ptr	= get_task_vfs();
		if(__unlikely(!fs_ptr || !task)) return -ENOSYS;
		argv				= translate_user_pointer(argv);
		char** renv			= env ? translate_user_pointer(env).as<char*>() : std::addressof(empty_env);
		if(__unlikely(!argv || !renv)) return -EFAULT;
		// TODO: check execute permissions
		try
		{
			file_vnode* n	= get_by_fd(fs_ptr, task, fd);
			if(n)
			{
				if(!n->current_mode.in)
					return -EACCES;
				else return prg_spawn(n, task, argv, renv);
			}
			else return -EBADF;
		}
		catch(std::logic_error& e)
		{
			panic(e.what());
			force_signal(task, 13);	// SIGPIPE
			return -EPIPE;
		}
		catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	spid_t syscall_tfork()
	{
		task_ctx* task		= active_task_context();
		if(__unlikely(!task)) return -ENOSYS;
		try { return static_cast<spid_t>(task->thread_fork()); }
		catch(std::out_of_range& e) 	{ return panic(e.what()), -EFAULT; }
		catch(std::overflow_error& e)	{ return panic(e.what()), -EAGAIN; }
		catch(std::bad_alloc&)			{ panic("[EXEC/THREAD] no memory for thread data"); }
		return -ENOMEM;
	}
	spid_t syscall_threadcreate(addr_t entry_pt, addr_t exit_pt, size_t stack_sz, bool start_detached, register_t arg)
	{
		task_ctx* task		= active_task_context();
		if(__unlikely(!task)) return -ENOSYS;
		try { return static_cast<spid_t>(task->thread_add(entry_pt, exit_pt, stack_sz, start_detached, arg)); }
		catch(std::out_of_range& e) 	{ return panic(e.what()), -EFAULT; }
		catch(std::overflow_error& e)	{ return panic(e.what()), -EAGAIN; }
		catch(std::bad_alloc&)			{ panic("[EXEC/THREAD] no memory for thread data"); }
		return -ENOMEM;
	}
	ssize_t syscall_getthreadcount()
	{
		task_ctx* task		= active_task_context();
		if(__unlikely(!task)) return -ENOSYS;
		// The stored value does not count the initial thread, so bump it by 1.
		// Note that callbacks do not count as threads for the purpose of this syscall.
		// Once quotas are implemented, those will be tracked separately and have their own quota.
		return static_cast<ssize_t>(task->active_added_thread_count + 1Z);
	}
	register_t syscall_threadjoin(pid_t thread) try
	{
		task_ctx* task		= active_task_context();
		if(__unlikely(!task)) return -ENOSYS;
		join_result result	= task->thread_join(thread);
		if(__unlikely(result == join_result::NXTHREAD)) return -ESRCH;
		else if(__unlikely(result == join_result::IMMEDIATE)) return task->task_struct.saved_regs.rax;
		kthread_ptr kth		= kthread_of(task);
		if(__unlikely(!sch.set_wait_untimed(kth))) return -ENOMEM;
		kthread_ptr next	= sch.yield();
		return next->saved_regs.rax;
	}
	catch(std::runtime_error& e) { return panic(e.what()), -EBUSY; }
	catch(std::out_of_range& e) { return panic(e.what()), -EFAULT; }
	int syscall_threaddetach(pid_t thread)
	{
		task_ctx* task		= active_task_context();
		if(__unlikely(!task)) return -ENOSYS;
		return task->thread_detach(thread);
	}
}