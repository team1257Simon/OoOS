#include <sched/task_list.hpp>
#include <errno.h>
static bool check_kill(task_ctx* caller, task_list::iterator target)
{
	if(static_cast<uint64_t>(target->get_parent_pid()) == caller->get_pid()) return true;
	for(task_ctx* c : caller->child_tasks) { if(check_kill(c, target)) return true; } 
	return false;
}
static void kill_one(task_ctx* target, int sig, bool force = false) 
{
	if(__unlikely(!target || (!force && target->task_sig_info.blocked_signals.btc(sig)))) return;
	if(!target->task_sig_info.signal_handlers[sig]) target->task_struct.task_ctl.killed = true;
	target->task_sig_info.pending_signals.bts(sig); 
	target->set_signal(sig, true);
}
static int kill_all(task_ctx* caller, int sig)
{
	for(task_list::iterator target = tl.begin(); target != tl.end(); target++)
		if(check_kill(caller, target)) 
			kill_one(target.base(), sig);
	return 0;
}
extern "C"
{
	void force_signal(task_ctx* task, int8_t sig) { kill_one(task, sig, true); }
	void on_invalid_syscall() { panic("[EXEC] D: invalid syscall"); force_signal(active_task_context(), 12); }
	long syscall_sigret()
	{
		task_ctx* task	= active_task_context();
		if(__unlikely(!task)) return -ENOSYS;
		if(!task->task_sig_info.pending_signals.btr(task->task_sig_info.active_signal)) return -EINVAL;
		task->end_signal();
		if(task->task_sig_info.pending_signals) task->set_signal(__builtin_ffsl(task->task_sig_info.pending_signals), false); // state is already saved
		else
		{
			kthread_ptr t(task->header(), translate_user_pointer(task->sigret_thread));
			t.activate();
			task->sigret_thread = nullptr;
		}
		return task->task_struct.saved_regs.rax;
	}
	signal_handler syscall_signal(int sig, signal_handler new_handler)
	{
		task_ctx* task								= active_task_context();
		signal_handler old_handler					= task->task_sig_info.signal_handlers[sig];
		task->task_sig_info.signal_handlers[sig]	= new_handler;
		return old_handler;
	}
	int syscall_kill(long pid, unsigned long sig)
	{
		if(__unlikely(sig > num_signals)) return -EINVAL;
		task_ctx* task				= active_task_context();
		if(pid < 0) return kill_all(task, static_cast<int>(sig));
		task_list::iterator target	= tl.find(pid);
		if(target == tl.end()) return -ESRCH;
		if(__unlikely(!check_kill(task, target))) return -EPERM;
		if(sig) kill_one(target.base(), static_cast<int>(sig));
		return 0;
	}
	int syscall_raise(int sig)
	{
		if(__unlikely(sig > num_signals)) return -EINVAL;
		kill_one(active_task_context(), sig);
		return 0;
	}
	int syscall_sigprocmask(sigprocmask_action how, sigset_t const* restrict set, sigset_t* restrict oset)
	{
		task_ctx* task			= active_task_context();
		sigset_t const* rset	= translate_user_pointer(set);
		if(__unlikely(!rset)) return -EFAULT;
		if(oset)
		{
			oset	= translate_user_pointer(oset);
			if(!oset) return -EFAULT;
			*oset	= task->task_sig_info.blocked_signals;
		}
		if(how == SIG_BLOCK)		{ task->task_sig_info.blocked_signals |= *rset; }
		else if(how == SIG_UNBLOCK)	{ task->task_sig_info.blocked_signals &= ~(*rset); }
		else if(how == SIG_SETMASK)	{ task->task_sig_info.blocked_signals = *rset; }
		else return -EINVAL;
		return 0;
	}
}