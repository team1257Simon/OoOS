#include "shared_object_map.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "kdebug.hpp"
#include "sys/errno.h"
static addr_t sysres_add(size_t len) { return current_active_task()->frame_ptr.ref<uframe_tag>().sysres_add(len); }
static addr_t global_search(const char* name) { for(elf64_shared_object const& so : shared_object_map::get_globals()) { if(addr_t result = so.resolve_by_name(name)) return result; } return nullptr; }
static addr_t full_search(task_ctx* task, const char* name)
{ 
    addr_t result; 
    if(elf64_dynamic_object* dyn = dynamic_cast<elf64_dynamic_object*>(task->object_handle); dyn && (result = dyn->resolve_by_name(name))) return result; 
    for(elf64_shared_object& so : *task->local_so_map) { if((result = so.resolve_by_name(name))) return result; } 
    return global_search(name);
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
    addr_t syscall_dlopen(const char* name, int flags)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!name) return addr_t(task); // other calls using these pointers will check for this special handle, which is the "self" handle
        filesystem* fs_ptr = task->get_vfs_ptr();
        if(!fs_ptr || !task->local_so_map) return addr_t(static_cast<uintptr_t>(-ENOSYS));
        name = translate_user_pointer(name);
        if(!name) return addr_t(static_cast<uintptr_t>(-EINVAL));
        std::string xname(name, std::strnlen(name, 256UL));
        std::vector<std::string> paths(task->dl_search_paths.cbegin(), task->dl_search_paths.cend());
        file_node* n = nullptr;
        std::string const* found_path = nullptr;
        for(std::string const& path : paths) { if(directory_node* d = fs_ptr->get_dir_nothrow(path, false)) { if(tnode* tn = d->find(xname); tn && tn->is_file()) { n = tn->as_file(); found_path = std::addressof(path); break; } } }
        if(!n) { return addr_t(static_cast<uintptr_t>(-ENOENT)); }
        if(flags & RTLD_NOLOAD)
        {
            shared_object_map::iterator so = task->local_so_map->get_if_resident(n);
            if(so != task->local_so_map->end() && (flags & RTLD_GLOBAL)) { so = task->local_so_map->transfer(shared_object_map::get_globals(), so); }
            if(so && (flags & RTLD_NODELETE)) { so->set_sticky(); }
            return so.base();
        }
        shared_object_map& sm = flags & RTLD_GLOBAL ? shared_object_map::get_globals() : *task->local_so_map;
        shared_object_map::iterator result = sm.add(n);
        std::string full_path = *found_path + fs_ptr->get_path_separator() + xname;
        sm.set_path(result, full_path);
        if(flags & RTLD_NODELETE) result->set_sticky();
        return result.base();
    }
    int syscall_dlclose(addr_t handle)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return -ENOSYS;
        if(handle == addr_t(task)) return 0; // dlclose on the "self" handle does nothing (UB)
        shared_object_map::iterator so(handle.minus(shared_object_map::node_offset));
        if(!is_valid_handle(*so)) { return -EINVAL; }
        if(!task->local_so_map) return -ENOSYS;
        if(task->local_so_map->contains(so->get_soname())) { task->local_so_map->remove(so); }
        else shared_object_map::get_globals().remove(so);
        return 0;
    }
    addr_t syscall_dlsym(addr_t handle, const char* name)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return addr_t(static_cast<uintptr_t>(-ENOSYS));
        name = translate_user_pointer(name);
        if(handle == addr_t(task)) { if(addr_t result = full_search(task, name)) return result; else return addr_t(static_cast<uintptr_t>(-ENOENT)); }
        if(!name) return addr_t(static_cast<uintptr_t>(-EINVAL));
        if(!handle) { if(addr_t result = global_search(name)) return result; else return addr_t(static_cast<uintptr_t>(-ENOENT)); }
        shared_object_map::iterator so(handle.minus(shared_object_map::node_offset));
        if(!is_valid_handle(*so)) { return addr_t(static_cast<uintptr_t>(-EINVAL)); }
        addr_t result = so->resolve_by_name(name);
        if(!result) addr_t(static_cast<uintptr_t>(-ENOENT));
        return result;
    }
    addr_t syscall_getsym(const char* name)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return addr_t(static_cast<uintptr_t>(-ENOSYS));
        name = translate_user_pointer(name);
        if(!name) return addr_t(static_cast<uintptr_t>(-EINVAL));
        if(addr_t result = full_search(task, name)) return result;
        return addr_t(static_cast<uintptr_t>(-ENOENT));
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
    int syscall_dlorigin(addr_t handle, const char** path_out, size_t* sz_out)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map || !task->ctx_filesystem) return -ENOSYS;
        if(path_out) path_out = translate_user_pointer(path_out);
        if(!path_out) return -EINVAL;
        if(sz_out) { sz_out = translate_user_pointer(sz_out); if(!sz_out) return -EINVAL; }
        shared_object_map::iterator so(handle.minus(shared_object_map::node_offset));
        if(!is_valid_handle(*so)) return -EINVAL;
        const char* path;
        if(!((path = task->local_so_map->get_path(so)) || (path = shared_object_map::get_globals().get_path(so)))) return -ENOENT;
        size_t len = std::strlen(path);
        if(char* result = sysres_add(len))
        {
            array_copy(translate_user_pointer(result), path, len);
            *path_out = result;
            if(sz_out) *sz_out = len;
            return 0;
        }
        else return -ENOMEM;
    }
    addr_t syscall_depends(elf64_dynamic_object* obj_handle)
    {
        char** result = sysres_add((obj_handle->get_dependencies().size() + 1) * sizeof(char*));
        char** result_real = translate_user_pointer(result);
        if(!result) return addr_t(static_cast<uintptr_t>(-ENOMEM));
        if(!result_real) return addr_t(static_cast<uintptr_t>(-EINVAL));
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
}