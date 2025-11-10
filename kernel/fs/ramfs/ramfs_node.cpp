#include "fs/ramfs.hpp"
#include "rtc.h"
ramfs_directory_vnode::ramfs_directory_vnode(std::string const& name, int fd) : directory_vnode(name, fd, addr_t(this)) {}
bool ramfs_directory_vnode::fsync() { /* nop */ return true; }
bool ramfs_directory_vnode::truncate() { directory_tnodes.clear(); file_count = 0; subdir_count = 0; return true; }
bool ramfs_directory_vnode::unlink(std::string const& what) { bool result = directory_tnodes.erase(what) != 0; if(result) sys_time(std::addressof(modif_time)); return result; }
tnode* ramfs_directory_vnode::add(vnode* n) { std::pair<tnode_dir::iterator, bool> result = directory_tnodes.emplace(n, n->name()); if(result.second) sys_time(std::addressof(modif_time)); return result.first.base(); }
bool ramfs_directory_vnode::link(tnode* original, std::string const& alias) { bool result = directory_tnodes.emplace(mklink(original, alias)).second; if(result) sys_time(std::addressof(modif_time)); return result; }
ramfs_file_vnode::ramfs_file_vnode(std::string const& name, int fd) : file_vnode(name, fd, addr_t(this)) {}
ramfs_file_vnode::size_type ramfs_file_vnode::write(const_pointer src, size_type n) { size_t result = sputn(src, n); if(result) sys_time(std::addressof(modif_time)); return result; }
ramfs_file_vnode::size_type ramfs_file_vnode::read(pointer dest, size_type n) { return sgetn(dest, n); }
char* ramfs_file_vnode::data() { return __qbeg(); }
ramfs_file_vnode::pos_type ramfs_file_vnode::tell() const { return __tell(); }
ramfs_file_vnode::pos_type ramfs_file_vnode::seek(off_type off, std::ios_base::seekdir way) { return seekoff(off, way); }
ramfs_file_vnode::pos_type ramfs_file_vnode::seek(pos_type pos) { return seekpos(pos); }
bool ramfs_file_vnode::fsync() { on_modify_queue(); return true; }
uint64_t ramfs_file_vnode::size() const noexcept { return __qsize(); }
bool ramfs_file_vnode::grow(size_t added) { if(!__q_grow_buffer(added)) return false; on_modify_queue(); return true; }
bool ramfs_file_vnode::truncate() { __qclear(); return true; }