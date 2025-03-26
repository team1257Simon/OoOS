#include "sched/scheduler.hpp"
#include "sched/task_list.hpp"
#include "frame_manager.hpp"
#include "kernel_mm.hpp"
#include "errno.h"
#include "elf64_exec.hpp"
#include "arch/arch_amd64.h"
#include "rtc.h"
static bool check_kill(task_ctx* caller, task_list::iterator target) { if(caller->is_system() || static_cast<uint64_t>(target->get_parent_pid()) == caller->get_pid()) return true; for(task_ctx* c : caller->child_tasks) { if(check_kill(c, target)) return true; } return false; }
extern "C"
{
    [[noreturn]] void handle_exit() { cli(); if(task_ctx* task = active_task_context(); task->is_user()) { task->terminate(); task_list::get().destroy_task(task->get_pid()); } sti(); kernel_reentry(); __builtin_unreachable(); }
    clock_t syscall_times(tms *out) { out = translate_user_pointer(out); if(task_ctx* task = active_task_context(); task->is_user() && out) { new (out) tms{ active_task_context()->get_times() }; return sys_time(nullptr); } else return -EINVAL; }
    int syscall_gettimeofday(timeval* restrict tm, void* restrict tz) { std::construct_at<timeval>(translate_user_pointer(tm), timestamp_to_timeval(rtc::get_instance().get_timestamp())); return 0; } 
    long syscall_getpid() { if(task_ctx* task = active_task_context(); task->is_user()) return static_cast<long>(task->get_pid()); else return 0L; /* Not an error technically; system tasks are PID 0 */ }
    long syscall_fork() { return -ENOSYS; /* NYI */ }
    void syscall_exit(int n) { if(task_ctx* task = active_task_context(); task->is_user()) { task->set_exit(n); } }
    void on_invalid_syscall() { panic("invalid syscall"); active_task_context()->set_exit(-1); /* bounds-check syscall numbers. */ }
    int syscall_kill(unsigned long pid, unsigned long sig) { if(task_ctx* task = active_task_context()) { if(task_list::iterator target = task_list::get().find(pid); !target->is_system()) { if(!check_kill(task, target)) return -EPERM; target->task_struct.task_ctl.sigkill = true; target->task_struct.task_ctl.signal_num = sig; target->set_exit(static_cast<int>(sig)); return 0; } } return -EINVAL; }
    pid_t syscall_wait(int* sc_out) { task_ctx* task = active_task_context(); sc_out = translate_user_pointer(sc_out); if(task->last_notified) { *sc_out = task->last_notified->exit_code; return task->last_notified->get_pid(); } else if(scheduler::get().set_wait_untimed(task->task_struct.self)) { task->notif_target = sc_out; task->task_struct.task_ctl.notify_cterm = true; while(task->task_struct.task_ctl.block) { pause(); } return task->last_notified ? task->last_notified->get_pid() : -EINTR; } return -EINVAL; }
    int syscall_sleep(unsigned long seconds) { if(task_ctx* task = active_task_context(); scheduler::get().set_wait_timed(task->task_struct.self, seconds * 1000, false)) { while(task->task_struct.task_ctl.block) { pause(); } return 0; } return -ENOSYS; }
    int syscall_execve(char* name, char** argv, char** env) 
    { 
        task_ctx* task = get_gs_base<task_ctx>();
        filesystem* fs_ptr = task->get_vfs_ptr();
        if(!fs_ptr) return -ENOSYS;
        name = translate_user_pointer(name);
        argv = translate_user_pointer(argv);
        env = translate_user_pointer(env);
        file_node* n = nullptr;
        try
        {
            n = fs_ptr->open_file(name, std::ios_base::in);
            if(!(n->mode.exec_group || n->mode.exec_owner || n->mode.exec_others)) { fs_ptr->close_file(n); return -EPERM; }
            elf64_executable exec{ n };
            if(!exec.load()) { fs_ptr->close_file(n); return -ENOEXEC; }
            fs_ptr->close_file(n);
            std::vector<const char*> xargv{};
            std::vector<const char*> xenv{};
            for(char** c = argv; *c; c++) xargv.push_back(*c);
            for(char** c = env; *c; c++) xenv.push_back(*c);
            xenv.push_back(nullptr);
            task_ctx* ctx = task_list::get().create_user_task(exec.describe(), std::move(xargv), task->get_pid(), task->task_struct.task_ctl.prio_base, task->task_struct.quantum_val);
            ctx->env_vec = std::move(xenv);
            ctx->set_stdio_ptrs(task->stdio_ptrs);
            ctx->init_task_state();
            ctx->start_task(task->exit_target);
            return 0;
        }
        catch(std::exception& e) { panic(e.what()); if(n) { fs_ptr->close_file(n); return -EPIPE; } else return -EAGAIN; } 
        return -EINVAL;
    }
}