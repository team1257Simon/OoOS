#include <sched/scheduler.hpp>
#include <sched/task_list.hpp>
#include <algorithm>
#include <errno.h>
#include <elf64_dynamic_exec.hpp>
#include <frame_manager.hpp>
#include <isr_table.hpp>
#include <kdebug.hpp>
#include <kernel_mm.hpp>
#include <prog_manager.hpp>
typedef std::pair<elf64_sym, addr_t> sym_pair;
constexpr static uint64_t ignored_mask = bit_mask<18, 19, 20, 21, 25, 32, 33, 34, 35, 36, 37>::value;
constexpr static std::allocator<shared_object_map> sm_alloc{};
constexpr static addr_t pml4_of(addr_t frame) noexcept { return frame.deref<uframe_tag>().pml4; }
static bool is_tls_sym(elf64_sym const& s) { return s.st_info.type == ST_TLS; }
filesystem* task_ctx::get_vfs_ptr() { return ctx_filesystem; }
void task_ctx::set_stdio_ptrs(std::array<file_vnode*, 3>&& ptrs) { array_copy(stdio_ptrs, ptrs.data(), 3UL); }
void task_ctx::start_task() { start_task(addr_t(std::addressof(handle_exit))); }
void task_ctx::restart_task() { restart_task(addr_t(std::addressof(handle_exit))); }
void task_ctx::set_stdio_ptrs(file_vnode* ptrs[3]) { array_copy(stdio_ptrs, ptrs, 3UL); }
elf64_dynamic_object* task_ctx::assert_dynamic() { return std::addressof(dynamic_cast<elf64_dynamic_executable&>(*program_handle)); }
static inline void init_fx(fx_state& fx)
{
	asm volatile("fxsave %0" : "=m"(fx) :: "memory");
	array_zero(fx.xmm, sizeof(fx_state::xmm) / sizeof(int128_t));
	array_zero(fx.stmm, sizeof(fx_state::stmm) / sizeof(long double));
}
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
task_ctx::task_ctx(task_descriptor&& desc, pid_t pid) :
	task_struct
	{
		.self				{ std::addressof(task_struct) },
		.frame_ptr			{ desc.program.frame_ptr },
		.saved_regs
		{
			.rbp			{ addr_t(desc.program.prg_stack).plus(desc.program.stack_size) },
			.rsp			{ addr_t(desc.program.prg_stack).plus(desc.program.stack_size) },
			.cr3			{ pml4_of(desc.program.frame_ptr) },
			.rflags			{ ini_flags },
			.ds				{ user_data },
			.ss				{ user_data },
			.cs				{ user_code }
		},
		.quantum_val		{ desc.quantum },
		.task_ctl
		{
			{
				.block			{ false },
				.can_interrupt	{ false },
				.should_notify	{ false },
				.killed			{ false },
				.prio_base		{ desc.priority }
			},
			{
				.signal_info	{ std::addressof(task_sig_info) },
				.parent_pid		{ desc.parent_pid },
				.task_id		{ pid },
				.task_uid		{ desc.uid },
				.task_gid		{ desc.gid }
			},
		},
		.tls_master				{ desc.program.prg_tls },
		.tls_size				{ desc.program.tls_size },
		.tls_align				{ desc.program.tls_align }
	},
	arg_vec						{ std::move(desc.argv) },
	dl_search_paths				{ desc.program.ld_path_count },
	entry						{ desc.program.entry },
	allocated_stack				{ desc.program.prg_stack },
	stack_allocated_size		{ desc.program.stack_size },
	ctx_filesystem				{ create_task_vfs() },
	current_state				{ execution_state::STOPPED },
	program_handle				{ static_cast<elf64_executable*>(desc.program.object_handle) },
	local_so_map				{ sm_alloc.allocate(1) }
	{
		std::construct_at(local_so_map, static_cast<uframe_tag*>(desc.program.frame_ptr));
		if(desc.program.ld_path && desc.program.ld_path_count)
			dl_search_paths.push_back(desc.program.ld_path, desc.program.ld_path + desc.program.ld_path_count);
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
				.task_id		{ tl.__upid() },
				.task_uid		{ that.task_struct.task_ctl.task_uid },
				.task_gid		{ that.task_struct.task_ctl.task_gid }
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
	tls_modules				{ that.tls_modules },
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
	opened_directories		{ that.opened_directories },
	dyn_thread				{ that.dyn_thread }
							{}
task_ctx::task_ctx(task_ctx&& that) :
	task_struct				{ that.task_struct },
	child_tasks				{ std::move(that.child_tasks) },
	arg_vec					{ std::move(that.arg_vec) },
	env_vec					{ std::move(that.env_vec) },
	dl_search_paths			{ std::move(that.dl_search_paths) },
	attached_so_handles		{ std::move(that.attached_so_handles) },
	tls_modules				{ std::move(that.tls_modules) },
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
	opened_directories		{ std::move(that.opened_directories) },
	dyn_thread				{ std::move(that.dyn_thread) }
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
		attach_object(ldso.base(), true);
	}
	else
	{
		// Static executables go directly to the entry point
		rdi_val						= arg_vec.size() - 1;
		task_struct.saved_regs.rip	= entry;
	}
	init_fx(task_struct.fxsv);
	array_zero(reinterpret_cast<void**>(task_sig_info.signal_handlers), num_signals);
	uframe_tag& frame	= get_frame();
	addr_t old_ext		= frame.extent;
	size_t total_len	= (arg_vec.size() + env_vec.size() + 2UL) * sizeof(char*);
	for(const char* str : arg_vec) { if(str) total_len += std::strlen(str); }
	for(const char* str : arg_vec) { if(str) total_len += std::strlen(str); }
	if(!frame.shift_extent(static_cast<ptrdiff_t>(total_len))) throw std::bad_alloc{};
	rt_argv_ptr			= old_ext;
	char** argv_real	= frame.translate(rt_argv_ptr);
	rt_env_ptr			= old_ext.plus((arg_vec.size() + 1) * sizeof(char*));
	old_ext				= rt_env_ptr;
	char** env_real		= frame.translate(rt_env_ptr);
	old_ext				+= (env_vec.size() + 1) * sizeof(char*);
	for(const char* str : arg_vec)
	{
		addr_t target_ptr	= old_ext;
		char* target_real	= frame.translate(target_ptr);
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
		char* target_real	= frame.translate(target_ptr);
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
	kthread_ptr t(header(), task_struct.thread_ptr);
	sch.register_task(t);
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
		array_zero(task_struct.fxsv.stmm, sizeof(fx_state::stmm) / sizeof(long double));
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
			if(n != 0) xklog("[SCHED/EXEC] D: process " + std::to_string(get_pid()) + " exited with code " + std::to_string(n));
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
void task_ctx::attach_object(elf64_object* obj, bool is_init)
{
	std::vector<block_descriptor> blocks	= obj->segment_blocks();
	kmm.enter_frame(task_struct.frame_ptr);
	kmm.map_to_current_frame(blocks);
	kmm.exit_frame();
	elf64_shared_object* so					= dynamic_cast<elf64_shared_object*>(obj);
	if(so)
	{
		attached_so_handles.push_back(so);
		if(so->module_tls() && so->module_tls_size() && !is_init) {
			so->module_tls_index(tls_modules.size());
			tls_modules.push_back(so);
		}
	}
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
void task_ctx::stack_push(register_t val)
{
	task_struct.saved_regs.rsp	-= sizeof(register_t);
	addr_t stack				= get_frame().translate(task_struct.saved_regs.rsp);
	stack.assign(val);
}
register_t task_ctx::stack_pop()
{
	addr_t stack				= get_frame().translate(task_struct.saved_regs.rsp);
	register_t result			= stack.deref<register_t>();
	task_struct.saved_regs.rsp	+= sizeof(register_t);
	return result;
}
void task_ctx::set_signal(int sig, bool save_state)
{
	if((ignored_mask & BIT(sig)) && !(task_sig_info.signal_handlers[sig])) task_sig_info.pending_signals.btr(sig);
	else
	{
		if(save_state)
		{
			signal_interrupted_thread		= task_struct.thread_ptr;
			task_sig_info.sigret_frame		= task_struct.saved_regs;
			task_sig_info.sigret_fxsave		= task_struct.fxsv;
			thread_switch(0U);
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
void task_ctx::end_signal()
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
bool task_ctx::subsume(elf64_program_descriptor const& desc, cstrvec&& args, cstrvec&& env)
{
	spid_t parent_pid					= task_struct.task_ctl.parent_pid;
	if(local_so_map)
	{
		if(parent_pid < 0)
		{
			local_so_map->shared_frame	= nullptr;
			fm.destroy_frame(get_frame());
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
		.tls_master			{ desc.prg_tls },
		.tls_size			{ desc.tls_size },
		.tls_align			{ desc.tls_align }
	};
	arg_vec					= std::move(args);
	env_vec 				= std::move(env);
	opened_directories.clear();
	dyn_thread.base_offsets.clear();
	tls_modules.clear();
	init_task_state();
	return true;
}
void task_ctx::tls_assemble()
{
	if(!dyn_thread.base_offsets.empty()) throw std::invalid_argument("[EXEC/THREAD] attempting to assemble TLS more than once; misplaced syscall?");
	uframe_tag& frame				= get_frame();
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
	for(std::vector<elf64_dynamic_object*>::reverse_iterator i = modules_with_tls.rbegin(); i != modules_with_tls.rend(); i++, midx++) {
		(*i)->module_tls_index(midx);
		tls_modules.push_back(*i);
	}
	addr_t end_virtual		= frame.extent;
	for(tls_sub_block& b : sub_blocks)
		end_virtual			= end_virtual.alignup(b.align).plus(b.size);
	end_virtual				= end_virtual.alignup(alignof(thread_t));
	task_struct.tls_align	= sub_blocks.front().align;
	task_struct.tls_master	= frame.extent.alignup(task_struct.tls_align);
	task_struct.tls_size	= static_cast<size_t>(end_virtual - task_struct.tls_master);
	if(!frame.shift_extent(end_virtual - frame.extent)) throw std::bad_alloc();
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
	uframe_tag& frame			= get_frame();
	addr_t tls_block_start		= task_struct.tls_master ? frame.extent.alignup(task_struct.tls_align) : nullptr;
	addr_t tls_block_end		= (task_struct.tls_master ? tls_block_start.plus(task_struct.tls_size) : frame.extent).alignup(alignof(thread_t));
	if(!frame.shift_extent(tls_block_end.plus(sizeof(thread_t)) - frame.extent)) throw std::bad_alloc();
	if(task_struct.tls_master)
	{
		addr_t src					= frame.translate(task_struct.tls_master);
		addr_t dest					= frame.translate(tls_block_start);
		if(!src || !dest) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
		array_copy<uint8_t>(dest, src, task_struct.tls_size);
	}
	addr_t real					= frame.translate(tls_block_end);
	if(!real) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	thread_t* thread_0			= new(real) thread_t
	{
		.self					= task_struct.thread_ptr,
		.saved_regs				= task_struct.saved_regs,
		.ctl_info
		{
			{
				.state				= thread_state::STOPPED,
				.park				= false,
				.non_timed_park		= false,
				.detached			= false,
				.cancel_disable		= false,
				.cancel_async		= false
			},
			{
				.thread_lock		= spinlock_t(),
				.thread_id			= 0U,
				.wait_time_delta	= 0UZ
			}
		},
		.stack_base				= allocated_stack,
		.stack_size				= stack_allocated_size,
		.tls_start				= tls_block_start
	};
	init_fx(thread_0->fxsv);
	thread_ptr_by_id.insert(std::make_pair(0U, thread_0));
	dyn_thread.instantiate(*thread_0);
	next_assigned_thread_id 	= 1U;
	task_struct.thread_ptr		= tls_block_end;
	sch.retrothread(header(), task_struct.thread_ptr);
	thread_0->ctl_info.state	= thread_state::RUNNING;
}
void task_ctx::thread_switch(pid_t to_thread)
{
	if(!thread_ptr_by_id.contains(to_thread)) throw std::invalid_argument("[EXEC/THREAD] no such thread ID: " + std::to_string(to_thread));
	uframe_tag& frame			= get_frame();
	thread_t* next_thread		= thread_ptr_by_id[to_thread];
	thread_t* current_thread	= frame.translate(task_struct.thread_ptr);
	if(__unlikely(current_thread->ctl_info.thread_id == to_thread)) return;	// if the thread is already active there's nothing to do
	thread_t* next_thread_real	= frame.translate(next_thread);
	if(!current_thread || !next_thread_real) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	ooos::update_thread_state(*current_thread, task_struct);
	task_struct.saved_regs		= next_thread_real->saved_regs;
	task_struct.fxsv			= next_thread_real->fxsv;
	task_struct.thread_ptr		= next_thread;
}
addr_t task_ctx::tls_get(size_t mod_idx, size_t offs)
{
	if(__unlikely(!(mod_idx < tls_modules.size() && mod_idx))) throw std::invalid_argument("[EXEC/THREAD] module ID is out of range");
	uframe_tag& frame					= get_frame();
	thread_t* thread					= frame.translate(task_struct.thread_ptr);
	if(!thread) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	ooos::dynamic_thread_vector* dtvp	= thread->dtv_ptr;
	if(!dtvp) throw std::runtime_error("[EXEC/THREAD] cannot load tls address without a DTV");
	ooos::dynamic_thread_vector& dtv	= *dtvp;
	// This indicates a dynamic module that has not been used in this thread yet.
	// Amortized constant insertions on the vector will minimize resizing.
	while(!(mod_idx < dtv.size())) dtv.push_back(nullptr);
	if(!dtv[mod_idx])
	{
		ooos::lock_thread_mutex(*thread);
		elf64_dynamic_object* mod		= tls_modules[mod_idx];
		addr_t target					= frame.extent.alignup(mod->module_tls_align());
		addr_t end						= target.plus(mod->module_tls_size());
		if(!frame.shift_extent(end - frame.extent)) throw std::bad_alloc();
		addr_t src						= frame.translate(mod->module_tls());
		addr_t dest						= frame.translate(target);
		if(!src || !dest) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
		size_t n						= static_cast<size_t>(end - target);
		array_copy<uint8_t>(dest, src, n);
		dtv[mod_idx] 					= target;
		ooos::lock_thread_mutex(*thread);
	}
	return dtv[mod_idx].plus(offs);
}
pid_t task_ctx::thread_fork()
{
	uframe_tag& frame			= get_frame();
	thread_t* current_thread	= frame.translate(task_struct.thread_ptr);
	if(!current_thread) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	ooos::update_thread_state(*current_thread, task_struct);
	thread_t* new_thread		= thread_create(*current_thread, true);
	dyn_thread.instantiate(*new_thread);
	thread_ptr_by_id.insert(std::make_pair(new_thread->ctl_info.thread_id, new_thread->self));
	kthread_ptr kth(header(), new_thread->self);
	sch.register_task(kth);
	new_thread->ctl_info.state	= thread_state::RUNNING;
	return new_thread->ctl_info.thread_id;
}
pid_t task_ctx::thread_vfork(addr_t entry_point, addr_t arg, addr_t exit_point)
{
	uframe_tag& frame			= get_frame();
	thread_t* current_thread	= frame.translate(task_struct.thread_ptr);
	if(!current_thread) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	ooos::update_thread_state(*current_thread, task_struct);
	thread_t* new_thread		= thread_create(*current_thread, false);
	addr_t real_stack			= frame.translate(new_thread->saved_regs.rsp.minus(sizeof(register_t)));
	if(!real_stack) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	real_stack.assign(exit_point);
	new_thread->saved_regs.rsp	-= sizeof(register_t);
	new_thread->saved_regs.rbp	-= sizeof(register_t);
	new_thread->saved_regs.rip	= entry_point;
	new_thread->saved_regs.rdi	= arg.full;
	dyn_thread.instantiate(*new_thread);
	thread_ptr_by_id.insert(std::make_pair(new_thread->ctl_info.thread_id, new_thread->self));
	kthread_ptr kth(header(), new_thread->self);
	sch.register_task(kth);
	new_thread->ctl_info.state	= thread_state::RUNNING;
	return new_thread->ctl_info.thread_id;
}
void task_ctx::thread_exit(pid_t thread_id, register_t result_val)
{
	using iterator			= std::map<pid_t, thread_t*>::iterator;
	iterator i				= thread_ptr_by_id.find(thread_id);
	if(i == thread_ptr_by_id.end()) throw std::invalid_argument("[EXEC/THREAD] no such thread ID: " + std::to_string(thread_id));
	uframe_tag& frame		= get_frame();
	thread_t* thread		= i->second;
	kthread_ptr kth(header(), thread);
	thread_t* real			= frame.translate(thread);
	if(!real) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	ooos::update_thread_state(*real, task_struct);
	bool detached			= real->ctl_info.detached;
	real->ctl_info.state	= thread_state::TERMINATED;
	sch.unregister_task(kth);
	if(notify_threads.contains(thread_id))
	{
		std::vector<thread_t*>& notif	= notify_threads[thread_id];
		for(thread_t* tptr : notif)
		{
			kthread_ptr tkth(header(), tptr);
			thread_t* treal			= frame.translate(tptr);
			if(!treal) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
			// Pass the thread's return value to the waiting thread
			treal->saved_regs.rax	= result_val;
			sch.interrupt_wait(tkth);
		}
		detached					= true;
	}
	if(detached)
	{
		notify_threads.erase(thread_id);
		dyn_thread.takedown(*real);
		thread_ptr_by_id.erase(i);
		// The memory remains a part of the process; both the thread ID and the memory used for the TLS and thread struct can be reused later if needed
		inactive_threads.push_back(thread);
		next_assigned_thread_id		= thread_id;
	}
	else real->saved_regs.rax		= result_val;
}
thread_t* task_ctx::thread_create(thread_t const& template_thread, bool copy_all_regs)
{
	uframe_tag& frame			= get_frame();
	addr_t block_start;
	addr_t block_end;
	addr_t stack_begin;
	addr_t stack_end;
	// TODO: check/enforce thread limit
	if(inactive_threads.empty())
	{
		block_start				= task_struct.tls_master ? frame.extent.alignup(task_struct.tls_align) : nullptr;
		block_end				= (task_struct.tls_master ? block_start.plus(task_struct.tls_size) : frame.extent).alignup(alignof(thread_t));
		if(!frame.shift_extent(block_end.plus(sizeof(thread_t)) - frame.extent)) throw std::bad_alloc();
		stack_begin				= frame.extent.next_page_aligned();
		stack_end				= stack_begin.plus(template_thread.stack_size);
		if(!frame.shift_extent(stack_end - frame.extent)) throw std::bad_alloc();
	}
	else
	{
		thread_t* old			= frame.translate(inactive_threads.back());
		if(!old) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
		inactive_threads.pop_back();
		block_start				= old->tls_start;
		block_end				= old->self;
		stack_begin				= old->stack_base;
		stack_end				= stack_begin.plus(old->stack_size);
	}
	if(task_struct.tls_master)
	{
		addr_t src				= frame.translate(task_struct.tls_master);
		addr_t dest				= frame.translate(block_start);
		if(!src || !dest) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
		array_copy<uint8_t>(dest, src, task_struct.tls_size);
	}
	addr_t real_thread			= frame.translate(block_end);
	if(!real_thread) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	uint32_t id 				= next_assigned_thread_id;
	next_assigned_thread_id++;
	thread_t* new_thread		= new(real_thread) thread_t
	{
		.self					= block_end,
		.saved_regs				= copy_all_regs ? template_thread.saved_regs : regstate_t
		{
			.cr3				= template_thread.saved_regs.cr3,
			.rflags				= ini_flags,
			.ds					= template_thread.saved_regs.ds,
			.ss					= template_thread.saved_regs.ss,
			.cs					= template_thread.saved_regs.cs
		},
		.fxsv					= template_thread.fxsv,
		.ctl_info
		{
			{
				.state				= thread_state::STOPPED,
				.park				= false,
				.non_timed_park		= false,
				.detached			= false,
				.cancel_disable		= false,
				.cancel_async		= false
			},
			{
				.thread_lock		= spinlock_t(),
				.thread_id			= id,
				.wait_time_delta	= 0UZ
			}
		},
		.stack_base				= stack_begin,
		.stack_size				= static_cast<size_t>(stack_end - stack_begin),
		.tls_start				= block_start
	};
	new_thread->saved_regs.rsp	= stack_begin.plus(template_thread.saved_regs.rsp - template_thread.stack_base);
	new_thread->saved_regs.rbp	= stack_begin.plus(template_thread.saved_regs.rbp - template_thread.stack_base);
	addr_t real_stack			= frame.translate(new_thread->stack_base);
	addr_t real_old_stack		= frame.translate(template_thread.stack_base);
	if(!real_stack || !real_old_stack) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	array_copy<uint8_t>(real_stack, real_old_stack, template_thread.stack_size);
	return new_thread;
}
join_result task_ctx::thread_join(pid_t with_thread)
{
	using iterator				= std::map<pid_t, thread_t*>::iterator;
	iterator i 					= thread_ptr_by_id.find(with_thread);
	if(__unlikely(i == thread_ptr_by_id.end())) return join_result::NXTHREAD;
	uframe_tag& frame			= get_frame();
	thread_t* thread			= i->second;
	thread_t* real				= frame.translate(thread);
	thread_t* current			= task_struct.thread_ptr;
	if(!current) throw std::logic_error("[EXEC/THREAD] cannot join to a nonexistent thread");
	current						= frame.translate(current);
	if(!real || !current) throw std::out_of_range("[EXEC/THREAD] virtual address fault");
	if(real->ctl_info.state == thread_state::TERMINATED)
	{
		current->saved_regs.rax	= task_struct.saved_regs.rax	= real->saved_regs.rax;
		dyn_thread.takedown(*real);
		next_assigned_thread_id	= i->first;
		thread_ptr_by_id.erase(i);
		inactive_threads.push_back(thread);
		return join_result::IMMEDIATE;
	}
	notify_threads[with_thread].push_back(current->self);
	return join_result::DEFER;
}
int task_ctx::thread_detach(pid_t thread_id)
{
	if(__unlikely(!thread_ptr_by_id.contains(thread_id))) return -ESRCH;
	thread_t* ptr			= get_frame().translate(thread_ptr_by_id[thread_id]);
	if(__unlikely(!ptr)) return -EFAULT;
	if(__unlikely(ptr->ctl_info.state != thread_state::RUNNING)) return -EINVAL;
	ptr->ctl_info.detached	= true;
	return 0;
}