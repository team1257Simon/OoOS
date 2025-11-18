#include <sched/worker.hpp>
#include <sched/scheduler.hpp>
extern "C"
{
	int worker_add(ooos::worker* w) { scheduler::add_worker_task(std::addressof(w->task_struct)); return 0; }
	int worker_entry(ooos::worker* w) { return w ? w->fn() : -1; }
	[[noreturn]] void worker_finish(ooos::worker* w, int code)
	{ 
		scheduler::remove_worker_task(std::addressof(w->task_struct));
		asm volatile("sti" ::: "memory");
		longjmp(w->caller_info, code ? code : 1);
		__builtin_unreachable();
	}
}
namespace ooos
{
	void worker::init_state()
	{
		fx_save(std::addressof(task_struct));
		array_zero(task_struct.fxsv.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
		for(int i = 0; i < 8; i++) task_struct.fxsv.stmm[i] = 0.L;
		array_zero(reinterpret_cast<void**>(worker_sig_info.signal_handlers), num_signals);
		stack_push(addr_t(worker_exit));
	}
	void worker::stack_push(addr_t value) {
		task_struct.saved_regs.rsp -= 8Z;
		task_struct.saved_regs.rsp.assign(value);
	}
}