#include "fs/fs_node.hpp"
#include "rtc.h"
inode_base::inode_base(std::string const &name, int vfd, uint64_t cid) : real_name{name}, fd{vfd}, real_id{cid}, create_time{ syscall_time(0) }, modif_time{ create_time } {}
int inode_base::vid() const noexcept { return fd; }
uint64_t inode_base::cid() const noexcept { return real_id; }
uint64_t inode_base::created_time() const noexcept { return create_time; }
uint64_t inode_base::modified_time() const noexcept { return modif_time; }
bool inode_base::rename(std::string const& n) { this->real_name = n; return true; }
const char *inode_base::name() const { return real_name.c_str(); }
bool file_inode_base::is_file() const noexcept { return true; }
bool file_inode_base::is_folder() const noexcept { return false; }
bool file_inode_base::chk_lock() const noexcept { return !test_lock(&__my_lock); }
void file_inode_base::acq_lock() { acquire(&__my_lock); }
void file_inode_base::rel_lock() { release(&__my_lock); }
file_inode_base::pos_type file_inode_base::seek(off_type offs, std::ios_base::seekdir way) { return this->xseekl(offs, way); }
file_inode_base::pos_type file_inode_base::seek(pos_type pos) { return this->xseekp(pos); }
file_inode_base::size_type file_inode_base::write(const_pointer src, size_type n) { return this->xwrite(src, n); }
file_inode_base::size_type file_inode_base::read(pointer dest, size_type n) { return this->xread(dest, n); }
tnode::tnode(inode_base* node, std::string const& name) : __my_node{ node }, __my_name{ name } { __my_node->refs.insert(this); }
tnode::tnode(inode_base* node, const char *name) : __my_node{node}, __my_name{name} { __my_node->refs.insert(this); }
void tnode::rename(std::string const& n) { __my_name = n; }
void tnode::rename(const char* n) { __my_name = n; }
const char *tnode::name() const { return __my_name.c_str(); }
inode_base &tnode::operator*() noexcept { return *__my_node; }
inode_base const &tnode::operator*() const noexcept { return *__my_node; }
inode_base *tnode::operator->() noexcept { return __my_node; }
inode_base const *tnode::operator->() const noexcept { return __my_node; }
bool tnode::if_file(std::function<bool(file_inode_base &)> const &action) { return this->is_file() && action(dynamic_cast<file_inode_base&>(*__my_node)); }
bool tnode::if_folder(std::function<bool(folder_inode_base &)> const &action) { return this->is_folder() && action(dynamic_cast<folder_inode_base&>(*__my_node)); }
bool tnode::if_file(std::function<bool(file_inode_base const &)> const &action) const { return  this->is_file() && action(dynamic_cast<file_inode_base const&>(*__my_node)); }
bool tnode::if_folder(std::function<bool(folder_inode_base const &)> const &action) const { return this->is_folder() && action(dynamic_cast<folder_inode_base const&>(*__my_node)); }
file_inode_base::file_inode_base(std::string const &name, int vfd, uint64_t cid) : inode_base{ name, vfd, cid } {}
file_inode_base* tnode::as_file() { return dynamic_cast<file_inode_base*>(__my_node); }
file_inode_base const* tnode::as_file() const { return dynamic_cast<file_inode_base const*>(__my_node); }
folder_inode_base* tnode::as_folder() { return dynamic_cast<folder_inode_base*>(__my_node); }
folder_inode_base const* tnode::as_folder() const { return dynamic_cast<folder_inode_base*>(__my_node); }
tnode mklink(tnode *original, std::string const &name) { return tnode(original->__my_node, name); }
bool tnode::is_file() const { return __my_node && __my_node->is_file(); }
bool tnode::is_folder() const { return __my_node && __my_node->is_folder(); }
void tnode::invlnode() noexcept { __my_node = nullptr; }
void inode_base::prune_refs() { for(tnode* ref : refs) ref->invlnode(); refs.clear(); }
inode_base::~inode_base() { prune_refs(); }
folder_inode_base::folder_inode_base(std::string const &name, uint64_t cid) : inode_base{ name, -1, cid } {}
bool folder_inode_base::is_file() const noexcept { return false; }
bool folder_inode_base::is_folder() const noexcept { return true; }
bool folder_inode_base::link(tnode *ptr, std::string const &name) { return this->xlink(ptr, name); }
tnode *folder_inode_base::add(inode_base *node) { return this->xadd(node, node->name()); }
bool folder_inode_base::unlink(std::string const &name) { return this->xunlink(name); }
tnode *folder_inode_base::find(std::string const &fname) { return this->xfind(fname); }
uint64_t folder_inode_base::num_files() const noexcept { return this->xgnfiles(); }
uint64_t folder_inode_base::num_folders() const noexcept { return this->xgnfolders(); }
uint64_t folder_inode_base::size() const noexcept { return this->xgnfiles() + this->xgnfolders(); }
bool folder_inode_base::relink(std::string const &oldn, std::string const &newn) { if(tnode* ptr = this->xfind(oldn)) { return this->unlink(oldn) && this->link(ptr, newn); } else return false; } 