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
    extern task_t* kproc;
    [[noreturn]] void handle_exit()
    {
        cli();
        task_ctx* task = active_task_context();
        if(task->is_user()) { task->terminate(); tl.destroy_task(task->get_pid()); }
        if(task_t* next = sch.fallthrough_yield(); next != nullptr) { fallthrough_reentry(next); }
        else { kernel_reentry(); }
        __builtin_unreachable();
    }
    clock_t syscall_times(tms* out) { out = translate_user_pointer(out); if(!out) return -EFAULT; if(task_ctx* task = active_task_context(); task->is_user()) { new(out) tms{ active_task_context()->get_times() }; return sys_time(nullptr); } else return -ENOSYS; }
    int syscall_gettimeofday(timeval* restrict tm, void* restrict tz) { tm = translate_user_pointer(tm); if(!tm) return -EFAULT; std::construct_at<timeval>(tm, timestamp_to_timeval(rtc::get_instance().get_timestamp())); return 0; } 
    long syscall_getpid() { if(task_ctx* task = active_task_context(); task->is_user()) return static_cast<long>(task->get_pid()); else return 0L; /* Not an error technically; system tasks are PID 0 */ }
    void syscall_exit(int n) { if(task_ctx* task = active_task_context(); task->is_user()) { task->set_exit(n); } }
    int syscall_sleep(unsigned long seconds)
    {
        if(task_ctx* task = active_task_context(); sch.set_wait_timed(reinterpret_cast<task_t*>(task), seconds * 1000, false)) 
        { 
            task_t* next = sch.manual_yield();
            if(next == reinterpret_cast<task_t*>(task))
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
        else if(sch.set_wait_untimed(reinterpret_cast<task_t*>(task))) 
        {
            task->notif_target = sc_out;
            task->task_struct.task_ctl.notify_cterm = true;
            task_t* next = sch.manual_yield();
            if(next == reinterpret_cast<task_t*>(task)) { return -ECHILD; }
            return next->saved_regs.rax;
        }
        return -ENOMEM;
    }
    long syscall_fork()
    {
        task_ctx* task = active_task_context();
        if(task_ctx* clone = tl.task_vfork(task); clone && clone->set_fork())
        {
            try { sch.register_task(reinterpret_cast<task_t*>(clone)); } catch(...) { return -ENOMEM; }
            clone->current_state = execution_state::RUNNING;
            task->add_child(clone);
            clone->task_struct.saved_regs.rax = 0;
            return clone->get_pid();
        }
        else return -EAGAIN;
    }
    long syscall_vfork()
    {
        task_ctx* task = active_task_context();
        if(task_ctx* clone = tl.task_vfork(task); clone && sch.set_wait_untimed(reinterpret_cast<task_t*>(task)))
        {
            try { sch.register_task(reinterpret_cast<task_t*>(clone)); } catch(...) { return -ENOMEM; }
            clone->current_state = execution_state::RUNNING;
            task->add_child(clone);
            task->task_struct.task_ctl.notify_cterm = true;
            clone->task_struct.saved_regs.rax = 0;
            task->task_struct.saved_regs.rax = clone->get_pid();
            task_t* next = sch.manual_yield();
            if(next == reinterpret_cast<task_t*>(task))
            { 
                next = clone->task_struct.self;
                next->quantum_rem = next->quantum_val;
                asm volatile("swapgs; wrgsbase %0; swapgs" :: "r"(next) : "memory");
            }
            return next->saved_regs.rax;
        }
        else return -EAGAIN;
    }
    int syscall_execve(char* restrict name, char** restrict argv, char** restrict env)
    {
        task_ctx* task = active_task_context();
        filesystem* fs_ptr = get_fs_instance();
        if(!fs_ptr) return -ENOSYS;
        name = translate_user_pointer(name);
        if(!name) return -EFAULT;
        file_node* n;
        try { n = fs_ptr->open_file(name, std::ios_base::in); } catch(std::exception& e) { panic(e.what()); return -ENOENT; }
        elf64_executable* ex = prog_manager::get_instance().add(n);
        if(!ex) return -ENOEXEC;
        std::vector<const char*> argv_v{}, env_v{};
        for(size_t i = 0; argv[i]; ++i) argv_v.push_back(argv[i]);
        for(size_t i = 0; env[i]; ++i) env_v.push_back(env[i]);
        try
        {
            if(task_list::iterator parent = tl.find(task->get_parent_pid()); parent != tl.end())
                sch.interrupt_wait(parent->task_struct.self);
            if(task->subsume(ex->describe(), std::move(argv_v), std::move(env_v))) 
                return task->task_struct.saved_regs.rax;
            else return -ENOMEM;
        }
        catch(std::invalid_argument& e) { panic(e.what()); return -ECANCELED; }
        catch(std::out_of_range& e) { panic(e.what()); return -EFAULT; }
        catch(std::bad_alloc&) { panic("no memory for argument vectors"); return -ENOMEM; }
        __builtin_unreachable();
    }
    long syscall_spawn(char* restrict name, char** restrict argv, char** restrict env)
    {
        task_ctx* task = active_task_context();
        filesystem* fs_ptr = get_fs_instance();
        if(!fs_ptr) return -ENOSYS;
        name = translate_user_pointer(name);
        if(!name) return -EFAULT;
        file_node* n;
        try { n = fs_ptr->open_file(name, std::ios_base::in); } catch(std::exception& e) { panic(e.what()); return -ENOENT; }
        elf64_executable* ex = prog_manager::get_instance().add(n);
        if(!ex) return -ENOEXEC;
        std::vector<const char*> argv_v{}, env_v{};
        for(size_t i = 0; argv[i]; ++i) argv_v.push_back(argv[i]);
        for(size_t i = 0; env[i]; ++i) env_v.push_back(env[i]);
        if(task_ctx* clone = tl.task_vfork(task))
        {
            try { sch.register_task(reinterpret_cast<task_t*>(clone)); } catch(...) { return -ENOMEM; }
            clone->current_state = execution_state::RUNNING;
            task->add_child(clone);
            try
            {
                if(clone->subsume(ex->describe(), std::move(argv_v), std::move(env_v))) 
                    return clone->get_pid();
                else return -ENOMEM;
            }
            catch(std::invalid_argument& e) { panic(e.what()); return -ECANCELED; }
            catch(std::out_of_range& e) { panic(e.what()); return -EFAULT; }
            catch(std::bad_alloc&) { panic("no memory for argument vectors"); return -ENOMEM; }            
        }
        return -EAGAIN;
    }
}