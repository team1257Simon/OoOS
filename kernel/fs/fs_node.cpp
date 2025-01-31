#include "fs/fs.hpp"
#include "rtc.h"
inode::inode(std::string const &name, int vfd, uint64_t cid) : fd{ vfd }, real_id{ cid }, create_time{ syscall_time(nullptr) }, modif_time{ create_time }, concrete_name{ name } {}
int inode::vid() const noexcept { return fd; }
void inode::vid(int id) noexcept { this->fd = id; }
uint64_t inode::cid() const noexcept { return real_id; }
uint64_t inode::created_time() const noexcept { return create_time; }
uint64_t inode::modified_time() const noexcept { return modif_time; }
bool inode::rename(std::string const& n) { this->concrete_name = n; return true; }
const char *inode::name() const { return concrete_name.c_str(); }
bool inode::is_file() const noexcept { return false; }
bool inode::is_folder() const noexcept { return false; }
bool inode::is_device() const noexcept { return false; }
void inode::unregister_reference(tnode *ref) { if(refs.erase(ref)) ref->invlnode(); }
void inode::prune_refs() { for(tnode* ref : refs) ref->invlnode(); refs.clear(); }
bool inode::has_refs() const noexcept { return refs.size() != 0; }
size_t inode::num_refs() const noexcept { return refs.size(); }
inode::~inode() { prune_refs(); }
bool file_inode::is_file() const noexcept { return true; }
bool file_inode::chk_lock() const noexcept { return !test_lock(&__my_lock); }
void file_inode::acq_lock() { acquire(&__my_lock); }
void file_inode::rel_lock() { release(&__my_lock); }
folder_inode::folder_inode(std::string const &name, uint64_t cid) : inode{ name, -1, cid } {}
bool folder_inode::is_folder() const noexcept { return true; }
uint64_t folder_inode::size() const noexcept { return this->num_files() + this->num_folders(); }
bool folder_inode::is_empty() const noexcept { return this->size() == 0; }
bool folder_inode::relink(std::string const &oldn, std::string const &newn) { if(tnode* ptr = this->find(oldn)) { return this->unlink(oldn) && this->link(ptr, newn); } else return false; } 
device_inode::device_inode(std::string const &name, int fd, vfs_filebuf_base<char> *dev_buffer) : file_inode{ name, fd, reinterpret_cast<uint64_t>(dev_buffer) }, __my_device{ dev_buffer } {}
bool device_inode::fsync() { return __my_device->pubsync() == 0; }
bool device_inode::is_device() const noexcept { return true; }
uint64_t device_inode::size() const noexcept { return __my_device->in_avail(); }
device_inode::size_type device_inode::write(const_pointer src, size_type n) { size_type result{ __my_device->sputn(src, n) }; if(result) syscall_time(&this->modif_time); return result; }
device_inode::size_type device_inode::read(pointer dest, size_type n) { return __my_device->sgetn(dest, n); }
device_inode::pos_type device_inode::seek(off_type off, std::ios_base::seekdir way) { return __my_device->pubseekoff(off, way); }
device_inode::pos_type device_inode::seek(pos_type pos) { return __my_device->pubseekpos(pos); }
tnode::tnode(inode* node, std::string const& name) : __my_node{ node }, __my_name{ name } { __my_node->refs.insert(this); }
tnode::tnode(inode* node, const char *name) : __my_node{ node }, __my_name{ name } { __my_node->refs.insert(this); }
tnode::tnode(std::string const& name) : __my_node { nullptr }, __my_name{ name } {}
void tnode::rename(std::string const& n) { __my_name = n; }
void tnode::rename(const char* n) { __my_name = n; }
const char *tnode::name() const { return __my_name.c_str(); }
inode &tnode::operator*() noexcept { return *__my_node; }
inode const &tnode::operator*() const noexcept { return *__my_node; }
inode *tnode::operator->() noexcept { return __my_node; }
inode const *tnode::operator->() const noexcept { return __my_node; }
bool tnode::if_file(std::function<bool(file_inode &)> const &action) { return this->is_file() && action(dynamic_cast<file_inode&>(*__my_node)); }
bool tnode::if_folder(std::function<bool(folder_inode &)> const &action) { return this->is_folder() && action(dynamic_cast<folder_inode&>(*__my_node)); }
bool tnode::if_file(std::function<bool(file_inode const &)> const &action) const { return  this->is_file() && action(dynamic_cast<file_inode const&>(*__my_node)); }
bool tnode::if_folder(std::function<bool(folder_inode const &)> const &action) const { return this->is_folder() && action(dynamic_cast<folder_inode const&>(*__my_node)); }
bool tnode::if_device(std::function<bool(device_inode &)> const &action) { return this->is_device() && action(dynamic_cast<device_inode&>(*__my_node)); }
bool tnode::if_device(std::function<bool(device_inode const &)> const &action) const { return this->is_device() && action(dynamic_cast<device_inode const&>(*__my_node)); }
file_inode::file_inode(std::string const &name, int vfd, uint64_t cid) : inode{ name, vfd, cid } {}
file_inode* tnode::as_file() { return dynamic_cast<file_inode*>(__my_node); }
file_inode const* tnode::as_file() const { return dynamic_cast<file_inode const*>(__my_node); }
folder_inode* tnode::as_folder() { return dynamic_cast<folder_inode*>(__my_node); }
folder_inode const* tnode::as_folder() const { return dynamic_cast<folder_inode*>(__my_node); }
device_inode *tnode::as_device() { return dynamic_cast<device_inode*>(__my_node); }
device_inode const *tnode::as_device() const { return dynamic_cast<device_inode const *>(__my_node); }
bool tnode::is_file() const { return __my_node && __my_node->is_file(); }
bool tnode::is_folder() const { return __my_node && __my_node->is_folder(); }
bool tnode::is_device() const { return __my_node && __my_node->is_device(); }
void tnode::invlnode() noexcept { __my_node = nullptr; }
bool tnode::assign(inode *n) noexcept { if(!__my_node) { __my_node = n; return true; } else return false; }
tnode mklink(tnode *original, std::string const &name) { return tnode(original->__my_node, name); }