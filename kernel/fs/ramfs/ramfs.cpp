#include "fs/ramfs.hpp"
#include "stdexcept"
ramfs::ramfs() : filesystem(), __root_dir("", next_fd++), __file_nodes{}, __directory_nodes{} {}
directory_node *ramfs::get_root_directory() { return std::addressof(__root_dir); }
void ramfs::dlfilenode(file_node* fd) { __file_nodes.erase(*fd); }
void ramfs::dldirnode(directory_node* dd) { if(!dd->is_empty()) { throw std::logic_error(std::string("[FS/RAMFS] cannot delete directory ") + dd->name() + " because it is not empty"); } __directory_nodes.erase(*dd); }
file_node* ramfs::mkfilenode(directory_node* parent, std::string const& name) { return __file_nodes.emplace(name, next_fd++).first.base(); }
directory_node* ramfs::mkdirnode(directory_node* parent, std::string const& name) { return __directory_nodes.emplace(name, next_fd++).first.base(); }
void ramfs::syncdirs() { /* nop */ }
dev_t ramfs::xgdevid() const noexcept { return ramfs_magic; }