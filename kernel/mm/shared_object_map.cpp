#define INST_SOM
#include "shared_object_map.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "kdebug.hpp"
#include "sys/errno.h"
constexpr addr_t dynamic_frame_base(0x80000000000UL);
shared_object_map shared_object_map::__globals{};
shared_object_map::iterator shared_object_map::__ld_so = __globals.end();
static inline uframe_tag* create_shared_frame() { return std::addressof(fm.create_frame(dynamic_frame_base, dynamic_frame_base)); }
shared_object_map& shared_object_map::get_globals() { return __globals; }
addr_t shared_object_map::__global_dynamic_extent() { return (__globals.shared_frame && __globals.shared_frame->dynamic_extent) ? __globals.shared_frame->dynamic_extent : dynamic_frame_base; }
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
void shared_object_map::set_path(iterator obj, std::string const& path) { __obj_paths.insert(std::make_pair(obj, std::move(std::string(path)))); }
const char* shared_object_map::get_path(iterator obj) const { if(auto i = __obj_paths.find(obj); i != __obj_paths.end()) { return i->second.c_str(); } else return nullptr; }
shared_object_map::shared_object_map(uframe_tag* frame, size_type init_ct) : 
    __base          ( init_ct ), 
    __obj_paths     ( init_ct ), 
    shared_frame    { frame ? frame : create_shared_frame() } 
                    { if(!shared_frame->dynamic_extent) { shared_frame->dynamic_extent = __global_dynamic_extent(); } }
bool shared_object_map::remove(iterator so_handle)
{
    so_handle->decref();
    if(so_handle->refs() || so_handle->is_sticky()) return false;
    __obj_paths.erase(so_handle); erase(so_handle);
    return true; 
}
shared_object_map::iterator shared_object_map::get_if_resident(file_node* so_file) 
{
    elf64_shared_object so(so_file, shared_frame); 
    iterator result = find(so.get_soname()); 
    if(result != end()) result->incref(); 
    return result; 
}
shared_object_map::iterator shared_object_map::add(file_node* so_file) 
{
    std::pair<iterator, bool> result = emplace(so_file, shared_frame); 
    if(result.second && !result.first->load()) { erase(result.first); throw std::runtime_error{ "load failed" }; } 
    if(!result.second) result.first->incref();
    else if(std::addressof(__globals) == this) { result.first->set_global(); }
    return result.first; 
}
shared_object_map::iterator shared_object_map::transfer(shared_object_map& that, iterator handle) 
{ 
    const char* hpath = get_path(handle); 
    std::string xhpath(hpath ? hpath : ""); 
    iterator result = that.emplace(std::move(*handle)).first; 
    __obj_paths.erase(handle); 
    erase(result->get_soname()); 
    for(block_descriptor& d : result->segment_blocks()) { shared_frame->transfer_block(*that.shared_frame, d); } 
    result->frame_tag = that.shared_frame; 
    that.set_path(result, xhpath);
    if(std::addressof(that) == std::addressof(__globals)) result->set_global();
    else result->set_global(false);
    return result; 
}
shared_object_map::iterator shared_object_map::get_ldso_object(filesystem* fs)
{
    if(__ld_so == __globals.end())
    {
        if(!fs) throw std::invalid_argument{ "need fs pointer to initialize ld.so object" };
        file_node* n = fs->open_file("lib/ld-ooos.so", std::ios_base::in);
        __ld_so = __globals.add(n);
        fs->close_file(n);
    }
    return __ld_so;
}
void shared_object_map::copy(shared_object_map const& that)
{
    if(std::addressof(that) == std::addressof(__globals)) throw std::invalid_argument{ "cannot clone global objects" };
    for(const_iterator i = that.begin(); i != that.end(); i++)
    {
        iterator j = emplace(*i).first;
        j->on_copy(shared_frame);
        set_path(j, that.get_path(iterator(const_cast<__node_ptr>(i.get_node()))));
    }
}