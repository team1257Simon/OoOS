#include "shared_object_map.hpp"
#include "elf64_dynamic_exec.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "sys/errno.h"
#include "arch/arch_amd64.h"
typedef std::pair<addr_t, bool> search_result;
typedef std::pair<elf64_sym, addr_t> sym_pair;
static addr_t sysres_add(size_t len) { return current_active_task()->frame_ptr.deref<uframe_tag>().sysres_add(len); }
static shared_object_map::iterator global_object_search(std::string const& name, int flags) { return (flags & RTLD_GLOBAL) ? shared_object_map::get_globals().find(name) : shared_object_map::get_globals().end(); }
static search_result global_search(const char* name)
{
	addr_t result	= nullptr;
	bool have_weak	= false;
	for(elf64_shared_object const& so : shared_object_map::get_globals())
	{
		sym_pair result_pair	= so.resolve_by_name(name);
		if(!result_pair.second) continue;
		if(result_pair.first.st_info.bind == SB_GLOBAL) return search_result(result_pair.second, true);
		else if(result_pair.first.st_info.bind == SB_WEAK) {
			result		= result_pair.second;
			have_weak	= true;
		}
	}
	return search_result(result, have_weak);
}
static search_result full_search(task_ctx* task, const char* name)
{
	addr_t result;
	if(elf64_dynamic_object* dyn = dynamic_cast<elf64_dynamic_object*>(task->program_handle))
	{
		sym_pair result_pair	= dyn->resolve_by_name(name);
		if(result_pair.first.st_info.bind == SB_WEAK)
			result				= result_pair.second;
		else if(result_pair.second) return search_result(result_pair.second, true);
	}
	for(elf64_shared_object& so : *task->local_so_map)
	{
		sym_pair result_pair	= so.resolve_by_name(name);
		if(!result_pair.second) continue;
		else if(result_pair.first.st_info.bind == SB_WEAK)
			result				= result_pair.second;
		else return search_result(result_pair.second, true);
	}
	if(result) return search_result(result, true);
	return global_search(name);
}
static search_result full_search(elf64_dynamic_object* obj, task_ctx* task, const char* name)
{
	elf64_shared_object* so	= dynamic_cast<elf64_shared_object*>(obj);
	bool have_weak			= false;
	if(so && so->is_symbolic())
	{
		sym_pair result_pair = so->resolve_by_name(name);
		if(result_pair.second) return search_result(result_pair.second, true);
		else if(result_pair.first.st_info.bind == SB_WEAK) have_weak = true;
	}
	search_result res						= full_search(task, name);
	// TODO: check symbol versioning
	if(have_weak && !res.second) res.second	= true;
	return res;
}
static elf64_dynamic_object* validate_handle(addr_t handle)
{
	// We need to do this so the dynamic cast doesn't get optimized out; we can use it to verify that the object is indeed a handle and not some random pointer
	volatile elf64_object* o				= static_cast<volatile elf64_object*>(handle.as<volatile elf64_dynamic_object>());
	barrier();
	return const_cast<elf64_dynamic_object*>(dynamic_cast<elf64_dynamic_object volatile*>(o));
}
extern "C"
{
	addr_t syscall_dlinit(addr_t handle, addr_t resolve)
	{
		elf64_dynamic_object* obj_handle	= validate_handle(handle);
		if(!obj_handle) return addr_t(static_cast<uintptr_t>(-EBADF));
		obj_handle->set_resolver(resolve);
		task_ctx* task						= active_task_context();
		if(!task->local_so_map) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		// References to objects in the GOT must be resolved now
		for(elf64_rela const& r : obj_handle->get_object_relas())
		{
			addr_t target			= translate_user_pointer(obj_handle->resolve_rela_target(r));
			if(!target) return addr_t(static_cast<uintptr_t>(-ELIBBAD));
			elf64_sym const& sym	= obj_handle->get_sym(r.r_info.sym_index);
			search_result sr		= full_search(obj_handle, task, obj_handle->symbol_name(sym));
			if(!sr.first && !sr.second && sym.st_info.bind != SB_WEAK) return addr_t(static_cast<uintptr_t>(-ELIBACC));
			target.assign(sr.first);
		}
		size_t len		= obj_handle->get_init().size() + 1;
		addr_t result	= sysres_add(len * sizeof(addr_t));
		if(!result) return addr_t(static_cast<uintptr_t>(-ENOMEM));
		addr_t res_real = translate_user_pointer(result);
		if(len > 1) array_copy(res_real, obj_handle->get_init().data(), len - 1);
		res_real.plus((len - 1) * sizeof(addr_t)).assign(nullptr);
		return result;
	}
	addr_t syscall_dlfini(addr_t handle)
	{
		elf64_dynamic_object* obj_handle	= validate_handle(handle);
		if(__unlikely(!obj_handle)) return addr_t(static_cast<uintptr_t>(-EBADF));
		size_t len							= obj_handle->get_fini().size() + 1;
		addr_t result						= sysres_add(len * sizeof(addr_t));
		if(__unlikely(!result)) return addr_t(static_cast<uintptr_t>(-ENOMEM));
		addr_t res_real						= translate_user_pointer(result);
		if(len > 1) array_copy(res_real, obj_handle->get_init().data(), len - 1);
		res_real.plus((len - 1) * sizeof(addr_t)).assign(nullptr);
		return result;
	}
	addr_t syscall_dlpreinit(addr_t handle, addr_t endfn)
	{
		elf64_dynamic_object* obj_handle		= validate_handle(handle);
		if(__unlikely(!obj_handle)) return addr_t(static_cast<uintptr_t>(-EBADF));
		task_ctx* task							= active_task_context();
		if(__unlikely(!task)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		if(elf64_dynamic_executable* x			= dynamic_cast<elf64_dynamic_executable*>(obj_handle))
		{
			task->dynamic_exit					= endfn;
			size_t len							= x->get_preinit().size() + 1;
			addr_t result						= sysres_add(len * sizeof(addr_t));
			if(__unlikely(!result)) return addr_t(static_cast<uintptr_t>(-ENOMEM));
			addr_t res_real					 	= translate_user_pointer(result);
			if(len > 1) array_copy(res_real, obj_handle->get_init().data(), len - 1);
			res_real.plus((len - 1) * sizeof(addr_t)).assign(nullptr);
			return result;
		}
		else
		{
			addr_t result						= sysres_add(sizeof(addr_t));
			if(!result) return addr_t(static_cast<uintptr_t>(-ENOMEM));
			translate_user_pointer(result).assign(nullptr);
			return result;
		}
	}
	addr_t syscall_dlopen(const char* name, int flags)
	{
		task_ctx* task		= active_task_context();
		filesystem* fs_ptr	= get_task_vfs();
		if(__unlikely(!fs_ptr || !task || !task->local_so_map)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		if(!name) return task->program_handle; // dlopen(nullptr, ...) gives a "self" handle which resolves to a global lookup when used with dlsym
		name				= translate_user_pointer(name);
		if(__unlikely(!name)) return addr_t(static_cast<uintptr_t>(-EFAULT));
		std::string xname(name, std::strnlen(name, 256UL));
		shared_object_map::iterator result;
		shared_object_map::iterator cached	= global_object_search(xname, flags);
		if(cached	!= shared_object_map::get_globals().end())
			result	= cached;
		else
		{
			std::vector<std::string> paths(task->dl_search_paths.cbegin(), task->dl_search_paths.cend());
			paths.push_back("lib");
			paths.push_back("usr/lib");
			file_vnode* n					= nullptr;
			std::string const* found_path	= nullptr;
			for(std::string const& path : paths)
			{
				if(directory_vnode* d		= fs_ptr->get_directory_or_null(path, false))
				{
					if(tnode* tn			= d->find(xname); tn && tn->is_file())
					{
						n					= fs_ptr->on_open(tn);
						found_path			= std::addressof(path);
						break;
					}
				}
			}
			if(!n) return addr_t(static_cast<uintptr_t>(-ELIBACC));
			struct __guard
			{
				file_vnode* __my_file;
				filesystem* __fs_ptr;
				__guard(file_vnode* n, filesystem* fs) : __my_file(n), __fs_ptr(fs) {}
				~__guard() { if(__my_file) __fs_ptr->close_file(__my_file); }
			} g(n, fs_ptr);
			if(flags & RTLD_NOLOAD)
			{
				shared_object_map::iterator so		= task->local_so_map->get_if_resident(n);
				if(so == task->local_so_map->end()) return nullptr;
				if(flags & RTLD_GLOBAL) { result	= task->local_so_map->transfer(shared_object_map::get_globals(), so); }
				else result							= so;
			}
			else
			{
				shared_object_map& sm	= flags & RTLD_GLOBAL ? shared_object_map::get_globals() : *task->local_so_map;
				result					= sm.add(n);
				if(__unlikely(!result)) return addr_t(static_cast<uintptr_t>(-ENOMEM));
				sm.set_path(result, *found_path);
			}
			fs_ptr->close_file(n);
			g.__my_file					= nullptr;
		}
		if(flags & RTLD_NODELETE) result->set_sticky();
		elf64_dynamic_object* handle	= result.base();
		if(result->is_global()) task->attach_object(handle, (flags & RTLD_PREINIT) != 0U);
		return handle;
	}
	int syscall_dlclose(addr_t handle)
	{
		task_ctx* task			= active_task_context();
		if(__unlikely(!task || !task->local_so_map)) return -ENOSYS;
		if(static_cast<elf64_object*>(handle.as<elf64_dynamic_object>()) == task->program_handle) return -EBADF; // dlclose on the "self" handle does nothing (UB)
		elf64_shared_object* so	= dynamic_cast<elf64_shared_object*>(handle.as<elf64_dynamic_object>());
		if(__unlikely(!so)) return -EINVAL;
		shared_object_map::iterator it(addr_t(so).minus(shared_object_map::node_offset));
		if(!task->local_so_map) return -ENOSYS;
		if(task->local_so_map->contains(so->get_soname())) task->local_so_map->remove(it);
		else shared_object_map::get_globals().remove(it);
		return 0;
	}
	addr_t syscall_dlsym(addr_t handle, const char* name)
	{
		task_ctx* task				= active_task_context();
		if(__unlikely(!task || !task->local_so_map)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		name						= translate_user_pointer(name);
		if(__unlikely(!name)) return addr_t(static_cast<uintptr_t>(-EFAULT));
		if(!handle)
		{
			search_result sr		= global_search(name);
			if(!sr.second) return addr_t(static_cast<uintptr_t>(-ENOENT));
			return sr.first;
		}
		elf64_dynamic_object* dyn	= validate_handle(handle);
		if(!dyn) return addr_t(static_cast<uintptr_t>(-EINVAL));
		if(static_cast<elf64_object*>(dyn) == task->program_handle)
		{
			search_result sr		= full_search(task, name);
			if(!sr.second) return addr_t(static_cast<uintptr_t>(-ENOENT));
			return sr.first;
		}
		elf64_shared_object* so		= dynamic_cast<elf64_shared_object*>(dyn);
		if(__unlikely(!so)) return addr_t(static_cast<uintptr_t>(-EBADF));
		sym_pair result_pair		= so->resolve_by_name(name);
		if(!result_pair.second && result_pair.first.st_info.bind == SB_WEAK) return nullptr;
		else if(__unlikely(!result_pair.second)) return addr_t(static_cast<uintptr_t>(-ENOENT));
		return result_pair.second;
	}
	addr_t syscall_resolve(uint32_t sym_idx, addr_t got_loaded_id)
	{
		task_ctx* task					= active_task_context();
		if(__unlikely(!task || !task->local_so_map)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		if(elf64_dynamic_object* obj	= validate_handle(got_loaded_id))
		{
			if(__unlikely(!obj->has_plt_relas())) { return addr_t(static_cast<uintptr_t>(-ENOEXEC)); }
			elf64_rela const& rela		= obj->get_plt_rela(sym_idx);
			if(rela.r_info.type != R_X86_64_JUMP_SLOT) return addr_t(static_cast<uintptr_t>(-ELIBSCN));
			addr_t target_pos			= translate_user_pointer(obj->resolve_rela_target(rela));
			elf64_sym const& sym		= obj->get_sym(rela.r_info.sym_index);
			search_result result		= full_search(obj, task, obj->symbol_name(sym));
			if((!result.second && !result.first) && sym.st_info.bind != SB_WEAK) return addr_t(static_cast<uintptr_t>(-ELIBACC));
			return target_pos.assign(result.first);
		}
		else return addr_t(static_cast<uintptr_t>(-EINVAL));
	}
	int syscall_dlpath(const char* path_str)
	{
		task_ctx* task	= active_task_context();
		if(__unlikely(!task || !task->local_so_map)) return -ENOSYS;
		path_str		= translate_user_pointer(path_str);
		if(__unlikely(!path_str)) return -EFAULT;
		try
		{
			size_t end_pos					= std::distance(path_str, static_cast<const char*>(std::find(path_str, std::strlen(path_str), ';')));
			std::vector<std::string> paths	= std::ext::split(std::string(path_str, end_pos), ':');
			task->dl_search_paths.push_back(paths.begin(), paths.end());
		}
		catch(...) { return -ENOMEM; }
		return 0;
	}
	int syscall_dlmap(elf64_dynamic_object* obj, elf64_dlmap_entry* ent)
	{
		task_ctx* task			= active_task_context();
		if(__unlikely(!task || !task->local_so_map)) return -ENOSYS;
		elf64_shared_object* so	= dynamic_cast<elf64_shared_object*>(obj);
		if(__unlikely(!so)) return -EBADF;
		ent = translate_user_pointer(ent);
		if(__unlikely(!ent)) return -EFAULT;
		ent->dynamic_section		= so->dyn_segment_ptr();
		ent->dynamic_section_length	= so->dyn_segment_len();
		shared_object_map::iterator it(addr_t(so).minus(shared_object_map::node_offset));
		const char* fp			= task->local_so_map->contains(so->get_soname()) ? task->local_so_map->get_path(it) : shared_object_map::get_globals().get_path(it);
		if(!fp) fp				= so->get_soname().data();
		size_t len				= std::strlen(fp) + 1;
		ent->absolute_pathname	= sysres_add(len);
		if(__unlikely(!ent->absolute_pathname)) return -ENOMEM;
		char* target_real = translate_user_pointer(ent->absolute_pathname);
		array_copy(target_real, fp, len - 1);
		target_real[len]				= '\0';
		ent->vaddr_offset				= so->get_load_offset();
		ent->object_handle				= obj;
		ent->global_offset_table_start	= obj->global_offset_table();
		return 0;
	}
	addr_t syscall_depends(addr_t handle)
	{
		elf64_dynamic_object* obj_handle		= validate_handle(handle);
		if(!obj_handle) return addr_t(static_cast<uintptr_t>(-EINVAL));
		std::vector<std::string> const& deps	= obj_handle->get_dependencies();
		size_t ndep								= deps.size();
		char** result							= sysres_add((ndep + 1) * sizeof(char*));
		char** result_real						= translate_user_pointer(result);
		if(__unlikely(!result || !result_real)) return addr_t(static_cast<uintptr_t>(-ENOMEM));
		size_t n;
		for(n = 0; n < ndep; n++)
		{
			std::string const& str	= deps[n];
			size_t len				= str.size() + 1;
			char* target			= sysres_add(len);
			if(!target) return addr_t(static_cast<uintptr_t>(-ENOMEM));
			char* target_real		= translate_user_pointer(target);
			array_copy(target_real, str.data(), len - 1);
			target_real[len - 1]	= '\0';
			result_real[n]			= target;
		}
		result_real[ndep]			= nullptr;
		return result;
	}
	int syscall_dladdr(addr_t sym_addr, dl_addr_info* info)
	{
		task_ctx* task	= active_task_context();
		if(__unlikely(!task || !task->local_so_map)) return -ENOSYS;
		info			= translate_user_pointer(info);
		if(__unlikely(!info)) return -EFAULT;
		array_zero(reinterpret_cast<uint64_t*>(info), sizeof(dl_addr_info) / sizeof(uint64_t));
		size_t n		= task->attached_so_handles.size();
		for(size_t i	= 0; i < n; i++)
		{
			elf64_shared_object* so		= task->attached_so_handles[i];
			if(!so->could_contain(sym_addr)) continue;
			info->so_vbase				= so->get_load_offset();
			std::string const& oname	= so->get_soname();
			size_t len					= oname.size() + 1;
			info->so_name				= sysres_add(len);
			if(__unlikely(!info->so_name)) return -ENOMEM;
			array_copy(translate_user_pointer(info->so_name), oname.data(), len);
			const char* sname			= so->sym_lookup(sym_addr);
			if(sname)
			{
				info->actual_addr	= so->resolve_by_name(sname).second;
				len				 	= std::strlen(sname) + 1;
				info->symbol_name	= sysres_add(len);
				if(!info->symbol_name) return -ENOMEM;
				array_copy(translate_user_pointer(info->symbol_name), sname, len);
			}
			return i;
		}
		return 0;
	}
	int syscall_tlinit()
	{
		try
		{
			task_ctx* task	= active_task_context();
			if(__unlikely(!task)) return -ENOSYS;
			task->tls_assemble();
			task->init_thread_0();
		}
		catch(std::bad_alloc&)			{ return -ENOMEM; }
		catch(std::bad_cast&)			{ return -EINVAL; }
		catch(std::invalid_argument& e)	{ panic(e.what()); return -EINVAL; }
		catch(std::out_of_range& e)		{ panic(e.what()); return -EFAULT; }
		catch(std::runtime_error& e)	{ panic(e.what()); return -ENOENT; }
		return 0;
	}
	addr_t syscall_tlget(tls_index* idx)
	{
		task_ctx* task	= active_task_context();
		if(__unlikely(!task)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		idx				= translate_user_pointer(idx);
		if(__unlikely(!idx)) return addr_t(static_cast<uintptr_t>(-EFAULT));
		try { return task->tls_get(idx->ti_module, idx->ti_offset); }
		catch(std::invalid_argument& e) { panic(e.what()); return addr_t(static_cast<uintptr_t>(-EINVAL)); }
		catch(std::out_of_range& e)		{ panic(e.what()); return addr_t(static_cast<uintptr_t>(-EFAULT)); }
		catch(std::runtime_error& e)	{ panic(e.what()); return addr_t(static_cast<uintptr_t>(-ENOSYS)); }
		catch(std::bad_alloc&)			{ return addr_t(static_cast<uintptr_t>(-ENOMEM)); }
	}
}