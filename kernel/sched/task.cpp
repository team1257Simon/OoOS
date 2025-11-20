#include "sched/scheduler.hpp"
#include "sched/task_list.hpp"
#include "prog_manager.hpp"
#include "frame_manager.hpp"
#include "kernel_mm.hpp"
#include "errno.h"
#include "fs/fat32.hpp"
#include "elf64_dynamic_exec.hpp"
#include "isr_table.hpp"
typedef std::pair<elf64_sym, addr_t> sym_pair;
constexpr static uint64_t ignored_mask = bit_mask<18, 19, 20, 21, 25, 32, 33, 34, 35, 36, 37>::value;
constexpr static std::allocator<shared_object_map> sm_alloc{};
constexpr static addr_t pml4_of(addr_t frame) noexcept { return frame.deref<uframe_tag>().pml4; }
static bool is_tls_sym(elf64_sym const& s) { return s.st_info.type == SYM_TLS; }
filesystem* task_ctx::get_vfs_ptr() { return ctx_filesystem; }
void task_ctx::set_stdio_ptrs(std::array<file_vnode*, 3>&& ptrs) { array_copy(stdio_ptrs, ptrs.data(), 3UL); }
void task_ctx::start_task() { start_task(addr_t(std::addressof(handle_exit))); }
void task_ctx::restart_task() { restart_task(addr_t(std::addressof(handle_exit))); }
void task_ctx::set_stdio_ptrs(file_vnode* ptrs[3]) { array_copy(stdio_ptrs, ptrs, 3UL); }
elf64_dynamic_object* task_ctx::assert_dynamic() { elf64_dynamic_executable& dyn = dynamic_cast<elf64_dynamic_executable&>(*program_handle); return std::addressof(dyn); }
static sym_pair tls_search(elf64_dynamic_object* obj, task_ctx* task, const char* name)
{
	elf64_shared_object* so	= dynamic_cast<elf64_shared_object*>(obj);
	if(so && so->is_symbolic())
	{
		sym_pair result_pair	= so->resolve_by_name(name);
		if(result_pair.second && is_tls_sym(result_pair.first))
			return result_pair;
	}
	if(elf64_dynamic_object* dyn = dynamic_cast<elf64_dynamic_object*>(task->program_handle))
	{
		sym_pair result_pair	= dyn->resolve_by_name(name);
		if(result_pair.second && is_tls_sym(result_pair.first))
			return result_pair;
	}
	for(elf64_shared_object& so : *task->local_so_map)
	{
		sym_pair result_pair	= so.resolve_by_name(name);
		if(result_pair.second && is_tls_sym(result_pair.first))
			return result_pair;
	}
	return sym_pair();
}
task_ctx::task_ctx(elf64_program_descriptor const& desc, std::vector<const char *>&& args, pid_t pid, spid_t parent_pid, priority_val prio, uint16_t quantum) :
	task_struct
	{
		.self				{ std::addressof(task_struct) },
		.frame_ptr			{ desc.frame_ptr },
		.saved_regs		
		{ 
			.rbp			{ addr_t(desc.prg_stack).plus(desc.stack_size) },
			.rsp			{ addr_t(desc.prg_stack).plus(desc.stack_size) },
			.cr3			{ pml4_of(desc.frame_ptr) },
			.rflags			{ ini_flags },
			.ds				{ user_data },
			.ss				{ user_data },
			.cs				{ user_code }
		},
		.quantum_val		{ quantum }, 
		.task_ctl			
		{
			{
				.block			{ false },
				.can_interrupt	{ false },
				.should_notify	{ false },
				.killed			{ false },
				.prio_base		{ prio }
			}, 
			{
				.signal_info	{ std::addressof(task_sig_info) },
				.parent_pid		{ parent_pid }, 
				.task_id		{ pid }
			}
		},
		.tls_master			{ addr_t(desc.prg_tls) },
		.tls_size			{ desc.tls_size },
		.tls_align			{ desc.tls_align }
	},
	arg_vec					{ std::move(args) },
	dl_search_paths			( desc.ld_path_count ),
	entry					{ desc.entry },
	allocated_stack			{ desc.prg_stack },
	stack_allocated_size	{ desc.stack_size },
	ctx_filesystem			{ create_task_vfs() },
	current_state			{ execution_state::STOPPED },
	program_handle			{ static_cast<elf64_executable*>(desc.object_handle) },
	local_so_map			{ sm_alloc.allocate(1) }
							{
								std::construct_at(local_so_map, static_cast<uframe_tag*>(desc.frame_ptr));
								if(desc.ld_path && desc.ld_path_count)
									dl_search_paths.push_back(desc.ld_path, desc.ld_path + desc.ld_path_count);
							}
