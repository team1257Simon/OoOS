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
filesystem* task_ctx::get_vfs_ptr() { return ctx_filesystem; }
constexpr static uint64_t get_frame_magic(addr_t tag) { return tag.ref<uint64_t>(); }
void task_ctx::add_child(task_ctx* that) { that->task_struct.task_ctl.parent_pid = this->task_struct.task_ctl.task_id; child_tasks.push_back(that); task_struct.num_child_procs = child_tasks.size(); task_struct.child_procs = reinterpret_cast<addr_t*>(child_tasks.data()); }
bool task_ctx::remove_child(task_ctx* that) { if(std::vector<task_ctx*>::const_iterator i = child_tasks.find(that); i != child_tasks.end()) { child_tasks.erase(i); return true; } return false; }
void sys_task_exit() { int retv; asm volatile("movl %%eax, %0" : "=r"(retv) :: "memory"); get_gs_base<task_ctx>()->set_exit(retv); }
void task_ctx::set_stdio_ptrs(std::array<file_node*, 3>&& ptrs) { array_copy(stdio_ptrs, ptrs.data(), 3UL); }
void task_ctx::start_task() { start_task(addr_t(std::addressof(handle_exit))); }
void task_ctx::restart_task() { restart_task(addr_t(std::addressof(handle_exit))); }
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
    if(arg_vec.empty() || arg_vec.back()) arg_vec.push_back(nullptr);
    set_arg_registers(arg_vec.size() - 1, reinterpret_cast<register_t>(arg_vec.data()), reinterpret_cast<register_t>(env_vec.data()));
    task_struct.saved_regs.rip = entry;
    fx_save(std::addressof(task_struct));
    __builtin_memset(task_struct.fxsv.xmm, 0, sizeof(task_struct.fxsv.xmm));
    for(int i = 0; i < 8; i++) { task_struct.fxsv.stmm[i] = 0.L; }
    if(is_user())
    {
        kmm.enter_frame(task_struct.frame_ptr);
        kmm.identity_map_to_user(this, sizeof(task_ctx), true, false);
        kmm.identity_map_to_user(arg_vec.data(), (arg_vec.size() + 1UL) * sizeof(char*), true, false);
        kmm.identity_map_to_user(env_vec.data(), (env_vec.size() + 1UL) * sizeof(char*), true, false);
        for(const char* str : env_vec) { if(str) kmm.identity_map_to_user(str, std::strlen(str), true, false); }
        for(const char* str : arg_vec) { if(str) kmm.identity_map_to_user(str, std::strlen(str), true, false); }
        kmm.exit_frame();
        interrupt_table::map_interrupt_callbacks(task_struct.frame_ptr);
    }
    else task_struct.saved_regs.rsp.ref<uintptr_t>() = addr_t(std::addressof(sys_task_exit));
}
void task_ctx::set_arg_registers(register_t rdi, register_t rsi, register_t rdx)
{
    task_struct.saved_regs.rdi = rdi;
    task_struct.saved_regs.rsi = rsi;
    task_struct.saved_regs.rdx = rdx;
}
task_ctx::task_ctx(task_functor task, std::vector<const char*>&& args, addr_t stack_base, ptrdiff_t stack_size, addr_t tls_base, size_t tls_len, addr_t frame_ptr, uint64_t pid, int64_t parent_pid, priority_val prio, uint16_t quantum) : 
    task_struct
    { 
        .self               { std::addressof(task_struct) }, 
        .frame_ptr          { frame_ptr }, 
        .saved_regs         
        { 
            .rbp            { stack_base + stack_size },
            .rsp            { stack_base + stack_size },
            .rflags         { 0x202UL },
            .ds             { data_segment(get_frame_magic(frame_ptr)) },
            .ss             { data_segment(get_frame_magic(frame_ptr)) },
            .cs             { code_segment(get_frame_magic(frame_ptr)) },
            .cr3            { __pml4_of(frame_ptr) },
        }, 
        .quantum_val        { quantum }, 
        .task_ctl           { { false, false, false, false, prio }, 0U, 0U, 0U, parent_pid, pid },
        .tls_block          { tls_base.plus(tls_len) }
    },
    arg_vec                 { std::move(args) },
    entry                   { task },
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
        .saved_regs         
        { 
            .rbp            { addr_t(desc.prg_stack).plus(desc.stack_size) },
            .rsp            { addr_t(desc.prg_stack).plus(desc.stack_size)},
            .rflags         { 0x202UL },
            .ds             { data_segment(get_frame_magic(desc.frame_ptr)) },
            .ss             { data_segment(get_frame_magic(desc.frame_ptr)) },
            .cs             { code_segment(get_frame_magic(desc.frame_ptr)) },
            .cr3            { __pml4_of(desc.frame_ptr) } 
        },
        .quantum_val        { quantum }, 
        .task_ctl           { { false, false, false, false, prio }, 0U, 0U, 0U, parent_pid, pid },
        .tls_block          { addr_t(desc.prg_tls).plus(desc.tls_size) }
    },
    arg_vec                 { std::move(args) },
    dl_search_paths         ( desc.ld_path_count ),
    entry                   { desc.entry },
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
    exit_target = exit_fn;
    task_struct.saved_regs.rip = entry;
    sch.register_task(task_struct.self);
    current_state = execution_state::RUNNING;
}
void task_ctx::restart_task(addr_t exit_fn)
{
    if(current_state != execution_state::TERMINATED)
    {
        exit_target = exit_fn;
        set_arg_registers(arg_vec.size(), reinterpret_cast<register_t>(arg_vec.data()), reinterpret_cast<register_t>(env_vec.data()));
        task_struct.saved_regs.rip = entry;
        task_struct.saved_regs.rbp = task_struct.saved_regs.rsp = allocated_stack.plus(stack_allocated_size);
        __builtin_memset(task_struct.fxsv.xmm, 0, sizeof(task_struct.fxsv.xmm));
        for(int i = 0; i < 8; i++) { task_struct.fxsv.stmm[i] = 0.L; }
        exit_code = 0;
        current_state = execution_state::RUNNING;
    }
}
void task_ctx::set_exit(int n) 
{
    exit_code = n;
    if(current_state == execution_state::RUNNING)
    {
        task_ctx* c = this, *p = nullptr;
        while(c->get_parent_pid() > 0 && tl.contains(static_cast<uint64_t>(c->get_parent_pid())))
        {
            p = std::addressof(*tl.find(static_cast<uint64_t>(c->get_parent_pid())));
            if(p->task_struct.task_ctl.notify_cterm && p->task_struct.task_ctl.block && sch.interrupt_wait(p->task_struct.self) && p->notif_target) p->notif_target.ref<int>() = n;
            p->last_notified = this;
            c = p;
        }
        // TODO: invoke the dynamic linker to call the destructors if applicable
    }
    if(exit_target) exit_target.ref<void()>()();
    else handle_exit();
    __builtin_unreachable();    
}
void task_ctx::attach_object(elf64_object* obj)
{
    std::vector<block_descr> blocks = obj->segment_blocks();
    kmm.enter_frame(task_struct.frame_ptr);
    kmm.map_to_current_frame(blocks);
    kmm.exit_frame();
    if(elf64_shared_object* so = dynamic_cast<elf64_shared_object*>(obj)) { attached_so_handles.push_back(so); }
}
void task_ctx::terminate()
{
    current_state = execution_state::TERMINATED;
    sch.unregister_task(task_struct.self);
    for(task_ctx* c : child_tasks) { if(c->current_state == execution_state::RUNNING) { if(exit_code) { c->exit_code = exit_code; c->terminate(); } } }
    if(is_user()) { frame_manager::get().destroy_frame(task_struct.frame_ptr.ref<uframe_tag>()); }
    if(local_so_map) { local_so_map->shared_frame = nullptr; }
}
tms task_ctx::get_times() const noexcept
{
    tms result{ task_struct.run_time, task_struct.sys_time, 0UL, 0UL };
    for(task_ctx* child : child_tasks) { result += child->get_times(); }
    return result;
}
void task_exec(elf64_program_descriptor const& prg, std::vector<const char*>&& args, std::vector<const char*>&& env, std::array<file_node*, 3>&& stdio_ptrs, addr_t exit_fn, int64_t parent_pid, priority_val pv, uint16_t quantum)
{
    task_ctx* ctx = tl.create_user_task(prg, std::move(args), parent_pid, pv, quantum);
    ctx->env_vec = std::move(env);
    ctx->init_task_state();
    ctx->set_stdio_ptrs(std::move(stdio_ptrs));
    if(exit_fn) { ctx->start_task(exit_fn); }
    else { ctx->start_task(); }
    user_entry(ctx->task_struct.self);
}