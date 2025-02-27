#include "sched/scheduler.hpp"
#include "sched/task_list.hpp"
#include "frame_manager.hpp"
#include "kernel_mm.hpp"
#include "errno.h"
#include "fs/fat32.hpp"
#include "elf64_exec.hpp"
#include "arch/com_amd64.h"
#include "isr_table.hpp"
static inline addr_t get_applicable_cr3(addr_t frame_ptr) { if(static_cast<uframe_tag*>(frame_ptr)->magic == UFRAME_MAGIC) return static_cast<uframe_tag*>(frame_ptr)->pml4; else return get_cr3(); }
filesystem *task_ctx::get_vfs_ptr() { return ctx_filesystem; }
task_ctx::task_ctx(task_ctx const &that) : task_ctx{ reinterpret_cast<task_functor>(that.task_struct.saved_regs.rip.operator void*()), std::vector<const char*>{ that.arg_vec }, that.allocated_stack, static_cast<ptrdiff_t>(that.stack_allocated_size), that.tls, that.tls_size, &(frame_manager::get().duplicate_frame(*(that.task_struct.frame_ptr.operator uframe_tag*()))), task_list::get().__mk_pid(), static_cast<int64_t>(that.get_pid()), that.task_struct.task_ctl.prio_base, that.task_struct.quantum_val } { set_stdio_ptrs(that.stdio_ptrs[0], that.stdio_ptrs[1], that.stdio_ptrs[2]); }
static bool check_kill(task_ctx* caller, task_list::iterator target) { if(caller->is_system() || static_cast<uint64_t>(target->get_parent_pid()) == caller->get_pid()) return true; for(task_ctx* c : caller->child_tasks) { if(check_kill(c, target)) return true; } return false; }
task_list::iterator ctx_fork(task_ctx const& t) { task_list::iterator result = task_list::get().emplace(t).first; result->init_task_state(); result->start_task(t.exit_target); return result; }
void sys_task_exit()
{
    
    int retv;
    asm volatile("movl %%eax, %0" : "=r"(retv) :: "memory");
    get_gs_base<task_ctx>()->set_exit(retv);
}
void task_ctx::set_stdio_ptrs(file_node *stdin, file_node *stdout, file_node *stderr)
{
    stdio_ptrs[0] = stdin;
    stdio_ptrs[1] = stdout;
    stdio_ptrs[2] = stderr;
}
void task_ctx::init_task_state()
{
    task_struct.saved_regs.rsi = std::bit_cast<register_t>(arg_vec.data());   
    task_struct.saved_regs.rdi = arg_vec.size();
    asm volatile("fxsave %0" : "=m"(task_struct.fxsv) :: "memory");
    __builtin_memset(task_struct.fxsv.xmm, 0, sizeof(task_struct.fxsv.xmm));
    for(int i = 0; i < 8; i++) { task_struct.fxsv.stmm[i] = 0.L; }
    if(is_system()) { task_struct.saved_regs.cs = 0x08; task_struct.saved_regs.ds = task_struct.saved_regs.ss = 0x10; }
    else
    {
        task_struct.saved_regs.cs = 0x23;
        task_struct.saved_regs.ds = task_struct.saved_regs.ss = 0x1B;
        kernel_memory_mgr::get().enter_frame(task_struct.frame_ptr);
        kernel_memory_mgr::get().identity_map_to_user(this, sizeof(task_ctx), true, false);
        kernel_memory_mgr::get().identity_map_to_user(arg_vec.data(), (arg_vec.size() + 1UL)* sizeof(char*), true, false);
        for(const char* str : arg_vec) { if(str) kernel_memory_mgr::get().identity_map_to_user(str, std::strlen(str), true, false); }
        kernel_memory_mgr::get().exit_frame();
    }
}
task_ctx::task_ctx(task_functor task, std::vector<const char*>&& args, addr_t stack_base, ptrdiff_t stack_size, addr_t tls_base, size_t tls_len, addr_t frame_ptr, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum) : 
    task_struct 
    { 
        &task_struct, 
        frame_ptr, 
        regstate_t
        {
            .rbp = stack_base + stack_size, 
            .rsp = stack_base + stack_size,
            .rip = addr_t{ task },
            .rflags = 0x202UL,
            .cr3 = get_applicable_cr3(frame_ptr)
        }, 
        quantum, 
        0U, 
        tcb_t 
        {
            { 
                false, 
                false, 
                false, 
                false, 
                prio 
            },
            0U, 
            0U, 
            0U, 
            parent_pid, 
            pid
        }, 
        fx_state{}, 
        0UL,
        0UL, 
        0UL, 
        nullptr, 
        0UL, 
        tls_base, 
        nullptr 
    },
    arg_vec{ std::move(args) }, 
    allocated_stack{ stack_base }, 
    stack_allocated_size{ static_cast<size_t>(stack_size) }, 
    tls{ tls_base }, 
    tls_size{ tls_len }, 
    ctx_filesystem{ fat32::get_instance() } 
    {}
