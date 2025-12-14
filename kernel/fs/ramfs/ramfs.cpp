#include <fs/ramfs.hpp>
#include <stdexcept>
static directory_vnode& ensure_empty(directory_vnode* dd) { if(dd->is_empty()) return *dd; throw std::logic_error(std::string("[FS/RAMFS] cannot delete directory ") + dd->name() + " because it is not empty"); }
ramfs::ramfs() : default_device_impl_fs(), __root_dir("", next_fd++), __file_nodes(), __directory_nodes() {}
ramfs::~ramfs() = default;
directory_vnode *ramfs::get_root_directory() { return std::addressof(__root_dir); }
void ramfs::dlfilenode(file_vnode* fd) { __file_nodes.erase(*fd); }
void ramfs::dldirnode(directory_vnode* dd) { __directory_nodes.erase(ensure_empty(dd)); }
file_vnode* ramfs::mkfilenode(directory_vnode* parent, std::string const& name) { return __file_nodes.emplace(name, next_fd++).first.base(); }
directory_vnode* ramfs::mkdirnode(directory_vnode* parent, std::string const& name) { return __directory_nodes.emplace(name, next_fd++).first.base(); }
void ramfs::syncdirs() { /* nop */ }
dev_t ramfs::xgdevid() const noexcept { return ramfs_magic; }