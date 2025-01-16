#include "fs/ramfs.hpp"
folder_inode_base *ramfs::get_root_directory() { return &__root_dir; }
void ramfs::dlfilenode(file_inode_base *fd)
{
    fd->prune_refs();
    __file_nodes.erase(*fd);
    for(ramfs_folder_inode& folder : __folder_nodes) folder.fsync();
}
void ramfs::dldirnode(folder_inode_base *dd)
{
    if(!dd->is_empty()) { throw std::logic_error{ std::string{ "cannot delete folder " } + dd->name() + " because it is not empty" }; }
    dd->prune_refs();
    __folder_nodes.erase(*dd);
    for(ramfs_folder_inode& folder : __folder_nodes) folder.fsync();
}
file_inode_base *ramfs::mkfilenode(folder_inode_base *parent, std::string const &name) { return std::addressof(*(__file_nodes.emplace(name, static_cast<int>(__file_nodes.size())).first)); }
folder_inode_base *ramfs::mkdirnode(folder_inode_base *parent, std::string const &name) { return std::addressof(*(__folder_nodes.emplace(name).first)); }