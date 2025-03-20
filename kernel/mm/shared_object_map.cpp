#include "shared_object_map.hpp"
#include "frame_manager.hpp"
#include "stdexcept"
#include "kdebug.hpp"
constexpr addr_t dynamic_frame_base(0x80000000000UL);
shared_object_map::shared_object_map(size_type init_ct) : __base(init_ct), __shared_frame{ std::addressof(frame_manager::get().create_frame(dynamic_frame_base, dynamic_frame_base)) } {}
shared_object_map::~shared_object_map() { frame_manager::get().destroy_frame(*__shared_frame); }
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
shared_object_map::size_type shared_object_map::erase(std::string const& what) { return __base::erase(what); }
shared_object_map::iterator shared_object_map::erase(const_iterator where) { return __base::erase(where); }
shared_object_map::size_type shared_object_map::size() const noexcept { return __base::size(); }
shared_object_map::iterator shared_object_map::add(file_node* so_file) { auto result = emplace(so_file, __shared_frame); if(result.second && !result.first->load()) { erase(result.first); throw std::runtime_error{ "load failed" }; } return result.first; }