#include "shared_object_map.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
#include "sched/task_ctx.hpp"
#include "kdebug.hpp"
#include "sys/errno.h"
constexpr addr_t dynamic_frame_base(0x80000000000UL);
shared_object_map shared_object_map::__global_shared_objects{};
shared_object_map &shared_object_map::get_globals() { return __global_shared_objects; }
shared_object_map::shared_object_map(uframe_tag *frame, size_type init_ct) : __base(init_ct), shared_frame{ frame ? frame : std::addressof(frame_manager::get().create_frame(dynamic_frame_base, dynamic_frame_base)) } {}
shared_object_map::shared_object_map(shared_object_map const& that, uframe_tag* nframe) : __base(that.__bucket_count), shared_frame{ nframe } { for(elf64_shared_object const& so : that) { insert(std::move(elf64_shared_object(so, nframe))); } }
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
bool shared_object_map::remove(iterator so_handle) { so_handle->decref(); if(so_handle->refs() || so_handle->is_sticky()) return false; erase(so_handle); return true; }
shared_object_map::iterator shared_object_map::transfer(shared_object_map& that, iterator handle) { iterator result = that.emplace(std::move(*handle)).first; erase(result->get_soname()); for(block_descr& d : result->segment_blocks()) { shared_frame->transfer_block(*that.shared_frame, d); } result->frame_tag = that.shared_frame; return result; }
constexpr static shared_object_map::iterator ecode(int e) { return shared_object_map::iterator(addr_t(uintptr_t(-e))); }
static addr_t local_search(task_ctx* task, const char* name) { for(elf64_shared_object& so : *task->local_so_map) { if(addr_t result = so.resolve_by_name(name)) return result; } return nullptr; }
static addr_t global_search(const char* name) { for(elf64_shared_object& so : shared_object_map::get_globals()) { if(addr_t result = so.resolve_by_name(name)) return result; } return nullptr; }
extern "C"
{
    shared_object_map::iterator syscall_dlopen(const char* name, int flags)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!name) return shared_object_map::iterator(addr_t(task)); // other calls using these pointers will check for this special handle, which is the "self" handle
        filesystem* fs_ptr = task->get_vfs_ptr();
        if(!fs_ptr || !task->local_so_map) return ecode(ENOSYS);
        name = translate_user_pointer(name);
        std::string xname(name, std::strnlen(name, 256UL));
        std::vector<std::string> paths{};
        for(const char* p : task->dl_search_paths) { paths.emplace_back(p, std::strnlen(p, 256UL)); }
        // TODO: also search by environment variable if present
        file_node* n = nullptr;
        for(std::string const& path : paths) { if(directory_node* d = fs_ptr->get_dir_nothrow(path, false)) { if(tnode* tn = d->find(xname); tn && tn->is_file()) { n = tn->as_file(); break; } } }
        if(!n) { return ecode(ENOENT); }
        // TODO: bind symbols; account for lazy vs immediate binding
        if(flags & RTLD_NOLOAD)
        {
            shared_object_map::iterator so = task->local_so_map->get_if_resident(n);
            if(so != task->local_so_map->end() && (flags & RTLD_GLOBAL)) { so = task->local_so_map->transfer(shared_object_map::get_globals(), so); }
            if(so && (flags & RTLD_NODELETE)) { so->set_sticky(); }
            return so;
        }
        shared_object_map::iterator result = flags & RTLD_GLOBAL ? shared_object_map::get_globals().add(n) : task->local_so_map->add(n);
        if(flags & RTLD_NODELETE) result->set_sticky();
        return result;
    }
    int syscall_dlclose(shared_object_map::iterator handle)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(addr_t(task) == addr_t(handle.get_node()) || !is_valid_handle(*handle)) return -EINVAL;
        if(!task->local_so_map) return -ENOSYS;
        if(task->local_so_map->contains(handle->get_soname())) { task->local_so_map->remove(handle); }
        else shared_object_map::get_globals().remove(handle);
        return 0;
    }
    addr_t syscall_dlsym(shared_object_map::iterator handle, const char *name)
    {
        task_ctx* task = get_gs_base<task_ctx>();
        if(!task->local_so_map) return addr_t(uintptr_t(-ENOSYS));
        name = translate_user_pointer(name);
        addr_t result;
        if(addr_t(task) == addr_t(handle.get_node()))
        {
            // TODO: include the program itself in the search
            if((result = local_search(task, name))) return result;
            if((result = global_search(name))) return result;
            return addr_t(uintptr_t(-ENOENT));
        }
        if(!handle.get_node()) { if((result = global_search(name))) return result; else return addr_t(uintptr_t(-ENOENT)); }
        if(!is_valid_handle(*handle)) { return addr_t(uintptr_t(-EINVAL)); }
        result = handle->resolve_by_name(name);
        if(!result) addr_t(uintptr_t(-ENOENT));
        return result;
    }
}