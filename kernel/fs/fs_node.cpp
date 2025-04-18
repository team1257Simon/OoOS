#include "fs/fs.hpp"
#include "rtc.h"
fs_node::fs_node(std::string const& name, int vfd, uint64_t cid) : fd{ vfd }, real_id{ cid }, create_time{ sys_time(nullptr) }, modif_time{ create_time }, concrete_name{ name } {}
int fs_node::vid() const noexcept { return fd; }
void fs_node::vid(int id) noexcept { fd = id; }
uint64_t fs_node::cid() const noexcept { return real_id; }
uint64_t fs_node::created_time() const noexcept { return create_time; }
uint64_t fs_node::modified_time() const noexcept { return modif_time; }
bool fs_node::rename(std::string const& n) { concrete_name = n; return true; }
const char *fs_node::name() const { return concrete_name.c_str(); }
bool fs_node::is_file() const noexcept { return false; }
bool fs_node::is_directory() const noexcept { return false; }
bool fs_node::is_device() const noexcept { return false; }
void fs_node::add_reference(tnode* ref) { refs.insert(ref); }
void fs_node::rm_reference(tnode* ref) { if(refs.erase(ref)) { ref->invlnode(); } }
void fs_node::prune_refs() { for(tnode* ref : refs) ref->invlnode(); refs.clear(); }
bool fs_node::has_refs() const noexcept { return refs.size() != 0; }
size_t fs_node::num_refs() const noexcept { return refs.size(); }
fs_node::~fs_node() { prune_refs(); }
file_node::file_node(std::string const& name, int vfd, uint64_t cid) : fs_node{ name, vfd, cid } {}
bool file_node::is_file() const noexcept { return true; }
bool file_node::chk_lock() const noexcept { return !test_lock(std::addressof(__my_lock)); }
void file_node::acq_lock() { acquire(std::addressof(__my_lock)); }
void file_node::rel_lock() { release(std::addressof(__my_lock)); }
directory_node::directory_node(std::string const& name, uint64_t cid) : fs_node{ name, -1, cid } {}
bool directory_node::is_directory() const noexcept { return true; }
uint64_t directory_node::size() const noexcept { return num_files() + num_subdirs(); }
bool directory_node::is_empty() const noexcept { return size() == 0; }
bool directory_node::relink(std::string const& oldn, std::string const& newn) { if(tnode* ptr = find(oldn)) { return unlink(oldn) && link(ptr, newn); } else return false; } 
device_node::device_node(std::string const& name, int fd, device_buffer* dev_buffer) : file_node{ name, fd, reinterpret_cast<uint64_t>(dev_buffer) }, __my_device{ dev_buffer } { mode = 027666; }
bool device_node::fsync() { return __my_device->pubsync() == 0; }
bool device_node::is_device() const noexcept { return true; }
uint64_t device_node::size() const noexcept { return __my_device->in_avail(); }
device_node::pos_type device_node::tell() const { return __my_device->tell(); }
device_node::size_type device_node::write(const_pointer src, size_type n) { size_type result = __my_device->sputn(src, n); if(result) sys_time(std::addressof(modif_time)); return result; }
device_node::size_type device_node::read(pointer dest, size_type n) { return __my_device->sgetn(dest, n); }
device_node::pos_type device_node::seek(off_type off, std::ios_base::seekdir way) { return __my_device->pubseekoff(off, way); }
device_node::pos_type device_node::seek(pos_type pos) { return __my_device->pubseekpos(pos); }
tnode::tnode(fs_node* node, std::string const& name) : __my_node{ node }, __my_name{ name } { __my_node->refs.insert(this); }
tnode::tnode(fs_node* node, const char* name) : __my_node{ node }, __my_name{ name } { __my_node->refs.insert(this); }
tnode::tnode(std::string name) : __my_node { nullptr }, __my_name{ name } {}
tnode::tnode(const char* name) : __my_node{ nullptr }, __my_name{ name } {}
void tnode::rename(std::string const& n) { __my_name = n; }
void tnode::rename(const char* n) { rename(std::string(n, std::strlen(n))); }
const char *tnode::name() const { return __my_name.c_str(); }
fs_node* tnode::ptr() noexcept { return __my_node; }
fs_node const* tnode::ptr() const noexcept { return __my_node; }
fs_node& tnode::ref() noexcept { return *__my_node; }
fs_node const& tnode::ref() const noexcept { return *__my_node; }
fs_node &tnode::operator*() noexcept { return *__my_node; }
fs_node const &tnode::operator*() const noexcept { return *__my_node; }
fs_node *tnode::operator->() noexcept { return __my_node; }
fs_node const *tnode::operator->() const noexcept { return __my_node; }
bool tnode::if_file(std::function<bool(file_node&)> const& action) { return is_file() && action(dynamic_cast<file_node&>(*__my_node)); }
bool tnode::if_folder(std::function<bool(directory_node&)> const& action) { return is_directory() && action(dynamic_cast<directory_node&>(*__my_node)); }
bool tnode::if_device(std::function<bool(device_node&)> const& action) { return is_device() && action(dynamic_cast<device_node&>(*__my_node)); }
bool tnode::if_file(std::function<bool(file_node const&)> const& action) const { return  is_file() && action(dynamic_cast<file_node const&>(*__my_node)); }
bool tnode::if_folder(std::function<bool(directory_node const&)> const& action) const { return is_directory() && action(dynamic_cast<directory_node const&>(*__my_node)); }
bool tnode::if_device(std::function<bool(device_node const&)> const& action) const { return is_device() && action(dynamic_cast<device_node const&>(*__my_node)); }
file_node* tnode::as_file() { return dynamic_cast<file_node*>(__my_node); }
file_node const* tnode::as_file() const { return dynamic_cast<file_node const*>(__my_node); }
directory_node* tnode::as_directory() { return dynamic_cast<directory_node*>(__my_node); }
directory_node const* tnode::as_directory() const { return dynamic_cast<directory_node*>(__my_node); }
device_node *tnode::as_device() { return dynamic_cast<device_node*>(__my_node); }
device_node const *tnode::as_device() const { return dynamic_cast<device_node const *>(__my_node); }
bool tnode::is_file() const { return __my_node && __my_node->is_file(); }
bool tnode::is_directory() const { return __my_node && __my_node->is_directory(); }
bool tnode::is_device() const { return __my_node && __my_node->is_device(); }
void tnode::invlnode() noexcept { __my_node = nullptr; }
bool tnode::assign(fs_node* n) noexcept { if(!__my_node) { __my_node = n; return true; } else return false; }
tnode mklink(tnode* original, std::string const& name) { return tnode(original->__my_node, name); }