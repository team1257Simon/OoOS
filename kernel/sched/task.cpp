#include "sched/scheduler.hpp"
#include "sched/task_list.hpp"
#include "frame_manager.hpp"
#include "kernel_mm.hpp"
#include "errno.h"
#include "fs/fat32.hpp"
#include "elf64_exec.hpp"
#include "arch/com_amd64.h"
#include "isr_table.hpp"
static std::allocator<shared_object_map> sm_alloc{};
static inline addr_t __pml4_of(addr_t frame_ptr) { if(frame_ptr.as<uframe_tag>()->magic == uframe_magic) return frame_ptr.as<uframe_tag>()->pml4; else return get_cr3(); }
filesystem *task_ctx::get_vfs_ptr() { return ctx_filesystem; }
static bool check_kill(task_ctx* caller, task_list::iterator target) { if(caller->is_system() || static_cast<uint64_t>(target->get_parent_pid()) == caller->get_pid()) return true; for(task_ctx* c : caller->child_tasks) { if(check_kill(c, target)) return true; } return false; }
void task_ctx::add_child(task_ctx* that) { that->task_struct.task_ctl.parent_pid = this->task_struct.task_ctl.task_id; child_tasks.push_back(that); task_struct.num_child_procs = child_tasks.size(); task_struct.child_procs = reinterpret_cast<addr_t*>(child_tasks.data()); }
bool task_ctx::remove_child(task_ctx* that) { if(std::vector<task_ctx*>::const_iterator i = child_tasks.find(that); i != child_tasks.end()) { child_tasks.erase(i); return true; } return false; }
void sys_task_exit() { int retv; asm volatile("movl %%eax, %0" : "=r"(retv) :: "memory"); get_gs_base<task_ctx>()->set_exit(retv); }
void task_ctx::set_stdio_ptrs(std::array<file_node*, 3>&& ptrs) { array_copy(stdio_ptrs, ptrs.data(), 3UL); }
void task_ctx::set_stdio_ptrs(file_node* ptrs[3]) { array_copy(stdio_ptrs, ptrs, 3UL); }
task_ctx::~task_ctx()
{ 
    if(local_so_map)
    { 
        shared_object_map& globals = shared_object_map::get_globals();
        std::vector<shared_object_map::iterator> stickies{};
        for(shared_object_map::iterator i = local_so_map->begin(); i != local_so_map->end(); i++) { if(i->is_sticky()) { stickies.push_back(i); } }
        for(shared_object_map::iterator i : stickies) { local_so_map->transfer(globals, i); }
        sm_alloc.deallocate(local_so_map, 1); 
    }
}
void task_ctx::init_task_state()
{
    task_struct.saved_regs.rsi = std::bit_cast<register_t>(arg_vec.data());   
    task_struct.saved_regs.rdi = arg_vec.size();
    fx_save(std::addressof(task_struct));
    __builtin_memset(task_struct.fxsv.xmm, 0, sizeof(task_struct.fxsv.xmm));
    for(int i = 0; i < 8; i++) { task_struct.fxsv.stmm[i] = 0.L; }
    if(is_system()) { task_struct.saved_regs.cs = 0x08; task_struct.saved_regs.ds = task_struct.saved_regs.ss = 0x10; }
    else
    {
        task_struct.saved_regs.cs = 0x23;
        task_struct.saved_regs.ds = task_struct.saved_regs.ss = 0x1B;
        kernel_memory_mgr::get().enter_frame(task_struct.frame_ptr);
        kernel_memory_mgr::get().identity_map_to_user(this, sizeof(task_ctx), true, false);
        kernel_memory_mgr::get().identity_map_to_user(arg_vec.data(), (arg_vec.size() + 1UL) * sizeof(char*), true, false);
        for(const char* str : arg_vec) { if(str) kernel_memory_mgr::get().identity_map_to_user(str, std::strlen(str), true, false); }
        kernel_memory_mgr::get().exit_frame();
    }
}
void task_ctx::init_task_state(task_ctx const& that)
{
    this->task_struct.saved_regs = that.task_struct.saved_regs;
    this->task_struct.saved_regs.cr3 = __pml4_of(this->task_struct.frame_ptr);
    this->task_struct.fxsv = that.task_struct.fxsv;
    if(is_user())
    {
        kernel_memory_mgr::get().enter_frame(task_struct.frame_ptr);
        kernel_memory_mgr::get().identity_map_to_user(this, sizeof(task_ctx), true, false);
        kernel_memory_mgr::get().identity_map_to_user(arg_vec.data(), (arg_vec.size() + 1UL) * sizeof(char*), true, false);
        for(const char* str : arg_vec) { if(str) kernel_memory_mgr::get().identity_map_to_user(str, std::strlen(str), true, false); }
        kernel_memory_mgr::get().exit_frame();
    }
}
task_ctx::task_ctx(task_functor task, std::vector<const char*>&& args, addr_t stack_base, ptrdiff_t stack_size, addr_t tls_base, size_t tls_len, addr_t frame_ptr, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum) : 
    task_struct
    { 
        .self               { std::addressof(task_struct) }, 
        .frame_ptr          { frame_ptr }, 
        .saved_regs         { .rbp{ stack_base + stack_size }, .rsp{ stack_base + stack_size }, .rip{ task }, .rflags{ 0x202UL }, .cr3{ __pml4_of(frame_ptr) } }, 
        .quantum_val        { quantum }, 
        .task_ctl           { { false, false, false, false, prio }, 0U, 0U, 0U, parent_pid, pid },
        .tls_block          { tls_base }
    },
    arg_vec                 { std::move(args) }, 
    allocated_stack         { stack_base }, 
    stack_allocated_size    { static_cast<size_t>(stack_size) }, 
    tls                     { tls_base }, 
    tls_size                { tls_len }, 
    ctx_filesystem          { create_task_vfs() },
    local_so_map            { is_user() ? sm_alloc.allocate(1) : nullptr }
                            { if(local_so_map) { std::construct_at(local_so_map, frame_ptr); } }
