#include "shared_object_map.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
#include "kdebug.hpp"
constexpr addr_t dynamic_frame_base(0x80000000000UL);
shared_object_map::shared_object_map(uframe_tag *frame, size_type init_ct) : __base(init_ct), shared_frame{ frame ? frame : std::addressof(frame_manager::get().create_frame(dynamic_frame_base, dynamic_frame_base)) } {}
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