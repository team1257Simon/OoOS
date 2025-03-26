#include "shared_object_map.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "kdebug.hpp"
#include "sys/errno.h"
constexpr addr_t dynamic_frame_base(0x80000000000UL);
shared_object_map shared_object_map::__global_shared_objects{};
shared_object_map &shared_object_map::get_globals() { return __global_shared_objects; }
shared_object_map::shared_object_map(uframe_tag *frame, size_type init_ct) : __base(init_ct), __obj_paths(init_ct), shared_frame{ frame ? frame : std::addressof(frame_manager::get().create_frame(dynamic_frame_base, dynamic_frame_base)) } { if(!shared_frame->dynamic_extent) { shared_frame->dynamic_extent = dynamic_frame_base; } }
shared_object_map::shared_object_map(shared_object_map const& that, uframe_tag* nframe) : __base(that.__bucket_count), __obj_paths(that.__obj_paths), shared_frame{ nframe } { for(elf64_shared_object const& so : that) { insert(std::move(elf64_shared_object(so, nframe))); } }
shared_object_map::~shared_object_map() { if(shared_frame) frame_manager::get().destroy_frame(*shared_frame); }
elf64_shared_object& shared_object_map::operator[](std::string const& name) { if(__node_ptr np = __find(name)) return np->__ref(); else throw std::out_of_range{ "Not found: " + name }; }
elf64_shared_object const& shared_object_map::operator[](std::string const& name) const { if(__const_node_ptr np = __find(name)) { return np->__ref(); } else throw std::out_of_range{ "Not found: " + name }; }
bool shared_object_map::contains(std::string const& name) const { return __base::contains(name); }
void shared_object_map::clear() { __base::clear(); }
shared_object_map::iterator shared_object_map::begin() noexcept { return __base::begin(); }
shared_object_map::const_iterator shared_object_map::begin() const noexcept { return __base::begin(); }
shared_object_map::const_iterator shared_object_map::cbegin() const noexcept { return __base::cbegin(); }
shared_object_map::iterator shared_object_map::end() noexcept { return __base::end(); }
shared_object_map::const_iterator shared_object_map::end() const noexcept { return __base::end(); }
shared_object_map::const_iterator shared_object_map::cend() const noexcept { return __base::cend(); }
shared_object_map::iterator shared_object_map::find(std::string const& what) noexcept { return __base::find(what); }
shared_object_map::const_iterator shared_object_map::find(std::string const& what) const noexcept { return __base::find(what); }
shared_object_map::size_type shared_object_map::size() const noexcept { return __base::size(); }
shared_object_map::iterator shared_object_map::get_if_resident(file_node* so_file) { elf64_shared_object so(so_file, shared_frame); iterator result = find(so.get_soname()); if(result != end()) result->incref(); return result; }
shared_object_map::iterator shared_object_map::add(file_node* so_file) { auto result = emplace(so_file, shared_frame); if(result.second && !result.first->load()) { erase(result.first); throw std::runtime_error{ "load failed" }; } if(!result.second) result.first->incref(); return result.first; }
void shared_object_map::set_path(iterator obj, std::string const& path) { __obj_paths.insert_or_assign(obj, std::move(std::string(path))); }
const char *shared_object_map::get_path(iterator obj) const { if(auto i = __obj_paths.find(obj); i != __obj_paths.end()) { return i->second.c_str(); } else return nullptr; }
bool shared_object_map::remove(iterator so_handle) { so_handle->decref(); if(so_handle->refs() || so_handle->is_sticky()) return false; __obj_paths.erase(so_handle); erase(so_handle); return true; }
shared_object_map::iterator shared_object_map::transfer(shared_object_map& that, iterator handle) { const char* hpath = get_path(handle); std::string xhpath(hpath ? hpath : ""); iterator result = that.emplace(std::move(*handle)).first; __obj_paths.erase(handle); erase(result->get_soname()); for(block_descr& d : result->segment_blocks()) { shared_frame->transfer_block(*that.shared_frame, d); } result->frame_tag = that.shared_frame; that.set_path(result, xhpath); return result; }
constexpr static shared_object_map::iterator ecode(int e) { return shared_object_map::iterator(addr_t(static_cast<uintptr_t>(-e))); }
static addr_t global_search(const char* name) { for(elf64_shared_object& so : shared_object_map::get_globals()) { if(addr_t result = so.resolve_by_name(name)) return result; } return nullptr; }
static addr_t full_search(task_ctx* task, const char* name) { addr_t result; if(elf64_dynamic_object* dyn = dynamic_cast<elf64_dynamic_object*>(task->object_handle); dyn && (result = dyn->resolve_by_name(name))) return result; for(elf64_shared_object& so : *task->local_so_map) { if((result = so.resolve_by_name(name))) return result; } return global_search(name); }
extern "C"
{
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
            return so.get_node();
        }
        shared_object_map& sm = flags & RTLD_GLOBAL ? shared_object_map::get_globals() : *task->local_so_map;
        shared_object_map::iterator result = sm.add(n);
        std::string full_path = *found_path + fs_ptr->get_path_separator() + xname;
        sm.set_path(result, full_path);
        if(flags & RTLD_NODELETE) result->set_sticky();
        return result.get_node();
    }
    int syscall_dlclose(addr_t handle)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return -ENOSYS;
        if(handle == addr_t(task)) return 0; // dlclose on the "self" handle does nothing (UB)
        shared_object_map::iterator so(handle);
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
        shared_object_map::iterator so(handle);
        if(!is_valid_handle(*so)) { return addr_t(static_cast<uintptr_t>(-EINVAL)); }
        addr_t result = so->resolve_by_name(name);
        if(!result) addr_t(static_cast<uintptr_t>(-ENOENT));
        return result;
    }
    addr_t syscall_getsym(const char* name)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return addr_t(static_cast<uintptr_t>(-ENOSYS));
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
        shared_object_map::iterator so(handle);
        if(!is_valid_handle(*so)) return -EINVAL;
        const char* path;
        if(!((path = task->local_so_map->get_path(so)) || (path = shared_object_map::get_globals().get_path(so)))) return -ENOENT;
        size_t len = std::strlen(path);
        uframe_tag* tag = task->task_struct.frame_ptr;
        if(static_cast<size_t>(tag->sysres_extent - tag->sysres_wm) < len)
        {
            kernel_memory_mgr::get().enter_frame(tag);
            addr_t mapping_target = tag->sysres_extent;
            addr_t allocated = kernel_memory_mgr::get().allocate_user_block(len, mapping_target, 0UL, false, false);
            if(!allocated) { kernel_memory_mgr::get().exit_frame(); return -ENOMEM; }
            tag->kernel_allocated_blocks.push_back(allocated);
            tag->sysres_extent += kernel_memory_mgr::get().page_aligned_region_size(mapping_target, len);
            kernel_memory_mgr::get().exit_frame();
        }
        char* result = tag->sysres_wm;
        tag->sysres_wm += len;
        array_copy(result, path, len);
        *path_out = result;
        if(sz_out) *sz_out = len;
        return 0;
    }
}