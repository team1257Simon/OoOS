#include "fs/fs.hpp"
#include "rtc.h"
fs_node::fs_node(std::string const& name, int vfd, uint64_t cid) : fd{ vfd }, real_id{ cid }, create_time{ sys_time(nullptr) }, modif_time{ create_time }, concrete_name{ name } {}
int fs_node::vid() const noexcept { return fd; }
void fs_node::vid(int id) noexcept { fd = id; }
uint64_t fs_node::cid() const noexcept { return real_id; }
uint64_t fs_node::created_time() const noexcept { return create_time; }
uint64_t fs_node::modified_time() const noexcept { return modif_time; }
bool fs_node::rename(std::string const& n) { concrete_name = n; return true; }
const char *fs_node::name() const { return concrete_name.c_str(); }
bool fs_node::is_file() const noexcept { return false; }
bool fs_node::is_directory() const noexcept { return false; }
bool fs_node::is_device() const noexcept { return false; }
bool fs_node::is_pipe() const noexcept { return false; }
void fs_node::add_reference(tnode* ref) { refs.insert(ref); }
void fs_node::rm_reference(tnode* ref) { if(refs.erase(ref)) { ref->invlnode(); } }
void fs_node::prune_refs() { for(tnode* ref : refs) ref->invlnode(); refs.clear(); }
bool fs_node::has_refs() const noexcept { return refs.size() != 0; }
size_t fs_node::num_refs() const noexcept { return refs.size(); }
fs_node::~fs_node() { prune_refs(); }
file_node::file_node(std::string const& name, int vfd, uint64_t cid) : fs_node{ name, vfd, cid } {}
bool file_node::is_file() const noexcept { return true; }
void file_node::force_write() { /* if applicable, treat all data in the file as dirty; does nothing in the default implementation */ }
directory_node::directory_node(std::string const& name, int vfd, uint64_t cid) : fs_node{ name, vfd, cid } {}
tnode* directory_node::find(std::string const& name) { tnode_dir::iterator i = directory_tnodes.find(name); if(i != directory_tnodes.end()) { return i.base(); } return nullptr; }
tnode* directory_node::find_l(std::string const& what) { return find(what); }
tnode* directory_node::find_r(std::string const& what, std::set<fs_node*>&) { return find(what); /* default implementation for FS without symlinks */ }
std::vector<std::string> directory_node::lsdir() const { std::vector<std::string> result{}; for(tnode const& tn : directory_tnodes) result.push_back(tn.name()); return result; }
size_t directory_node::readdir(std::vector<tnode*>& out_vec) { size_t result = 0UL; for(tnode& tn : directory_tnodes) { out_vec.push_back(std::addressof(tn)); ++result; } return result; }
uint64_t directory_node::num_subdirs() const noexcept { return subdir_count; }
uint64_t directory_node::num_files() const noexcept { return file_count; }
uint64_t directory_node::size() const noexcept { return directory_tnodes.size(); }
bool directory_node::is_directory() const noexcept { return true; }
bool directory_node::is_empty() const noexcept { return size() == 0; }
bool directory_node::relink(std::string const& oldn, std::string const& newn) { if(tnode* ptr = find(oldn)) { return unlink(oldn) && link(ptr, newn); } else return false; } 
device_node::device_node(std::string const& name, int fd, device_stream* dev_buffer, dev_t id) : file_node{ name, fd, reinterpret_cast<uint64_t>(dev_buffer) }, __dev_buffer{ dev_buffer }, __dev_id{ id } { mode = 027666; }
bool device_node::fsync() { return __dev_buffer->pubsync() == 0; }
bool device_node::is_device() const noexcept { return true; }
uint64_t device_node::size() const noexcept { return __dev_buffer->in_avail(); }
bool device_node::truncate() { return true; /* NOP for device nodes */ }
char* device_node::data() { return __dev_buffer->in_data(); }
bool device_node::grow(size_t) { return false; }
device_node::pos_type device_node::tell() const { return __dev_buffer->pubseekoff(0L, std::ios_base::cur); }
device_node::size_type device_node::write(const_pointer src, size_type n) { size_type result = __dev_buffer->sputn(src, n); if(result) sys_time(std::addressof(modif_time)); return result; }
device_node::size_type device_node::read(pointer dest, size_type n) { return __dev_buffer->sgetn(dest, n); }
device_node::pos_type device_node::seek(off_type off, std::ios_base::seekdir way) { return __dev_buffer->pubseekoff(off, way); }
device_node::pos_type device_node::seek(pos_type pos) { return __dev_buffer->pubseekpos(pos); }
tnode::tnode(fs_node* node, std::string const& name) : __my_node{ node }, __my_name{ name } { if(__my_node) __my_node->refs.insert(this); }
tnode::tnode(fs_node* node, const char* name) : __my_node{ node }, __my_name(name) { if(__my_node) __my_node->refs.insert(this); }
tnode::tnode(std::string name) : __my_node{ nullptr }, __my_name{ name } {}
tnode::tnode(const char* name) : __my_node{ nullptr }, __my_name(name) {}
void tnode::rename(std::string const& n) { __my_name = n; }
void tnode::rename(const char* n) { rename(std::string(n, std::strlen(n))); }
const char* tnode::name() const { return __my_name.c_str(); }
fs_node* tnode::ptr() noexcept { return __my_node; }
fs_node const* tnode::ptr() const noexcept { return __my_node; }
fs_node& tnode::ref() noexcept { return *__my_node; }
fs_node const& tnode::ref() const noexcept { return *__my_node; }
fs_node& tnode::operator*() noexcept { return *__my_node; }
fs_node const& tnode::operator*() const noexcept { return *__my_node; }
fs_node* tnode::operator->() noexcept { return __my_node; }
fs_node const* tnode::operator->() const noexcept { return __my_node; }
file_node* tnode::as_file() { return dynamic_cast<file_node*>(__my_node); }
file_node const* tnode::as_file() const { return dynamic_cast<file_node const*>(__my_node); }
directory_node* tnode::as_directory() { return dynamic_cast<directory_node*>(__my_node); }
directory_node const* tnode::as_directory() const { return dynamic_cast<directory_node*>(__my_node); }
device_node* tnode::as_device() { return dynamic_cast<device_node*>(__my_node); }
device_node const* tnode::as_device() const { return dynamic_cast<device_node const *>(__my_node); }
bool tnode::is_file() const { return __my_node && __my_node->is_file(); }
bool tnode::is_directory() const { return __my_node && __my_node->is_directory(); }
bool tnode::is_device() const { return __my_node && __my_node->is_device(); }
bool tnode::is_pipe() const { return __my_node && __my_node->is_pipe(); }
void tnode::invlnode() noexcept { __my_node = nullptr; }
bool tnode::assign(fs_node* n) noexcept { if(!__my_node) { __my_node = n; return true; } else return false; }
tnode mklink(tnode* original, std::string const& name) { return tnode(original->__my_node, name); }
pipe_node::pipe_node(std::string const& name, int vid, size_t id) : file_node(name, vid, 0), __pipe(id) { current_mode = std::ios_base::out; mode.t_regular = false; mode.t_fifo = true; }
pipe_node::pipe_node(std::string const& name, int vid) : file_node(name, vid, 0), __pipe() { current_mode = std::ios_base::in; mode.t_regular = false; mode.t_fifo = true; }
pipe_node::pipe_node(int vid, size_t id) : pipe_node("", vid, id) {}
pipe_node::pipe_node(int vid) : pipe_node("", vid) {}
bool pipe_node::fsync() { return true; }
uint64_t pipe_node::size() const noexcept { return __pipe->size(); }
bool pipe_node::truncate() { return __pipe->truncate(); }
char* pipe_node::data() { return __pipe->data(); }
bool pipe_node::grow(size_t added) { return __pipe->grow(added); }
size_t pipe_node::pipe_id() const { return __pipe.get_id(); }
pipe_node::pos_type pipe_node::tell() const { return __pipe->tell(current_mode); }
pipe_node::size_type pipe_node::write(const_pointer src, size_type n) { return __pipe->sputn(src, n); }
pipe_node::size_type pipe_node::read(pointer dest, size_type n) { return __pipe->sgetn(dest, n); }
pipe_node::pos_type pipe_node::seek(off_type off, std::ios_base::seekdir way) { return __pipe->pubseekoff(off, way, current_mode); }
pipe_node::pos_type pipe_node::seek(pos_type pos) { return __pipe->pubseekpos(pos, current_mode); }
bool pipe_node::is_pipe() const noexcept { return true; }