task_ctx::task_ctx(task_ctx const& that) :
	task_struct
	{
		.self				{ std::addressof(task_struct) },
		.frame_ptr			{ that.task_struct.frame_ptr },
		.saved_regs			{ that.task_struct.saved_regs },
		.quantum_val		{ that.task_struct.quantum_val },
		.quantum_rem		{ that.task_struct.quantum_rem },
		.task_ctl
		{
			{
				.block			{ false },
				.can_interrupt	{ false },
				.should_notify	{ false },
				.killed			{ false },
				.prio_base		{ that.task_struct.task_ctl.prio_base }
			},
			{
				.signal_info	{ std::addressof(task_sig_info) },
				.parent_pid		{ static_cast<spid_t>(that.get_pid()) },
				.task_id		{ tl.__upid() }
			}
		},
		.fxsv				{ that.task_struct.fxsv },
		.tls_master			{ that.task_struct.tls_master },
		.tls_size			{ that.task_struct.tls_size },
		.tls_align			{ that.task_struct.tls_align }
	},
	child_tasks				{},
	arg_vec					{ that.arg_vec },
	env_vec					{ that.env_vec },
	dl_search_paths			{ that.dl_search_paths },
	attached_so_handles		{ that.attached_so_handles },
	entry					{ that.entry },
	allocated_stack			{ that.allocated_stack },
	stack_allocated_size	{ that.stack_allocated_size },
	ctx_filesystem			{ that.ctx_filesystem },
	stdio_ptrs				{ that.stdio_ptrs[0], that.stdio_ptrs[1], that.stdio_ptrs[2] },
	current_state			{ that.current_state },
	exit_code				{ that.exit_code },
	exit_target				{ that.exit_target },
	dynamic_exit			{ that.dynamic_exit },
	notif_target			{ nullptr },
	last_notified			{ nullptr },
	program_handle			{ that.program_handle },
	local_so_map			{ that.local_so_map },
	rt_argv_ptr				{ that.rt_argv_ptr },
	rt_env_ptr				{ that.rt_env_ptr },
	task_sig_info			{},
	opened_directories		{ that.opened_directories }
							{}
task_ctx::task_ctx(task_ctx&& that) :
	task_struct				{ that.task_struct },
	child_tasks				{ std::move(that.child_tasks) },
	arg_vec					{ std::move(that.arg_vec) },
	env_vec					{ std::move(that.env_vec) },
	dl_search_paths			{ std::move(that.dl_search_paths) },
	attached_so_handles		{ std::move(that.attached_so_handles) },
	entry					{ std::move(that.entry) },
	allocated_stack			{ std::move(that.allocated_stack) },
	stack_allocated_size	{ std::move(that.stack_allocated_size) },
	ctx_filesystem			{ std::move(that.ctx_filesystem) },
	stdio_ptrs				{ std::move(that.stdio_ptrs[0]), std::move(that.stdio_ptrs[1]), std::move(that.stdio_ptrs[2]) },
	current_state			{ std::move(that.current_state) },
	exit_code				{ std::move(that.exit_code) },
	exit_target				{ std::move(that.exit_target) },
	dynamic_exit			{ std::move(that.dynamic_exit) },
	notif_target			{ std::move(that.notif_target) },
	last_notified			{ std::move(that.last_notified) },
	program_handle			{ std::move(that.program_handle) },
	local_so_map			{ std::move(that.local_so_map) },
	rt_argv_ptr				{ std::move(that.rt_argv_ptr) },
	rt_env_ptr				{ std::move(that.rt_env_ptr) },
	task_sig_info			{ std::move(that.task_sig_info) },
	opened_directories		{ std::move(that.opened_directories) }
	{ 
		array_zero(reinterpret_cast<uint64_t*>(std::addressof(that)), (sizeof(task_ctx) / sizeof(uint64_t))); 
		task_struct.self					= std::addressof(task_struct);
		task_struct.task_ctl.signal_info	= std::addressof(task_sig_info);
		that.task_struct.frame_ptr			= nullptr;
	}