task_ctx::task_ctx(elf64_program_descriptor const& desc, std::vector<const char *>&& args, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum) :
    task_struct
    {
        .self               { std::addressof(task_struct) },
        .frame_ptr          { desc.frame_ptr },
        .saved_regs         { .rbp{ addr_t(desc.prg_stack).plus(desc.stack_size) }, .rsp{ addr_t(desc.prg_stack).plus(desc.stack_size)}, .rip{ desc.entry }, .rflags{ 0x202UL }, .cr3{ __pml4_of(desc.frame_ptr) } },
        .quantum_val        { quantum }, 
        .task_ctl           { { false, false, false, false, prio }, 0U, 0U, 0U, parent_pid, pid },
        .tls_block          { desc.prg_tls }
    },
    arg_vec                 { std::move(args) },
    dl_search_paths         ( desc.ld_path_count ),
    allocated_stack         { desc.prg_stack },
    stack_allocated_size    { desc.stack_size },
    tls                     { desc.prg_tls },
    tls_size                { desc.tls_size },
    ctx_filesystem          { create_task_vfs() },
    object_handle           { static_cast<elf64_object*>(desc.object_handle) },
    local_so_map            { sm_alloc.allocate(1) }
{ std::construct_at(local_so_map, static_cast<uframe_tag*>(desc.frame_ptr)); if(desc.ld_path && desc.ld_path_count) { dl_search_paths.push_back(desc.ld_path, desc.ld_path + desc.ld_path_count); } }
void task_ctx::start_task(addr_t exit_fn)
{
    if(env_vec.empty() || env_vec.back()) { env_vec.push_back(nullptr); }
    task_struct.saved_regs.rdx = uintptr_t(addr_t{ env_vec.data() });
    exit_target = exit_fn;  
    if(is_user())
    {
        kernel_memory_mgr::get().enter_frame(task_struct.frame_ptr);
        kernel_memory_mgr::get().identity_map_to_user(env_vec.data(), (env_vec.size() + 1UL) * sizeof(char*), true, false);
        for(const char* str : env_vec) { if(str) kernel_memory_mgr::get().identity_map_to_user(str, std::strlen(str), true, false); }
        kernel_memory_mgr::get().exit_frame();
        interrupt_table::map_interrupt_callbacks(task_struct.frame_ptr);
    }
    else *task_struct.saved_regs.rsp.as<uintptr_t>() = addr_t(std::addressof(sys_task_exit));
    scheduler::get().register_task(task_struct.self);
    current_state = execution_state::RUNNING;
}
void task_ctx::set_exit(int n) 
{
    exit_code = n;    
    if(exit_target) 
    {
        task_ctx* c = this, *p = nullptr;
        while(c->get_parent_pid() > 0 && task_list::get().contains(static_cast<uint64_t>(c->get_parent_pid())))
        {
            p = std::addressof(*task_list::get().find(static_cast<uint64_t>(c->get_parent_pid())));
            if(p->task_struct.task_ctl.notify_cterm && p->task_struct.task_ctl.block && scheduler::get().interrupt_wait(p->task_struct.self) && p->notif_target) p->notif_target.ref<int>() = n;
            p->last_notified = this;
            c = p;
        }
        exit_target.ref<void()>()();
    }
    else handle_exit();
    __builtin_unreachable();    
}
void task_ctx::terminate()
{
    current_state = execution_state::TERMINATED;
    scheduler::get().unregister_task(task_struct.self);
    for(task_ctx* c : child_tasks) { if(c->current_state == execution_state::RUNNING) { if(exit_code) { c->exit_code = exit_code; c->terminate(); } } }
    if(is_user()) {frame_manager::get().destroy_frame(task_struct.frame_ptr.ref<uframe_tag>()); }
    if(local_so_map) { local_so_map->shared_frame = nullptr; } 
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
void task_exec(elf64_program_descriptor const& prg, std::vector<const char*>&& args, std::vector<const char*>&& env, std::array<file_node*, 3>&& stdio_ptrs, addr_t exit_fn, int64_t parent_pid, priority_val pv, uint16_t quantum)
{
    task_ctx* ctx = task_list::get().create_user_task(prg, std::move(args), parent_pid, pv, quantum);
    ctx->env_vec = std::move(env);
    ctx->init_task_state();
    ctx->set_stdio_ptrs(std::move(stdio_ptrs));
    if(exit_fn) { ctx->start_task(exit_fn); }
    else { ctx->start_task(); }
    user_entry(ctx->task_struct.self);
}
extern "C"
{
    [[noreturn]] void handle_exit() { cli(); if(task_ctx* task = active_task_context(); task->is_user()) { task->terminate(); task_list::get().destroy_task(task->get_pid()); } sti(); kernel_reentry(); __builtin_unreachable(); }
    clock_t syscall_times(tms *out) { out = translate_user_pointer(out); if(task_ctx* task = active_task_context(); task->is_user() && out) { new (out) tms{ active_task_context()->get_times() }; return sys_time(nullptr); } else return -EINVAL; }
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