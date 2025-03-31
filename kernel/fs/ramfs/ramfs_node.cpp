#include "fs/ramfs.hpp"
#include "rtc.h"
ramfs_directory_inode::ramfs_directory_inode(std::string const& name) : directory_node{ name, addr_t(this) } {}
bool ramfs_directory_inode::fsync() { /* nop */ return true; }
std::vector<std::string> ramfs_directory_inode::lsdir() const { std::vector<std::string> result{}; for(tnode_dir::const_iterator i = __my_dir.begin(); i != __my_dir.end(); ++i) result.push_back(i->name()); return result; }
uint64_t ramfs_directory_inode::num_subdirs() const noexcept { return __subdir_count; }
uint64_t ramfs_directory_inode::num_files() const noexcept { return __file_count; }
bool ramfs_directory_inode::unlink(std::string const& what) { bool result = __my_dir.erase(what) != 0; if(result) sys_time(std::addressof(modif_time)); return result; }
tnode* ramfs_directory_inode::add(fs_node* n) { std::pair<tnode_dir::iterator, bool> result = __my_dir.emplace(n, n->name()); if(result.second) sys_time(std::addressof(modif_time)); return result.first.base(); }
bool ramfs_directory_inode::link(tnode* original, std::string const& alias) { bool result = __my_dir.emplace(mklink(original, alias)).second; if(result) sys_time(std::addressof(modif_time)); return result; }
tnode *ramfs_directory_inode::find(std::string const& name) { tnode_dir::iterator i = __my_dir.find(name); if(i != __my_dir.end()) { return i.base(); } return nullptr; }
ramfs_file_inode::ramfs_file_inode(std::string const& name, int fd) : file_node{ name, fd, addr_t(this) } {}
ramfs_file_inode::size_type ramfs_file_inode::write(const_pointer src, size_type n) { size_t result = sputn(src, n); if(result) sys_time(std::addressof(modif_time)); return result; }
ramfs_file_inode::size_type ramfs_file_inode::read(pointer dest, size_type n) { return sgetn(dest, n); }
ramfs_file_inode::pos_type ramfs_file_inode::tell() const { return std::ext::dynamic_queue_streambuf<char>::tell(); }
ramfs_file_inode::pos_type ramfs_file_inode::seek(off_type off, std::ios_base::seekdir way) { return seekoff(off, way); }
ramfs_file_inode::pos_type ramfs_file_inode::seek(pos_type pos) { return seekpos(pos); }
bool ramfs_file_inode::fsync() { on_modify_queue(); return true; }
uint64_t ramfs_file_inode::size() const noexcept { return __qsize(); }