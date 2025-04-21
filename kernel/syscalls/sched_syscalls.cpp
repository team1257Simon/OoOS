#include "sched/scheduler.hpp"
#include "sched/task_list.hpp"
#include "frame_manager.hpp"
#include "prog_manager.hpp"
#include "kernel_mm.hpp"
#include "errno.h"
#include "elf64_exec.hpp"
#include "arch/arch_amd64.h"
#include "rtc.h"
#include "kdebug.hpp"
extern "C"
{
    [[noreturn]] void handle_exit()
    {
        cli();
        if(task_ctx* task = active_task_context(); task->is_user()) { task->terminate(); tl.destroy_task(task->get_pid()); }
        if(task_t* next = sch.fallthrough_yield()) { fallthrough_reentry(next); }
        else { kernel_reentry(); }
        __builtin_unreachable();
    }
    clock_t syscall_times(tms* out) { out = translate_user_pointer(out); if(task_ctx* task = active_task_context(); task->is_user() && out) { new(out) tms{ active_task_context()->get_times() }; return sys_time(nullptr); } else return -EINVAL; }
    int syscall_gettimeofday(timeval* restrict tm, void* restrict tz) { std::construct_at<timeval>(translate_user_pointer(tm), timestamp_to_timeval(rtc::get_instance().get_timestamp())); return 0; } 
    long syscall_getpid() { if(task_ctx* task = active_task_context(); task->is_user()) return static_cast<long>(task->get_pid()); else return 0L; /* Not an error technically; system tasks are PID 0 */ }
    void syscall_exit(int n) { if(task_ctx* task = active_task_context(); task->is_user()) { task->set_exit(n); } }
    int syscall_sleep(unsigned long seconds)
    {
        if(task_ctx* task = active_task_context(); sch.set_wait_timed(task->task_struct.self, seconds * 1000, false)) 
        { 
            task_t* next = sch.manual_yield();
            if(next == task->task_struct.self.as<task_t>())
            {
                sti();
                while(task->task_struct.task_ctl.block) pause();
                cli();
                return 0;
            }
            task->task_struct.saved_regs.rax = 0;
            return next->saved_regs.rax;
        }
        return -ENOSYS;
    }
    pid_t syscall_wait(int* sc_out)
    {
        task_ctx* task = active_task_context();
        sc_out = translate_user_pointer(sc_out);
        if(task->last_notified) { if(sc_out) *sc_out = task->last_notified->exit_code; return task->last_notified->get_pid(); } 
        else if(sch.set_wait_untimed(task->task_struct.self)) 
        {
            task->notif_target = sc_out;
            task->task_struct.task_ctl.notify_cterm = true;
            task_t* next = sch.manual_yield();
            if(next == task->task_struct.self.as<task_t>()) { return -ECHILD; }
            return next->saved_regs.rax;
        }
        return -EINVAL;
    }
    long syscall_fork()
    {
        task_ctx* task = active_task_context();
        // TODO check/enforce thread limit
        if(task_ctx* clone = tl.context_vfork(task); clone && clone->set_fork())
        {
            try { sch.register_task(clone->task_struct.self); } catch(...) { return -ENOMEM; }
            clone->current_state = execution_state::RUNNING;
            task->add_child(clone);
            clone->task_struct.saved_regs.rax = 0;
            return clone->get_pid();
        }
        else return -ENOMEM;
    }
    int syscall_execve(char* name, char** argv, char** env)
    {
        task_ctx* task = active_task_context();
        filesystem* fs_ptr = get_fs_instance();
        if(!fs_ptr) return -ENOSYS;
        name = translate_user_pointer(name);
        if(!name) return -EINVAL;
        file_node* n;
        try{ n = fs_ptr->open_file(name, std::ios_base::in); } catch(std::exception& e) { panic(e.what()); return -ENOENT; }
        elf64_executable* ex = prog_manager::get_instance().add(n);
        if(!ex) return -ENOEXEC;
        std::vector<const char*> argv_v{}, env_v{};
        for(size_t i = 0; argv[i]; ++i) argv_v.push_back(argv[i]);
        for(size_t i = 0; env[i]; ++i) env_v.push_back(env[i]);
        task->exit_code = 0;
        file_node* stdio_ptrs[3] = { task->stdio_ptrs[0], task->stdio_ptrs[1], task->stdio_ptrs[2] };
        addr_t exit_target = task->exit_target;
        priority_val pv = task->task_struct.task_ctl.prio_base;
        uint16_t qv = task->task_struct.quantum_val;
        pid_t parent_id = task->get_parent_pid();
        pid_t id = task->get_pid();
        sch.unregister_task(task->task_struct.self);
        tl.destroy_task(id);
        task_ctx* ntask = tl.create_user_task(ex->describe(), std::move(argv_v), parent_id, pv, qv, id);
        ntask->env_vec = std::move(env_v);
        ntask->set_stdio_ptrs(stdio_ptrs);
        ntask->start_task(exit_target);
        asm volatile("swapgs; wrgsbase %0; swapgs" :: "r"(ntask) : "memory");
        return 0;
    }
}