task_ctx::~task_ctx()
{
	if(local_so_map)
	{
		shared_object_map& globals 			= shared_object_map::get_globals();
		std::vector<shared_object_map::iterator> stickies{};
		for(shared_object_map::iterator i	= local_so_map->begin(); i != local_so_map->end(); i++)
			if(i->is_sticky())
				stickies.push_back(i);
		for(shared_object_map::iterator i : stickies) { local_so_map->transfer(globals, i); }
		local_so_map->shared_frame 			= nullptr;
		sm_alloc.deallocate(local_so_map, 1);
	}
	if(!task_struct.frame_ptr) return; 
	try { fm.destroy_frame(task_struct.frame_ptr.deref<uframe_tag>()); }
	catch(std::exception& e) { panic(e.what()); }
}
void task_ctx::add_child(task_ctx* that)
{
	child_tasks.push_back(that);
	task_struct.num_child_procs	= child_tasks.size();
	task_struct.child_procs		= reinterpret_cast<task_t**>(child_tasks.data());
}
bool task_ctx::remove_child(task_ctx* that)
{
	std::vector<task_ctx*>::const_iterator i	= child_tasks.find(that);
	if(i == child_tasks.end()) return false;
	child_tasks.erase(i);
	return true;
}
void task_ctx::init_task_state()
{
	if(arg_vec.empty() || arg_vec.back()) arg_vec.push_back(nullptr);
	register_t rdi_val;
	if(elf64_dynamic_object* dyn			= dynamic_cast<elf64_dynamic_object*>(program_handle))
	{
		rdi_val								= reinterpret_cast<register_t>(dyn);
		task_struct.saved_regs.rbx			= static_cast<register_t>(entry.full);
		shared_object_map::iterator ldso	= shared_object_map::get_ldso_object(ctx_filesystem);
		addr_t ldso_entry 					= ldso->entry_point(); 
		if(ldso_entry)
			task_struct.saved_regs.rip		= ldso_entry;
		else throw std::runtime_error("[PRG/EXEC] dynamic linker object has no entry point");
		attach_object(ldso.base());
	}
	else
	{
		// Static executables go directly to the entry point
		rdi_val						= arg_vec.size() - 1;
		task_struct.saved_regs.rip	= entry;
	}
	fx_save(std::addressof(task_struct));
	array_zero(task_struct.fxsv.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
	for(int i = 0; i < 8; i++) task_struct.fxsv.stmm[i] = 0.L;
	array_zero(reinterpret_cast<void**>(task_sig_info.signal_handlers), num_signals);
	uframe_tag* tag		= task_struct.frame_ptr;
	addr_t old_ext		= tag->extent;
	size_t total_len	= (arg_vec.size() + env_vec.size() + 2UL) * sizeof(char*);
	for(const char* str : arg_vec) { if(str) total_len += std::strlen(str); }
	for(const char* str : arg_vec) { if(str) total_len += std::strlen(str); }
	if(!tag->shift_extent(static_cast<ptrdiff_t>(total_len))) throw std::bad_alloc{};
	rt_argv_ptr			= old_ext;
	char** argv_real	= tag->translate(rt_argv_ptr);
	rt_env_ptr			= old_ext.plus((arg_vec.size() + 1) * sizeof(char*));
	old_ext				= rt_env_ptr;
	char** env_real		= tag->translate(rt_env_ptr);
	old_ext				+= (env_vec.size() + 1) * sizeof(char*);
	for(const char* str : arg_vec)
	{
		addr_t target_ptr	= old_ext;
		char* target_real	= tag->translate(target_ptr);
		size_t len			= std::strlen(str);
		array_copy(target_real, str, len);
		target_real[len]	= 0;
		*argv_real			= target_ptr;
		argv_real++;
		old_ext				+= len + 1;
	}
	*argv_real				= nullptr;
	for(const char* str : env_vec)
	{
		addr_t target_ptr	= old_ext;
		char* target_real	= tag->translate(target_ptr);
		size_t len			= std::strlen(str);
		array_copy(target_real, str, len);
		target_real[len]	= 0;
		*env_real			= target_ptr;
		env_real++;
		old_ext				+= len + 1;
	}
	*env_real				= nullptr;
	set_arg_registers(rdi_val, rt_argv_ptr.full, rt_env_ptr.full);
}
void task_ctx::set_arg_registers(register_t rdi, register_t rsi, register_t rdx)
{
	task_struct.saved_regs.rdi	= rdi;
	task_struct.saved_regs.rsi	= rsi;
	task_struct.saved_regs.rdx	= rdx;
}
void task_ctx::start_task(addr_t exit_fn)
{
	exit_target		= exit_fn ? exit_fn : addr_t(std::addressof(handle_exit));
	sch.register_task(header());
	current_state	= execution_state::RUNNING;
}
void task_ctx::restart_task(addr_t exit_fn)
{
	if(current_state != execution_state::TERMINATED)
	{
		exit_target					= exit_fn;
		set_arg_registers(arg_vec.size(), reinterpret_cast<register_t>(arg_vec.data()), reinterpret_cast<register_t>(env_vec.data()));
		task_struct.saved_regs.rip	= entry;
		task_struct.saved_regs.rbp	= task_struct.saved_regs.rsp = allocated_stack.plus(stack_allocated_size);
		array_zero(task_struct.fxsv.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
		for(int i = 0; i < 8; i++) { task_struct.fxsv.stmm[i] = 0.L; }
		exit_code					= 0;
		current_state				= execution_state::RUNNING;
	}
}
void task_ctx::set_exit(int n)
{
	exit_code		= n;
	if(current_state == execution_state::RUNNING)
	{
		task_ctx* c	= this;
		task_ctx* p	= nullptr;
		while(c->get_parent_pid() > 0 && tl.contains(static_cast<uint64_t>(c->get_parent_pid())))
		{
			p								= tl.find(static_cast<uint64_t>(c->get_parent_pid())).base();
			if(p->task_struct.task_ctl.should_notify && p->task_struct.task_ctl.block && sch.interrupt_wait(p->task_struct.self) && p->notif_target)
				p->notif_target.assign(n);
			p->last_notified				= this;
			p->remove_child(this);
			p->task_struct.saved_regs.rax	= get_pid();
			c								= p;
		}
		if(elf64_dynamic_object* dyn = dynamic_cast<elf64_dynamic_object*>(program_handle); dyn && dynamic_exit)
		{
			if(n != 0) { xklog("[SCHED/EXEC] D: process " + std::to_string(get_pid()) + " exited with code " + std::to_string(n)); }
			else
			{
				task_struct.saved_regs.rdi	= reinterpret_cast<register_t>(dyn);
				task_struct.saved_regs.rip	= dynamic_exit;
				current_state				= execution_state::IN_DYN_EXIT;
				return;
			}
		}
	}
	if(exit_target) exit_target.invoke<void()>();
	else handle_exit();
	__builtin_unreachable();	
}
void task_ctx::attach_object(elf64_object* obj)
{
	std::vector<block_descriptor> blocks	= obj->segment_blocks();
	kmm.enter_frame(task_struct.frame_ptr);
	kmm.map_to_current_frame(blocks);
	kmm.exit_frame();
	if(elf64_shared_object* so				= dynamic_cast<elf64_shared_object*>(obj)) { attached_so_handles.push_back(so); }
}
void task_ctx::terminate()
{
	current_state	= execution_state::TERMINATED;
	sch.unregister_task(std::addressof(task_struct));
	for(task_ctx* c : child_tasks) { if(c->current_state == execution_state::RUNNING) { if(exit_code) { c->exit_code = exit_code; c->terminate(); } } }
}
tms task_ctx::get_times() const noexcept
{
	tms result(task_struct.run_time, task_struct.sys_time, 0UL, 0UL);
	for(task_ctx* child : child_tasks) result += child->get_times();
	return result;
}
void task_exec(elf64_program_descriptor const& prg, std::vector<const char*>&& args, std::vector<const char*>&& env, std::array<file_vnode*, 3>&& stdio_ptrs, addr_t exit_fn, int64_t parent_pid, priority_val pv, uint16_t quantum)
{
	task_ctx* ctx	= tl.create_user_task(prg, std::move(args), parent_pid, pv, quantum);
	ctx->env_vec	= std::move(env);
	ctx->init_task_state();
	ctx->set_stdio_ptrs(std::move(stdio_ptrs));
	if(exit_fn) ctx->start_task(exit_fn);
	else ctx->start_task();
	user_entry(std::addressof(ctx->task_struct));
}
void task_ctx::stack_push(register_t val)
{
	task_struct.saved_regs.rsp	-= sizeof(register_t);
	addr_t stack				= task_struct.frame_ptr.deref<uframe_tag>().translate(task_struct.saved_regs.rsp);
	stack.assign(val);
}
register_t task_ctx::stack_pop()
{
	addr_t stack				= task_struct.frame_ptr.deref<uframe_tag>().translate(task_struct.saved_regs.rsp);
	register_t result			= stack.deref<register_t>();
	task_struct.saved_regs.rsp	+= sizeof(register_t);
	return result;
}
void task_ctx::set_signal(int sig, bool save_state)
{
	if((ignored_mask & BIT(sig)) && !(task_sig_info.signal_handlers[sig])) task_sig_info.pending_signals.btr(sig);
	else
	{
		if(save_state) {
			task_sig_info.sigret_frame	= task_struct.saved_regs;
			task_sig_info.sigret_fxsave	= task_struct.fxsv;
		}
		task_sig_info.active_signal			= sig;
		task_struct.saved_regs.rdi			= sig;
		signal_handler handler				= task_sig_info.signal_handlers[sig];
		if(!handler) handler				= signal_exit;
		task_struct.saved_regs.rsi			= reinterpret_cast<register_t>(handler);
		task_sig_info.signal_handlers[sig]	= nullptr;
		task_struct.saved_regs.rip			= addr_t(sigtramp_enter);
		stack_push(static_cast<register_t>(task_sig_info.sigret_frame.rip.full));
	}
}
register_t task_ctx::end_signal()
{
	addr_t end_rsp				= task_struct.saved_regs.rsp;
	addr_t end_rip				= task_struct.saved_regs.rip;
	task_struct.saved_regs		= task_sig_info.sigret_frame;
	task_struct.saved_regs.rsp	= end_rsp;
	task_struct.saved_regs.rip	= end_rip;
	task_struct.fxsv			= task_sig_info.sigret_fxsave;
	task_sig_info.active_signal	= 0;
	stack_push(task_sig_info.sigret_frame.r11);
	stack_push(task_sig_info.sigret_frame.rcx);
	return task_sig_info.sigret_frame.rax;
}
bool task_ctx::set_fork()
{
	try 
	{
		uframe_tag* old_frame			= task_struct.frame_ptr;
		shared_object_map* old_so_map	= local_so_map;
		uframe_tag* new_frame			= std::addressof(fm.fork_frame(old_frame));
		task_struct.frame_ptr			= new_frame;
		task_struct.saved_regs.cr3		= new_frame->pml4;
		if(old_so_map) local_so_map		= sm_alloc.allocate(1UZ);
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
		program_handle					= prog_manager::get_instance().clone(program_handle);
		if(__unlikely(!program_handle)) return false;
		program_handle->on_copy(new_frame);
		if(opened_directories.empty()) return true;
		std::map<int, posix_directory> old_dirs(std::move(opened_directories));
		opened_directories.clear();
		for(std::map<int, posix_directory>::iterator i = old_dirs.begin(); i != old_dirs.end(); i++)
			opened_directories.emplace(std::piecewise_construct, std::forward_as_tuple(i->first), std::forward_as_tuple(new_frame, i->second.get_base_vaddr()));
		return true;
	}
	catch(std::exception& e) { panic(e.what()); }
	return false;
}
bool task_ctx::subsume(elf64_program_descriptor const& desc, std::vector<const char*>&& args, std::vector<const char*>&& env)
{
	spid_t parent_pid					= task_struct.task_ctl.parent_pid;
	if(local_so_map)
	{
		if(parent_pid < 0)
		{
			local_so_map->shared_frame	= nullptr;
			fm.destroy_frame(task_struct.frame_ptr.deref<uframe_tag>());
			sm_alloc.deallocate(local_so_map, 1UZ);
		}
		local_so_map		= sm_alloc.allocate(1UZ);
		if(__unlikely(!local_so_map)) return false;
	}
	uframe_tag* new_tag		= static_cast<uframe_tag*>(desc.frame_ptr);
	if(!new_tag) throw std::invalid_argument("[COMPAT/execve] frame must not be null");
	task_struct.frame_ptr	= new_tag;
	allocated_stack			= desc.prg_stack;
	stack_allocated_size	= desc.stack_size;
	pid_t pid				= get_pid();
	priority_val prio		= task_struct.task_ctl.prio_base;
	uint16_t quantum		= task_struct.quantum_val;
	new(std::addressof(task_struct)) task_t
	{
		.self				{ std::addressof(task_struct) },
		.frame_ptr			{ desc.frame_ptr },
		.saved_regs		
		{
			.rbp			{ addr_t(desc.prg_stack).plus(desc.stack_size) },
			.rsp			{ addr_t(desc.prg_stack).plus(desc.stack_size) },
			.cr3			{ pml4_of(desc.frame_ptr) },
			.rflags			{ ini_flags },
			.ds				{ user_data },
			.ss				{ user_data },
			.cs				{ user_code }
		},
		.quantum_val		{ quantum }, 
		.task_ctl			
		{
			{
				.block			{ false },
				.can_interrupt	{ false },
				.should_notify	{ false },
				.killed			{ false },
				.prio_base		{ prio }
			}, 
			{
				.signal_info	{ std::addressof(task_sig_info) },
				.parent_pid		{ parent_pid }, 
				.task_id		{ pid }
			}
		},
		.tls_master			{ addr_t(desc.prg_tls) },
		.tls_size			{ desc.tls_size },
		.tls_align			{ desc.tls_align }
	};
	arg_vec					= std::move(args);
	env_vec 				= std::move(env);
	opened_directories.clear();
	init_task_state();
	return true;
}
void task_ctx::tls_assemble()
{
	if(!dyn_thread.base_offsets.empty()) throw std::invalid_argument("[EXEC/THREAD] attempting to assemble TLS more than once; misplaced syscall?");
	uframe_tag& frame				= task_struct.frame_ptr.deref<uframe_tag>();
	elf64_dynamic_object* dyn_prg	= assert_dynamic();
	struct tls_sub_block
	{
		size_t size;
		size_t align;
		addr_t real_addr;
	};
	std::vector<tls_sub_block> sub_blocks;
	std::vector<elf64_dynamic_object*> modules_with_tls;
	for(elf64_shared_object* so : attached_so_handles)
	{
		if(addr_t tls_addr	= so->module_tls())
		{
			addr_t real		= frame.translate(tls_addr);
			if(!real) throw std::out_of_range("[EXEC/THREAD] TLS virtual address fault in shared object: " + so->get_soname());
			sub_blocks.emplace_back(so->module_tls_size(), so->module_tls_align(), real);
			modules_with_tls.push_back(so);
		}
	}
	size_t base_size		= task_struct.tls_size;
	if(base_size)
	{
		size_t base_align	= task_struct.tls_align;
		addr_t base_real	= frame.translate(task_struct.tls_master);
		if(!base_real) throw std::out_of_range("[EXEC/THREAD] TLS base virtual address fault");
		sub_blocks.emplace_back(base_size, base_align, base_real);
		modules_with_tls.push_back(dyn_prg);
	}
	if(__unlikely(sub_blocks.empty())) return;	// no TLS -> nothing to do
	size_t midx 			= 1UZ;
	for(std::vector<elf64_dynamic_object*>::reverse_iterator i = modules_with_tls.rbegin(); i != modules_with_tls.rend(); i++, midx++)
		(*i)->module_tls_index(midx);
	addr_t end_virtual		= frame.extent;
	for(tls_sub_block& b : sub_blocks)
		end_virtual			= end_virtual.alignup(b.align).plus(b.size);
	end_virtual				= end_virtual.alignup(alignof(thread_t));
	task_struct.tls_size	= static_cast<size_t>(end_virtual - frame.extent);
	if(!frame.shift_extent(static_cast<ptrdiff_t>(task_struct.tls_size))) throw std::bad_alloc();
	task_struct.tls_align	= sub_blocks.front().align;
	dyn_thread.base_offsets.push_back(0Z);
	addr_t sub_block_start	= end_virtual;
	struct failure_guard
	{
		ooos::task_dtv* dtv;
		void release() noexcept { dtv = nullptr; }
		~failure_guard() { if(dtv) dtv->base_offsets.clear(); }
	} g(std::addressof(dyn_thread));
	for(std::vector<tls_sub_block>::reverse_iterator i = sub_blocks.rbegin(); i != sub_blocks.rend(); i++)
	{
		sub_block_start		= sub_block_start.minus(i->size).trunc(i->align);
		dyn_thread.base_offsets.push_back(end_virtual - sub_block_start);
		addr_t start_real	= frame.translate(sub_block_start);
		if(!start_real) throw std::out_of_range("[EXEC/THREAD] TLS virtual address fault");
		array_copy<uint8_t>(start_real, i->real_addr, i->size);
	}
	task_struct.tls_master	= sub_block_start;
	task_struct.thread_ptr	= end_virtual;
	for(elf64_rela const& r : dyn_prg->get_tls_relas())
	{
		addr_t target					= frame.translate(dyn_prg->resolve_rela_target(r));
		if(!target) throw std::out_of_range("[EXEC/THREAD] TLS relocation target virtual address fault");
		sym_pair result					= tls_search(dyn_prg, this, dyn_prg->symbol_name(dyn_prg->get_sym(r.r_info.sym_index)));
		if(!result.second) throw std::runtime_error("[EXEC/THREAD] TLS relocation has invalid or absent symbol");
		elf64_dynamic_object* sym_owner	= result.second;
		target.assign(sym_owner->resolve_tls_rela(result.first, r, dyn_thread.base_offsets));
	}
	for(elf64_shared_object* so : attached_so_handles)
	{
		for(elf64_rela const& r : so->get_tls_relas())
		{
			addr_t target					= frame.translate(so->resolve_rela_target(r));
			if(!target) throw std::out_of_range("[EXEC/THREAD] TLS relocation target virtual address fault");
			sym_pair result					= tls_search(so, this, so->symbol_name(so->get_sym(r.r_info.sym_index)));
			if(!result.second) throw std::runtime_error("[EXEC/THREAD] TLS relocation has invalid or absent symbol");
			elf64_dynamic_object* sym_owner	= result.second;
			target.assign(sym_owner->resolve_tls_rela(result.first, r, dyn_thread.base_offsets));
		}
	}
	g.release();
}
void task_ctx::init_thread_0()
{
	uframe_tag& frame		= task_struct.frame_ptr.deref<uframe_tag>();
	if(!frame.shift_extent(sizeof(thread_t))) throw std::bad_alloc();
	thread_t* thread_0		= new(task_struct.thread_ptr) thread_t
	{
		.self					= task_struct.thread_ptr,
		.saved_regs				= task_struct.saved_regs,
		.fxsv					= task_struct.fxsv,
		.ctl_info
		{
			.state				= execution_state::STOPPED,
			.park				= false,
			.non_timed_park		= false,
			.thread_lock		= spinlock_t(),
			.thread_id			= 0U,
			.wait_time_delta	= 0UZ
		},
		.stack_base				= allocated_stack,
		.stack_size				= stack_allocated_size
	};
	thread_ptr_by_id.insert(std::make_pair(0U, thread_0));
	dyn_thread.instantiate(*thread_0);
}