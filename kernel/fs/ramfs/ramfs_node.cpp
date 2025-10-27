#include "fs/ramfs.hpp"
#include "rtc.h"
ramfs_directory_node::ramfs_directory_node(std::string const& name, int fd) : directory_node(name, fd, addr_t(this)) {}
bool ramfs_directory_node::fsync() { /* nop */ return true; }
bool ramfs_directory_node::truncate() { directory_tnodes.clear(); file_count = 0; subdir_count = 0; return true; }
bool ramfs_directory_node::unlink(std::string const& what) { bool result = directory_tnodes.erase(what) != 0; if(result) sys_time(std::addressof(modif_time)); return result; }
tnode* ramfs_directory_node::add(fs_node* n) { std::pair<tnode_dir::iterator, bool> result = directory_tnodes.emplace(n, n->name()); if(result.second) sys_time(std::addressof(modif_time)); return result.first.base(); }
bool ramfs_directory_node::link(tnode* original, std::string const& alias) { bool result = directory_tnodes.emplace(mklink(original, alias)).second; if(result) sys_time(std::addressof(modif_time)); return result; }
ramfs_file_node::ramfs_file_node(std::string const& name, int fd) : file_node(name, fd, addr_t(this)) {}
ramfs_file_node::size_type ramfs_file_node::write(const_pointer src, size_type n) { size_t result = sputn(src, n); if(result) sys_time(std::addressof(modif_time)); return result; }
ramfs_file_node::size_type ramfs_file_node::read(pointer dest, size_type n) { return sgetn(dest, n); }
char* ramfs_file_node::data() { return __qbeg(); }
ramfs_file_node::pos_type ramfs_file_node::tell() const { return __tell(); }
ramfs_file_node::pos_type ramfs_file_node::seek(off_type off, std::ios_base::seekdir way) { return seekoff(off, way); }
ramfs_file_node::pos_type ramfs_file_node::seek(pos_type pos) { return seekpos(pos); }
bool ramfs_file_node::fsync() { on_modify_queue(); return true; }
uint64_t ramfs_file_node::size() const noexcept { return __qsize(); }
bool ramfs_file_node::grow(size_t added) { if(!__q_grow_buffer(added)) return false; on_modify_queue(); return true; }
bool ramfs_file_node::truncate() { __qclear(); return true; }