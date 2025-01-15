#include "fs/fs_node.hpp"
#include "rtc.h"
inode_base::inode_base(std::string const &name, int vfd, uint64_t cid) : real_name{ name }, fd{ vfd }, real_id{ cid }, create_time{ syscall_time(0) }, modif_time{ create_time } {}
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
tnode::tnode(inode_base* node, std::string const& name) : __my_node{ node }, __my_name{ name } { __my_node->refs.insert(this); }
tnode::tnode(inode_base* node, const char *name) : __my_node{ node }, __my_name{ name } { __my_node->refs.insert(this); }
void tnode::rename(std::string const& n) { __my_name = n; }
void tnode::rename(const char* n) { __my_name = n; }
const char *tnode::name() const { return __my_name.c_str(); }
inode_base &tnode::operator*() noexcept { return *__my_node; }
inode_base const &tnode::operator*() const noexcept { return *__my_node; }
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
uint64_t folder_inode_base::size() const noexcept { return this->num_files() + this->num_folders(); }
bool folder_inode_base::relink(std::string const &oldn, std::string const &newn) { if(tnode* ptr = this->find(oldn)) { return this->unlink(oldn) && this->link(ptr, newn); } else return false; } 