#include "fs/ramfs.hpp"
ramfs::ramfs() : filesystem(), __root_dir{ "" }, __file_nodes{}, __directory_nodes{} {}
directory_node *ramfs::get_root_directory() { return std::addressof(__root_dir); }
void ramfs::dlfilenode(file_node* fd) { fd->prune_refs(); __file_nodes.erase(*fd); syncdirs(); }
void ramfs::dldirnode(directory_node* dd) { if(!dd->is_empty()) { throw std::logic_error{ std::string{ "cannot delete directory " } + dd->name() + " because it is not empty" }; } dd->prune_refs(); __directory_nodes.erase(*dd); syncdirs(); }
file_node *ramfs::mkfilenode(directory_node *parent, std::string const& name) { return __file_nodes.emplace(name, next_fd++).first.base(); }
directory_node *ramfs::mkdirnode(directory_node *parent, std::string const& name) { return __directory_nodes.emplace(name).first.base(); }
void ramfs::syncdirs() { for(ramfs_directory_inode& folder : __directory_nodes) folder.fsync(); }
dev_t ramfs::xgdevid() const noexcept { return ramfs_magic; }