#include "shared_object_map.hpp"
#include "elf64_dynamic_exec.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "kdebug.hpp"
#include "sys/errno.h"
#include "arch/arch_amd64.h"
static addr_t sysres_add(size_t len) { return current_active_task()->frame_ptr.ref<uframe_tag>().sysres_add(len); }
static addr_t global_search(const char* name)
{
    addr_t result = nullptr;
    for(elf64_shared_object const& so : shared_object_map::get_globals())
    {
        std::pair<elf64_sym, addr_t> result_pair = so.resolve_by_name(name);
        if(!result_pair.second) continue;
        if(result_pair.first.st_info.bind == SB_GLOBAL) return result_pair.second;
        else if(result_pair.first.st_info.bind == SB_WEAK) result = result_pair.second;
    }
    return result;
}
static addr_t full_search(task_ctx* task, const char* name)
{ 
    addr_t result; 
    if(elf64_dynamic_object* dyn = dynamic_cast<elf64_dynamic_object*>(task->object_handle))
    {   
        if(std::pair<elf64_sym, addr_t> result_pair = dyn->resolve_by_name(name); result_pair.second)
        {
            if(result_pair.first.st_info.bind == SB_GLOBAL) return result_pair.second;
            else if(result_pair.first.st_info.bind == SB_WEAK) result = result_pair.second;
            else result = nullptr;
        }
    }
    for(elf64_shared_object& so : *task->local_so_map)
    {
        std::pair<elf64_sym, addr_t> result_pair = so.resolve_by_name(name);
        if(!result_pair.second) continue;
        if(result_pair.first.st_info.bind == SB_GLOBAL) return result_pair.second;
        else if(result_pair.first.st_info.bind == SB_WEAK) result = result_pair.second;
    }
    for(elf64_shared_object const& so : shared_object_map::get_globals())
    {
        std::pair<elf64_sym, addr_t> result_pair = so.resolve_by_name(name);
        if(!result_pair.second) continue;
        if(result_pair.first.st_info.bind == SB_GLOBAL) return result_pair.second;
        else if(result_pair.first.st_info.bind == SB_WEAK) result = result_pair.second;
    }
    return result;
}
extern "C"
{
    addr_t syscall_dlinit(elf64_dynamic_object* obj_handle)
    {
        size_t len = obj_handle->get_init().size();
        addr_t result = sysres_add(len * sizeof(addr_t));
        if(!result) return addr_t(static_cast<uintptr_t>(-ENOMEM));
        array_copy(translate_user_pointer(result), obj_handle->get_init().data(), len);
        return result;
    }
    addr_t syscall_dlfini(elf64_dynamic_object* obj_handle)
    {
        size_t len = obj_handle->get_fini().size();
        addr_t result = sysres_add(len * sizeof(addr_t));
        if(!result) return addr_t(static_cast<uintptr_t>(-ENOMEM));
        array_copy(translate_user_pointer(result), obj_handle->get_fini().data(), len);
        return result;
    }
    addr_t syscall_dlpreinit(elf64_dynamic_object* obj_handle)
    {
        if(elf64_dynamic_executable* x = dynamic_cast<elf64_dynamic_executable*>(obj_handle))
        {
            size_t len = x->get_preinit().size();
            addr_t result = sysres_add(len * sizeof(addr_t));
            if(!result) return addr_t(static_cast<uintptr_t>(-ENOMEM));
            array_copy(translate_user_pointer(result), x->get_preinit().data(), len);
            return result;
        }
        else
        {
            addr_t result = sysres_add(sizeof(addr_t));
            if(!result) return addr_t(static_cast<uintptr_t>(-ENOMEM));
            result.ref<addr_t>() = nullptr;
            return result;
        }
    }
    addr_t syscall_dlopen(const char* name, int flags)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        filesystem* fs_ptr = task->get_vfs_ptr();
        if(!fs_ptr || !task->local_so_map) return addr_t(static_cast<uintptr_t>(-ENOSYS));
        if(!name) return task->object_handle; // dlopen(nullptr, ...) gives a "self" handle which resolves to a global lookup when used with dlsym
        name = translate_user_pointer(name);
        if(!name) return addr_t(static_cast<uintptr_t>(-EINVAL));
        std::string xname(name, std::strnlen(name, 256UL));
        shared_object_map::iterator result;
        if(shared_object_map::iterator cached = shared_object_map::get_globals().find(xname); cached != shared_object_map::get_globals().end()) { result = cached; }
        else 
        {
            std::vector<std::string> paths(task->dl_search_paths.cbegin(), task->dl_search_paths.cend());
            file_node* n = nullptr;
            std::string const* found_path = nullptr;
            for(std::string const& path : paths) { if(directory_node* d = fs_ptr->get_dir_nothrow(path, false)) { if(tnode* tn = d->find(xname); tn && tn->is_file()) { n = tn->as_file(); found_path = std::addressof(path); break; } } }
            if(!n) { return addr_t(static_cast<uintptr_t>(-ENOENT)); }
            if(flags & RTLD_NOLOAD)
            {
                shared_object_map::iterator so = task->local_so_map->get_if_resident(n);
                if(so == task->local_so_map->end()) return nullptr;
                if(flags & RTLD_GLOBAL) { result = task->local_so_map->transfer(shared_object_map::get_globals(), so); }
                else result = so;
            }
            else
            {
                shared_object_map& sm = flags & RTLD_GLOBAL ? shared_object_map::get_globals() : *task->local_so_map;
                result = sm.add(n);
                sm.set_path(result, *found_path);
            }
        }
        if(flags & RTLD_NODELETE) result->set_sticky();
        elf64_dynamic_object* handle = result.base();
        task->attach_object(handle);
        return handle;
    }
    int syscall_dlclose(addr_t handle)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return -ENOSYS;
        if(handle == addr_t(task->object_handle)) return 0; // dlclose on the "self" handle does nothing (UB)
        elf64_dynamic_object* obj = handle;
        elf64_shared_object* so = dynamic_cast<elf64_shared_object*>(obj);
        if(!so || !is_valid_handle(*so)) { return -EINVAL; }
        shared_object_map::iterator it(addr_t(so).minus(shared_object_map::node_offset));
        if(!task->local_so_map) return -ENOSYS;
        if(task->local_so_map->contains(so->get_soname())) { task->local_so_map->remove(it); }
        else shared_object_map::get_globals().remove(it);
        return 0;
    }
    addr_t syscall_dlsym(addr_t handle, const char* name)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return addr_t(static_cast<uintptr_t>(-ENOSYS));
        name = translate_user_pointer(name);
        if(handle == addr_t(task->object_handle)) { if(addr_t result = full_search(task, name)) return result; else return addr_t(static_cast<uintptr_t>(-ENOENT)); }
        if(!name) return addr_t(static_cast<uintptr_t>(-EINVAL));
        if(!handle) { if(addr_t result = global_search(name)) return result; else return addr_t(static_cast<uintptr_t>(-ENOENT)); }
        elf64_dynamic_object* obj = handle;
        elf64_shared_object* so = dynamic_cast<elf64_shared_object*>(obj);
        if(!so || !is_valid_handle(*so)) { return addr_t(static_cast<uintptr_t>(-EINVAL)); }
        addr_t result = so->resolve_by_name(name).second;
        if(!result) addr_t(static_cast<uintptr_t>(-ENOENT));
        return result;
    }
    addr_t syscall_dlresolve(uint32_t sym_idx)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return addr_t(static_cast<uintptr_t>(-ENOSYS));
        if(elf64_dynamic_object* obj = dynamic_cast<elf64_dynamic_object*>(task->object_handle))
        {
            elf64_rela const& rela = obj->get_plt_rela(sym_idx);
            if(rela.r_info.type != R_X86_64_JUMP_SLOT) return addr_t(static_cast<uintptr_t>(-EINVAL));
            addr_t target_pos = translate_user_pointer(obj->resolve_rela_target(rela));
            elf64_sym const& sym = obj->get_sym(rela.r_info.sym_index);
            addr_t result = full_search(task, obj->symbol_name(sym));
            if(!result) return addr_t(static_cast<uintptr_t>(-ENOENT));
            target_pos.ref<addr_t>() = result;
            return result;
        }
        else return addr_t(static_cast<uintptr_t>(-ENOSYS));
    }
    int syscall_dlpath(const char* path_str)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return -ENOSYS;
        path_str = translate_user_pointer(path_str);
        if(!path_str) return -EINVAL;
        try
        {
            size_t end_pos = std::distance(path_str, static_cast<const char*>(std::find(path_str, std::strlen(path_str), ';')));
            std::vector<std::string> paths = std::ext::split(std::string(path_str, end_pos), ':');
            task->dl_search_paths.push_back(paths.begin(), paths.end());
        }
        catch(...) { return -ENOMEM; }
        return 0;
    }
    int syscall_dlmap(elf64_dynamic_object* obj, elf64_dlmap_entry* ent)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return -ENOSYS;
        ent = translate_user_pointer(ent);
        if(!ent) return -EINVAL;
        elf64_shared_object* so = dynamic_cast<elf64_shared_object*>(obj);
        if(!so || !is_valid_handle(*so)) return -EINVAL;
        ent->dynamic_section = so->dyn_segment_ptr();
        ent->dynamic_section_length = so->dyn_segment_len();
        shared_object_map::iterator it(addr_t(so).minus(shared_object_map::node_offset));
        const char* fp;
        if(task->local_so_map->contains(so->get_soname())) fp = task->local_so_map->get_path(it);
        else fp = shared_object_map::get_globals().get_path(it);
        if(fp)
        {
            size_t len = std::strlen(fp) + 1;
            ent->absolute_pathname = sysres_add(len);
            if(!ent->absolute_pathname) return -ENOMEM;
            char* target_real = translate_user_pointer(ent->absolute_pathname);
            array_copy(target_real, fp, len - 1);
            target_real[len] = '\0';
        }
        else ent->absolute_pathname = nullptr;
        ent->vaddr_offset = so->get_load_offset();
        ent->object_handle = obj;
        return 0;
    }
    addr_t syscall_depends(elf64_dynamic_object* obj_handle)
    {
        char** result = sysres_add((obj_handle->get_dependencies().size() + 1) * sizeof(char*));
        char** result_real = translate_user_pointer(result);
        if(!result || !result_real) return addr_t(static_cast<uintptr_t>(-ENOMEM));
        size_t n = 0;
        for(const char* str : obj_handle->get_dependencies())
        {
            size_t len = std::strlen(str) + 1;
            char* target = sysres_add(len);
            if(!target) return addr_t(static_cast<uintptr_t>(-ENOMEM));
            char* target_real = translate_user_pointer(target);
            array_copy(target_real, str, len - 1);
            target_real[len] = '\0';
            result_real[n++] = target;
        }
        result_real[n] = nullptr;
        return result;
    }
    int syscall_dladdr(addr_t sym_addr, dl_addr_info* info)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return -ENOSYS;
        info = translate_user_pointer(info);
        if(!info) return -EINVAL;
        array_zero<uint64_t>(addr_t(info), sizeof(dl_addr_info) / sizeof(uint64_t));
        size_t n = task->attached_so_handles.size();
        for(size_t i = 0; i < n; i++)
        {
            elf64_shared_object* so = task->attached_so_handles[i];
            if(!so->could_contain(sym_addr)) continue;
            info->so_vbase = so->get_load_offset();
            std::string const& oname = so->get_soname();
            size_t len = oname.size() + 1;
            info->so_name = sysres_add(len);
            if(!info->so_name) return -ENOMEM;
            array_copy(translate_user_pointer(info->so_name), oname.data(), len);
            const char* sname = so->sym_lookup(sym_addr);
            if(sname) 
            {
                info->actual_addr = so->resolve_by_name(sname).second;
                len = std::strlen(sname) + 1;
                info->symbol_name = sysres_add(len);
                if(!info->symbol_name) return -ENOMEM;
                array_copy(translate_user_pointer(info->symbol_name), sname, len);
            }
            return i;
        }
        return 0;
    }
}