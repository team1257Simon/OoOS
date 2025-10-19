#include "sched/scheduler.hpp"
#include "sched/task_list.hpp"
#include "prog_manager.hpp"
#include "frame_manager.hpp"
#include "kernel_mm.hpp"
#include "kdebug.hpp"
#include "errno.h"
#include "fs/fat32.hpp"
#include "elf64_exec.hpp"
#include "isr_table.hpp"
constexpr static uint64_t ignored_mask = bit_mask<18, 19, 20, 21, 25, 32, 33, 34, 35, 36, 37>::value;
constexpr static std::allocator<shared_object_map> sm_alloc{};
constexpr static uint64_t get_frame_magic(addr_t tag) { return tag.ref<uint64_t>(); }
static inline addr_t pml4_of(addr_t frame_ptr) { if(get_frame_magic(frame_ptr) == uframe_magic) return frame_ptr.as<uframe_tag>()->pml4; else return get_cr3(); }
void sys_task_exit() { int retv; asm volatile("movl %%eax, %0" : "=r"(retv) :: "memory"); get_gs_base<task_ctx>()->set_exit(retv); }
filesystem* task_ctx::get_vfs_ptr() { return ctx_filesystem; }
void task_ctx::add_child(task_ctx* that) { child_tasks.push_back(that); task_struct.num_child_procs = child_tasks.size(); task_struct.child_procs = reinterpret_cast<addr_t*>(child_tasks.data()); }
bool task_ctx::remove_child(task_ctx* that) { if(std::vector<task_ctx*>::const_iterator i = child_tasks.find(that); i != child_tasks.end()) { child_tasks.erase(i); return true; } return false; }
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
        local_so_map->shared_frame = nullptr;
        sm_alloc.deallocate(local_so_map, 1);
    }
    if(is_user()) try { fm.destroy_frame(task_struct.frame_ptr.ref<uframe_tag>()); } catch(std::exception& e) { panic(e.what()); }
}
void task_ctx::init_task_state()
{
    if(arg_vec.empty() || arg_vec.back()) arg_vec.push_back(nullptr);
    register_t rdi_val;
    if(elf64_dynamic_object* dyn = dynamic_cast<elf64_dynamic_object*>(program_handle))
    {
        rdi_val                             = reinterpret_cast<register_t>(dyn);
        task_struct.saved_regs.rbx          = static_cast<register_t>(entry.full);
        shared_object_map::iterator ldso    = shared_object_map::get_ldso_object(ctx_filesystem);
        if(addr_t ldso_entry = ldso->entry_point(); __builtin_expect(static_cast<bool>(ldso_entry), true)) { task_struct.saved_regs.rip = ldso_entry; }
        else { throw std::runtime_error("[PRG/EXEC] dynamic linker object has no entry point"); }
        attach_object(ldso.base());
    }
    else
    {
        // Static executables go directly to the entry point
        rdi_val                     = arg_vec.size() - 1;
        task_struct.saved_regs.rip  = entry;
    }
    fx_save(std::addressof(task_struct));
    array_zero(task_struct.fxsv.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
    for(int i = 0; i < 8; i++) { task_struct.fxsv.stmm[i] = 0.L; }
    if(is_user())
    {
        array_zero(reinterpret_cast<void**>(task_sig_info.signal_handlers), num_signals);
        uframe_tag* tag     = task_struct.frame_ptr;
        addr_t old_ext      = tag->extent;
        size_t total_len    = (arg_vec.size() + env_vec.size() + 2UL) * sizeof(char*);
        for(const char* str : arg_vec) { if(str) total_len += std::strlen(str); }
        for(const char* str : arg_vec) { if(str) total_len += std::strlen(str); }
        if(!tag->shift_extent(static_cast<ptrdiff_t>(total_len))) throw std::bad_alloc{};
        rt_argv_ptr         = old_ext;
        char** argv_real    = tag->translate(rt_argv_ptr);
        rt_env_ptr          = old_ext.plus((arg_vec.size() + 1) * sizeof(char*));
        old_ext             = rt_env_ptr;
        char** env_real     = tag->translate(rt_env_ptr);
        old_ext             += (env_vec.size() + 1) * sizeof(char*);
        for(const char* str : arg_vec)
        {
            addr_t target_ptr   = old_ext;
            char* target_real   = tag->translate(target_ptr);
            size_t len          = std::strlen(str);
            array_copy(target_real, str, len);
            target_real[len]    = 0;
            *argv_real          = target_ptr;
            argv_real++;
            old_ext             += len + 1;
        }
        *argv_real = nullptr;
        for(const char* str : env_vec)
        {
            addr_t target_ptr   = old_ext;
            char* target_real   = tag->translate(target_ptr);
            size_t len          = std::strlen(str);
            array_copy(target_real, str, len);
            target_real[len]    = 0;
            *env_real           = target_ptr;
            env_real++;
            old_ext             += len + 1;
        }
        *env_real = nullptr;
    }
    else
    {
        task_struct.saved_regs.rsp.assign(std::addressof(sys_task_exit));
        rt_argv_ptr     = arg_vec.data();
        rt_env_ptr      = env_vec.data();
    }
    set_arg_registers(rdi_val, rt_argv_ptr.full, rt_env_ptr.full);
}
void task_ctx::set_arg_registers(register_t rdi, register_t rsi, register_t rdx)
{
    task_struct.saved_regs.rdi = rdi;
    task_struct.saved_regs.rsi = rsi;
    task_struct.saved_regs.rdx = rdx;
}
task_ctx::task_ctx(task_functor task, std::vector<const char*>&& args, addr_t stack_base, ptrdiff_t stack_size, addr_t tls_base, size_t tls_len, addr_t frame_ptr, pid_t pid, spid_t parent_pid, priority_val prio, uint16_t quantum) : 
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
            .cr3            { pml4_of(frame_ptr) },
        }, 
        .quantum_val        { quantum }, 
        .task_ctl           
        { 
            {
                .block          { false },
                .can_interrupt  { false },
                .should_notify  { false },
                .killed         { false },
                .prio_base      { prio }
            }, 
            {
                .signal_info    { std::addressof(task_sig_info) },
                .parent_pid     { parent_pid }, 
                .task_id        { pid }
            }
        },
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
task_ctx::task_ctx(elf64_program_descriptor const& desc, std::vector<const char *>&& args, pid_t pid, spid_t parent_pid, priority_val prio, uint16_t quantum) :
    task_struct
    {
        .self               { std::addressof(task_struct) },
        .frame_ptr          { desc.frame_ptr },
        .saved_regs         
        { 
            .rbp            { addr_t(desc.prg_stack).plus(desc.stack_size) },
            .rsp            { addr_t(desc.prg_stack).plus(desc.stack_size) },
            .rflags         { 0x202UL },
            .ds             { data_segment(get_frame_magic(desc.frame_ptr)) },
            .ss             { data_segment(get_frame_magic(desc.frame_ptr)) },
            .cs             { code_segment(get_frame_magic(desc.frame_ptr)) },
            .cr3            { pml4_of(desc.frame_ptr) } 
        },
        .quantum_val        { quantum }, 
        .task_ctl           
        {
            {
                .block          { false },
                .can_interrupt  { false },
                .should_notify  { false },
                .killed         { false },
                .prio_base      { prio }
            }, 
            {
                .signal_info    { std::addressof(task_sig_info) },
                .parent_pid     { parent_pid }, 
                .task_id        { pid }
            }
        },
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
    program_handle          { static_cast<elf64_executable*>(desc.object_handle) },
    local_so_map            { sm_alloc.allocate(1) }
                            { std::construct_at(local_so_map, static_cast<uframe_tag*>(desc.frame_ptr)); if(desc.ld_path && desc.ld_path_count) { dl_search_paths.push_back(desc.ld_path, desc.ld_path + desc.ld_path_count); } }
task_ctx::task_ctx(task_ctx const& that) :
    task_struct
    {
        .self               { this },
        .frame_ptr          { that.task_struct.frame_ptr },
        .saved_regs         { that.task_struct.saved_regs },
        .quantum_val        { that.task_struct.quantum_val },
        .quantum_rem        { that.task_struct.quantum_rem },
        .task_ctl
        {
            {
                .block          { false },
                .can_interrupt  { false },
                .should_notify  { false },
                .killed         { false },
                .prio_base      { that.task_struct.task_ctl.prio_base }
            },
            {
                .signal_info    { std::addressof(task_sig_info) },
                .parent_pid     { static_cast<spid_t>(that.get_pid()) },
                .task_id        { tl.__upid() }
            }
        },
        .fxsv               { that.task_struct.fxsv },
        .tls_block          { that.task_struct.tls_block }
    },
    child_tasks             {},
    arg_vec                 { that.arg_vec },
    env_vec                 { that.env_vec },
    dl_search_paths         { that.dl_search_paths },
    attached_so_handles     { that.attached_so_handles },
    entry                   { that.entry },
    allocated_stack         { that.allocated_stack },
    stack_allocated_size    { that.stack_allocated_size },
    tls                     { that.tls },
    tls_size                { that.tls_size },
    ctx_filesystem          { that.ctx_filesystem },
    stdio_ptrs              { that.stdio_ptrs[0], that.stdio_ptrs[1], that.stdio_ptrs[2] },
    current_state           { that.current_state },
    exit_code               { that.exit_code },
    exit_target             { that.exit_target },
    dynamic_exit            { that.dynamic_exit },
    notif_target            {},
    last_notified           {},
    program_handle          { that.program_handle },
    local_so_map            { that.local_so_map },
    rt_argv_ptr             { that.rt_argv_ptr },
    rt_env_ptr              { that.rt_env_ptr },
    task_sig_info           {},
    opened_directories      { that.opened_directories }
                            {}
task_ctx::task_ctx(task_ctx&& that) :
    task_struct             { that.task_struct },
    child_tasks             { std::move(that.child_tasks) },
    arg_vec                 { std::move(that.arg_vec) },
    env_vec                 { std::move(that.env_vec) },
    dl_search_paths         { std::move(that.dl_search_paths) },
    attached_so_handles     { std::move(that.attached_so_handles) },
    entry                   { std::move(that.entry) },
    allocated_stack         { std::move(that.allocated_stack) },
    stack_allocated_size    { std::move(that.stack_allocated_size) },
    tls                     { std::move(that.tls) },
    tls_size                { std::move(that.tls_size) },
    ctx_filesystem          { std::move(that.ctx_filesystem) },
    stdio_ptrs              { std::move(that.stdio_ptrs[0]), std::move(that.stdio_ptrs[1]), std::move(that.stdio_ptrs[2]) },
    current_state           { std::move(that.current_state) },
    exit_code               { std::move(that.exit_code) },
    exit_target             { std::move(that.exit_target) },
    dynamic_exit            { std::move(that.dynamic_exit) },
    notif_target            { std::move(that.notif_target) },
    last_notified           { std::move(that.last_notified) },
    program_handle          { std::move(that.program_handle) },
    local_so_map            { std::move(that.local_so_map) },
    rt_argv_ptr             { std::move(that.rt_argv_ptr) },
    rt_env_ptr              { std::move(that.rt_env_ptr) },
    task_sig_info           { std::move(that.task_sig_info) },
    opened_directories      { std::move(that.opened_directories) }
    { 
        array_zero(reinterpret_cast<uint64_t*>(std::addressof(that)), (sizeof(task_ctx) / sizeof(uint64_t))); 
        task_struct.self                    = this;
        task_struct.task_ctl.signal_info    = std::addressof(task_sig_info);
        that.task_struct.frame_ptr          = nullptr;
    }
void task_ctx::start_task(addr_t exit_fn)
{
    exit_target     = exit_fn ? exit_fn : addr_t(std::addressof(handle_exit));
    sch.register_task(header());
    current_state   = execution_state::RUNNING;
}
void task_ctx::restart_task(addr_t exit_fn)
{
    if(current_state != execution_state::TERMINATED)
    {
        exit_target                 = exit_fn;
        set_arg_registers(arg_vec.size(), reinterpret_cast<register_t>(arg_vec.data()), reinterpret_cast<register_t>(env_vec.data()));
        task_struct.saved_regs.rip  = entry;
        task_struct.saved_regs.rbp  = task_struct.saved_regs.rsp = allocated_stack.plus(stack_allocated_size);
        array_zero(task_struct.fxsv.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
        for(int i = 0; i < 8; i++) { task_struct.fxsv.stmm[i] = 0.L; }
        exit_code       = 0;
        current_state   = execution_state::RUNNING;
    }
}
void task_ctx::set_exit(int n)
{
    exit_code = n;
    if(current_state == execution_state::RUNNING)
    {
        task_ctx* c = this;
        task_ctx* p = nullptr;
        while(c->get_parent_pid() > 0 && tl.contains(static_cast<uint64_t>(c->get_parent_pid())))
        {
            p                               = tl.find(static_cast<uint64_t>(c->get_parent_pid())).base();
            if(p->task_struct.task_ctl.should_notify && p->task_struct.task_ctl.block && sch.interrupt_wait(p->task_struct.self) && p->notif_target)
                p->notif_target.assign(n);
            p->last_notified                = this;
            p->remove_child(this);
            p->task_struct.saved_regs.rax   = get_pid();
            c                               = p;
        }
        if(elf64_dynamic_object* dyn = dynamic_cast<elf64_dynamic_object*>(program_handle); dyn && dynamic_exit)
        {
            if(n != 0) { xklog("[SCHED/EXEC] D: process " + std::to_string(get_pid()) + " exited with code " + std::to_string(n)); }
            else
            {
                task_struct.saved_regs.rdi  = reinterpret_cast<register_t>(dyn);
                task_struct.saved_regs.rip  = dynamic_exit;
                current_state               = execution_state::IN_DYN_EXIT;
                user_reentry();
                __builtin_unreachable();
            }
        }
    }
    if(exit_target) exit_target.invoke<void()>();
    else handle_exit();
    __builtin_unreachable();    
}
void task_ctx::attach_object(elf64_object* obj)
{
    if(!is_user()) return;
    std::vector<block_descriptor> blocks = obj->segment_blocks();
    kmm.enter_frame(task_struct.frame_ptr);
    kmm.map_to_current_frame(blocks);
    kmm.exit_frame();
    if(elf64_shared_object* so = dynamic_cast<elf64_shared_object*>(obj)) { attached_so_handles.push_back(so); }
}
void task_ctx::terminate()
{
    current_state = execution_state::TERMINATED;
    sch.unregister_task(std::addressof(task_struct));
    for(task_ctx* c : child_tasks) { if(c->current_state == execution_state::RUNNING) { if(exit_code) { c->exit_code = exit_code; c->terminate(); } } }
}
tms task_ctx::get_times() const noexcept
{
    tms result(task_struct.run_time, task_struct.sys_time, 0UL, 0UL);
    for(task_ctx* child : child_tasks) { result += child->get_times(); }
    return result;
}
void task_exec(elf64_program_descriptor const& prg, std::vector<const char*>&& args, std::vector<const char*>&& env, std::array<file_node*, 3>&& stdio_ptrs, addr_t exit_fn, int64_t parent_pid, priority_val pv, uint16_t quantum)
{
    task_ctx* ctx   = tl.create_user_task(prg, std::move(args), parent_pid, pv, quantum);
    ctx->env_vec    = std::move(env);
    ctx->init_task_state();
    ctx->set_stdio_ptrs(std::move(stdio_ptrs));
    if(exit_fn) { ctx->start_task(exit_fn); }
    else { ctx->start_task(); }
    user_entry(ctx->task_struct.self);
}
void task_ctx::stack_push(register_t val)
{
    task_struct.saved_regs.rsp  -= sizeof(register_t);
    addr_t stack                = is_user() ? task_struct.frame_ptr.ref<uframe_tag>().translate(task_struct.saved_regs.rsp) : task_struct.saved_regs.rsp;
    stack.assign(val);
}
register_t task_ctx::stack_pop()
{
    addr_t stack                = is_user() ? task_struct.frame_ptr.ref<uframe_tag>().translate(task_struct.saved_regs.rsp) : task_struct.saved_regs.rsp;
    register_t result           = stack.ref<register_t>();
    task_struct.saved_regs.rsp  += sizeof(register_t);
    return result;
}
void task_ctx::set_signal(int sig, bool save_state)
{
    if((ignored_mask & BIT(sig)) && !(task_sig_info.signal_handlers[sig])) { task_sig_info.pending_signals.btr(sig); return; }
    if(save_state) { task_sig_info.sigret_frame = task_struct.saved_regs; task_sig_info.sigret_fxsave = task_struct.fxsv; }
    task_sig_info.active_signal         = sig;
    task_struct.saved_regs.rdi          = sig;
    signal_handler handler              = task_sig_info.signal_handlers[sig];
    if(!handler) handler                = signal_exit;
    task_struct.saved_regs.rsi          = reinterpret_cast<register_t>(handler);
    task_sig_info.signal_handlers[sig]  = nullptr;
    task_struct.saved_regs.rip          = addr_t(sigtramp_enter);
    stack_push(static_cast<register_t>(task_sig_info.sigret_frame.rip.full));
}
register_t task_ctx::end_signal()
{
    addr_t end_rsp              = task_struct.saved_regs.rsp;
    addr_t end_rip              = task_struct.saved_regs.rip;
    task_struct.saved_regs      = task_sig_info.sigret_frame;
    task_struct.saved_regs.rsp  = end_rsp;
    task_struct.saved_regs.rip  = end_rip;
    task_struct.fxsv            = task_sig_info.sigret_fxsave;
    task_sig_info.active_signal = 0;
    stack_push(task_sig_info.sigret_frame.r11);
    stack_push(task_sig_info.sigret_frame.rcx);
    return task_sig_info.sigret_frame.rax;
}
bool task_ctx::set_fork()
{
    try 
    {
        uframe_tag* old_frame           = task_struct.frame_ptr;
        shared_object_map* old_so_map   = local_so_map;
        uframe_tag* new_frame           = std::addressof(fm.fork_frame(old_frame));
        task_struct.frame_ptr           = new_frame;
        task_struct.saved_regs.cr3      = new_frame->pml4;
        if(old_so_map) local_so_map     = sm_alloc.allocate(1);
        if(local_so_map)
        { 
            std::construct_at(local_so_map, new_frame);
            for(elf64_shared_object* so : attached_so_handles) 
            {
                kmm.enter_frame(new_frame);
                kmm.map_to_current_frame(so->segment_blocks());
                kmm.exit_frame();
            } 
            local_so_map->copy(*old_so_map);
        }
        program_handle = prog_manager::get_instance().clone(program_handle);
        if(!program_handle) return false;
        program_handle->on_copy(new_frame);
        if(opened_directories.empty()) return true;
        std::map<int, posix_directory> old_dirs(std::move(opened_directories));
        opened_directories.clear();
        for(std::map<int, posix_directory>::iterator i = old_dirs.begin(); i != old_dirs.end(); i++) { opened_directories.emplace(std::piecewise_construct, std::forward_as_tuple(i->first), std::forward_as_tuple(new_frame, i->second.get_base_vaddr())); }
        return true;
    }
    catch(std::exception& e) { panic(e.what()); return false; }
}
bool task_ctx::subsume(elf64_program_descriptor const& desc, std::vector<const char*>&& args, std::vector<const char*>&& env)
{
    spid_t parent_pid = task_struct.task_ctl.parent_pid;
    if(local_so_map)
    {
        if(parent_pid < 0)
        {
            local_so_map->shared_frame = nullptr;
            fm.destroy_frame(task_struct.frame_ptr.ref<uframe_tag>());
            sm_alloc.deallocate(local_so_map, 1);
        }
        local_so_map = sm_alloc.allocate(1UZ);
        if(!local_so_map) return false;
    }
    uframe_tag* new_tag     = static_cast<uframe_tag*>(desc.frame_ptr);
    if(!new_tag) throw std::invalid_argument("[COMPAT/execve] frame must not be null");
    task_struct.frame_ptr   = new_tag;
    allocated_stack         = desc.prg_stack;
    stack_allocated_size    = desc.stack_size;
    tls                     = desc.prg_tls;
    tls_size                = desc.tls_size;
    pid_t pid               = get_pid();
    priority_val prio       = task_struct.task_ctl.prio_base;
    uint16_t quantum        = task_struct.quantum_val;
    new(std::addressof(task_struct)) task_t
    {
        .self               { std::addressof(task_struct) },
        .frame_ptr          { desc.frame_ptr },
        .saved_regs         
        { 
            .rbp            { addr_t(desc.prg_stack).plus(desc.stack_size) },
            .rsp            { addr_t(desc.prg_stack).plus(desc.stack_size) },
            .rflags         { 0x202UL },
            .ds             { data_segment(get_frame_magic(desc.frame_ptr)) },
            .ss             { data_segment(get_frame_magic(desc.frame_ptr)) },
            .cs             { code_segment(get_frame_magic(desc.frame_ptr)) },
            .cr3            { pml4_of(desc.frame_ptr) } 
        },
        .quantum_val        { quantum }, 
        .task_ctl           
        {
            {
                .block          { false },
                .can_interrupt  { false },
                .should_notify  { false },
                .killed         { false },
                .prio_base      { prio }
            }, 
            {
                .signal_info    { std::addressof(task_sig_info) },
                .parent_pid     { parent_pid }, 
                .task_id        { pid }
            }
        },
        .tls_block          { addr_t(desc.prg_tls).plus(desc.tls_size) }
    };
    arg_vec = std::move(args);
    env_vec = std::move(env);
    opened_directories.clear();
    init_task_state();
    return true;
}