void task_ctx::add_child(task_ctx *that) { that->task_struct.task_ctl.parent_pid = this->task_struct.task_ctl.task_id; child_tasks.push_back(that); task_struct.num_child_procs = child_tasks.size(); task_struct.child_procs = reinterpret_cast<addr_t*>(child_tasks.data()); }
bool task_ctx::remove_child(task_ctx *that) { if(std::vector<task_ctx*>::const_iterator i = child_tasks.find(that); i != child_tasks.end()) { child_tasks.erase(i); return true; } return false; }
void task_ctx::start_task(addr_t exit_fn)
{
    if(env_vec.empty() || env_vec.back()) { env_vec.push_back(nullptr); }
    task_struct.saved_regs.rdx = uintptr_t(addr_t{ env_vec.data() });
    this->exit_target = exit_fn;  
    if(is_user())
    {
        kernel_memory_mgr::get().enter_frame(task_struct.frame_ptr);
        kernel_memory_mgr::get().identity_map_to_user(env_vec.data(), (env_vec.size() + 1UL) * sizeof(char*), true, false);
        for(const char* str : env_vec) { if(str) kernel_memory_mgr::get().identity_map_to_user(str, std::strlen(str), true, false); }
        kernel_memory_mgr::get().exit_frame();
        interrupt_table::map_interrupt_callbacks(task_struct.frame_ptr);
    }
    else *static_cast<uintptr_t*>(task_struct.saved_regs.rsp) = addr_t(&sys_task_exit);
    scheduler::get().register_task(this->task_struct.self);
    this->current_state = execution_state::RUNNING;
}
void task_ctx::set_exit(int n) 
{
    this->exit_code = n;    
    if(this->exit_target) 
    {
        task_ctx* c = this, *p = nullptr;
        while(c->get_parent_pid() > 0 && task_list::get().contains(static_cast<uint64_t>(c->get_parent_pid())))
        {
            p = std::addressof(*task_list::get().find(static_cast<uint64_t>(c->get_parent_pid())));
            if(p->task_struct.task_ctl.notify_cterm && p->task_struct.task_ctl.block && scheduler::get().interrupt_wait(p->task_struct.self) && p->notif_target) *static_cast<int*>(p->notif_target) = n;
            p->last_notified = this;
            c = p;
        }
        static_cast<void(*)()>(exit_target)();
    }
    else handle_exit();
    __builtin_unreachable();    
}
void task_ctx::terminate()
{
    this->current_state = execution_state::TERMINATED;
    scheduler::get().unregister_task(task_struct.self);
    for(task_ctx* c : this->child_tasks) { if(c->current_state == execution_state::RUNNING) { if(exit_code) c->exit_code = exit_code; c->terminate(); } }
    if(is_user()) frame_manager::get().destroy_frame(*static_cast<uframe_tag*>(this->task_struct.frame_ptr));
}
tms task_ctx::get_times() const noexcept
{
    tms result{ task_struct.run_time, task_struct.sys_time, 0UL, 0UL };
    for(task_ctx* child : child_tasks)
    {
        tms add_ct = child->get_times();
        result.tms_cstime += (add_ct.tms_cstime + add_ct.tms_stime);
        result.tms_cutime += (add_ct.tms_cutime + add_ct.tms_utime);
    }
    return result;
}
extern "C"
{
    [[noreturn]] void handle_exit() { cli(); if(task_ctx* task = active_task_context(); task->is_user()) { task->terminate(); task_list::get().destroy_task(task->get_pid()); } sti(); kernel_reentry(); __builtin_unreachable(); }
    clock_t syscall_times(tms *out) { out = translate_user_pointer(out); if(task_ctx* task = active_task_context(); task->is_user() && out) { new (out) tms{ active_task_context()->get_times() }; return syscall_time(nullptr); } else return -EINVAL; }
    long syscall_getpid() { if(task_ctx* task = active_task_context(); task->is_user()) return static_cast<long>(task->get_pid()); else return 0L; /* Not an error technically; system tasks are PID 0 */ }
    long syscall_fork() { try { if(task_ctx* task = active_task_context()) { if(task_list::iterator result = ctx_fork(*task); result != task_list::get().end()) { return static_cast<long>(result->get_pid()); } else return -EAGAIN; } } catch(std::exception& e) { panic(e.what()); return -ENOMEM; } return -EINVAL; }
    void syscall_exit(int n) { if(task_ctx* task = active_task_context(); task->is_user()) { task->set_exit(n); } }
    int syscall_kill(unsigned long pid, unsigned long sig) { if(task_ctx* task = active_task_context()) { if(task_list::iterator target = task_list::get().find(pid); !target->is_system()) { if(!check_kill(task, target)) return -EPERM; target->task_struct.task_ctl.sigkill = true; target->task_struct.task_ctl.signal_num = sig; target->set_exit(static_cast<int>(sig)); return 0; } } return -EINVAL; }
    pid_t syscall_wait(int *sc_out) { task_ctx* task = active_task_context(); sc_out = translate_user_pointer(sc_out); if(task->last_notified) { *sc_out = task->last_notified->exit_code; return task->last_notified->get_pid(); } else if(scheduler::get().set_wait_untimed(task->task_struct.self)) { task->notif_target = sc_out; task->task_struct.task_ctl.notify_cterm = true; while(task->task_struct.task_ctl.block) { PAUSE; } return task->last_notified ? task->last_notified->get_pid() : -EINTR; } return -EINVAL; }
    int syscall_sleep(unsigned long seconds) { if(task_ctx* task = active_task_context(); scheduler::get().set_wait_timed(task->task_struct.self, seconds * 1000, false)) { while(task->task_struct.task_ctl.block) { PAUSE; } return 0; } return -ENOSYS; }
    int syscall_execve(char *name, char **argv, char **env) 
    { 
        task_ctx* task = get_gs_base<task_ctx>();
        filesystem* fs_ptr = task->get_vfs_ptr();
        if(!fs_ptr) return -ENOSYS;
        name = translate_user_pointer(name);
        argv = translate_user_pointer(argv);
        env = translate_user_pointer(env);
        file_node* n{ nullptr };
        try
        {
            n = fs_ptr->open_file(name, std::ios_base::in);
            if(!(n->mode.exec_group || n->mode.exec_owner || n->mode.exec_others)) { fs_ptr->close_file(n); return -EPERM; }
            elf64_executable exec{ n };
            if(!exec.load()) { fs_ptr->close_file(n); return -ENOEXEC; }
            task_ctx* i = task_list::get().create_user_task(exec.describe(), std::vector<const char*>{}, task->get_pid(), task->task_struct.task_ctl.prio_base, task->task_struct.quantum_val);
            for(char** c = env; *c; c++) i->env_vec.push_back(*c);
            for(char** c = argv; *c; c++) i->arg_vec.push_back(*c);
            i->init_task_state();
            i->env_vec.push_back(nullptr);
            i->start_task(task->exit_target);
            fs_ptr->close_file(n);
            return 0;
        }
        catch(std::exception& e) { panic(e.what()); if(n) { fs_ptr->close_file(n); return -EPIPE; } else return -EAGAIN; } 
        return -EINVAL;
    }
}