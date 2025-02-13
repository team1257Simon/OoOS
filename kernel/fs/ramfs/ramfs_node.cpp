#include "fs/ramfs.hpp"
#include "rtc.h"
ramfs_folder_inode::ramfs_folder_inode(std::string const& name) : folder_inode{ name, addr_t(this) } {}
bool ramfs_folder_inode::fsync() { for(tnode_dir::iterator first = __my_dir.begin(), last = __my_dir.end(); first != last; ) { if(!first->operator bool()) first = __my_dir.erase(first); else ++first; } return true; }
std::vector<std::string> ramfs_folder_inode::lsdir() const { std::vector<std::string> result{}; for(tnode_dir::const_iterator i = __my_dir.begin(); i != __my_dir.end(); ++i) result.push_back(i->name()); return result; }
uint64_t ramfs_folder_inode::num_folders() const noexcept { return __my_subdir_cnt; }
uint64_t ramfs_folder_inode::num_files() const noexcept { return __my_file_cnt; }
bool ramfs_folder_inode::unlink(std::string const &what) { bool result = __my_dir.erase(what) != 0; if(result) syscall_time(&this->modif_time); return result; }
tnode* ramfs_folder_inode::add(inode* n) { std::pair<tnode_dir::iterator, bool> result = __my_dir.emplace(n, n->name()); if(result.second) syscall_time(&this->modif_time); return result.first.base(); }
bool ramfs_folder_inode::link(tnode* original, std::string const& alias) { bool result = __my_dir.emplace(mklink(original, alias)).second; if(result) syscall_time(&this->modif_time); return result; }
tnode *ramfs_folder_inode::find(std::string const& name) { tnode_dir::iterator i = __my_dir.find(name); if(i != __my_dir.end()) { return i.base(); } return nullptr; }
ramfs_file_inode::ramfs_file_inode(std::string const& name, int fd) : file_inode{ name, fd, addr_t(this) } {}
ramfs_file_inode::size_type ramfs_file_inode::write(const_pointer src, size_type n) { size_t result = this->sputn(src, n); if(result) syscall_time(&this->modif_time); return result; }
ramfs_file_inode::size_type ramfs_file_inode::read(pointer dest, size_type n) { return this->sgetn(dest, n); }
ramfs_file_inode::pos_type ramfs_file_inode::tell() const { return this->generic_binary_buffer<char>::tell(); }
ramfs_file_inode::pos_type ramfs_file_inode::seek(off_type off, std::ios_base::seekdir way) { return this->seekoff(off, way); }
ramfs_file_inode::pos_type ramfs_file_inode::seek(pos_type pos) { return this->seekpos(pos); }
bool ramfs_file_inode::fsync() { this->__q_on_modify(); return true; }
uint64_t ramfs_file_inode::size() const noexcept { return this->__qsize(); }