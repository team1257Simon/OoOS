#include "fs/fs_node.hpp"
bool file_inode_base::is_file() const noexcept { return true; }
bool file_inode_base::is_folder() const noexcept { return false; }
bool file_inode_base::chk_lock() const noexcept { return test_lock(&__my_lock); }
void file_inode_base::acq_lock() { acquire(&__my_lock); }
void file_inode_base::rel_lock() { release(&__my_lock); }
file_inode_base::pos_type file_inode_base::seek(off_type offs, std::ios_base::seekdir way) { return this->xseekl(offs, way); }
file_inode_base::pos_type file_inode_base::seek(pos_type pos) { return this->xseekp(pos); }
file_inode_base::size_type file_inode_base::write(const_pointer src, size_type n) { return this->xwrite(src, n); }
file_inode_base::size_type file_inode_base::read(pointer dest, size_type n) { return this->xread(dest, n); }
tnode_base::tnode_base(inode_base* node, std::string const& name) : __my_node{ node }, __my_name{ name } {}
tnode_base::tnode_base(inode_base *node, const char *name) : __my_node{node}, __my_name{name} {}
void tnode_base::rename(std::string const& n) { __my_name = n; }
void tnode_base::rename(const char* n) { __my_name = n; }
const char *tnode_base::name() const { return __my_name.c_str(); }
inode_base &tnode_base::operator*() noexcept { return *__my_node; }
inode_base const &tnode_base::operator*() const noexcept { return *__my_node; }
inode_base *tnode_base::operator->() noexcept { return __my_node; }
inode_base const *tnode_base::operator->() const noexcept { return __my_node; }
bool tnode_base::if_file(std::function<bool(file_inode_base &)> const &action) { return __my_node->is_file() && action(dynamic_cast<file_inode_base&>(*__my_node)); }
bool tnode_base::if_folder(std::function<bool(folder_inode_base &)> const &action) { return __my_node->is_folder() && action(dynamic_cast<folder_inode_base&>(*__my_node)); }
bool tnode_base::if_file(std::function<bool(file_inode_base const &)> const &action) const { return  __my_node->is_file() && action(dynamic_cast<file_inode_base const&>(*__my_node)); }
bool tnode_base::if_folder(std::function<bool(folder_inode_base const &)> const &action) const { return __my_node->is_folder() && action(dynamic_cast<folder_inode_base const&>(*__my_node)); }
file_inode_base* tnode_base::as_file() { return dynamic_cast<file_inode_base*>(__my_node); }
file_inode_base const* tnode_base::as_file() const { return dynamic_cast<file_inode_base const*>(__my_node); }
folder_inode_base* tnode_base::as_folder() { return dynamic_cast<folder_inode_base*>(__my_node); }
folder_inode_base const* tnode_base::as_folder() const { return dynamic_cast<folder_inode_base*>(__my_node); }
bool folder_inode_base::is_file() const noexcept { return false; }
bool folder_inode_base::is_folder() const noexcept { return true; }
bool folder_inode_base::link(tnode_base *ptr, std::string const &name) { return this->xlink(ptr, name); }
bool folder_inode_base::unlink(std::string const &name) { return this->xunlink(name); }
tnode_base *folder_inode_base::find(std::string const &fname) { return this->xfind(fname); }
uint64_t folder_inode_base::num_files() const noexcept { return this->xgnfiles(); }
uint64_t folder_inode_base::num_folders() const noexcept { return this->xgnfolders(); }
uint64_t folder_inode_base::size() const noexcept { return this->xgnfiles() + this->xgnfolders(); }
bool folder_inode_base::relink(std::string const &oldn, std::string const &newn) { if(tnode_base* ptr = this->xfind(oldn)) { return this->unlink(oldn) && this->link(ptr, newn); } else return false; }