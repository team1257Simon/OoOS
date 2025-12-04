#include <fs/fs.hpp>
#include <sched/task_ctx.hpp>	// uid_undef, gid_undef
#include <rtc.h>
vnode::vnode(std::string const& name, int vfd, uint64_t cid) : fd(vfd), real_id(cid), create_time(sys_time(nullptr)), modif_time(create_time), concrete_name(name) {}
vnode::vnode(int vfd, uint64_t cid) : fd(vfd), real_id(cid), create_time(sys_time(nullptr)), modif_time(create_time), concrete_name() {}
int vnode::vid() const noexcept { return fd; }
void vnode::vid(int id) noexcept { fd = id; }
uint64_t vnode::cid() const noexcept { return real_id; }
uint64_t vnode::created_time() const noexcept { return create_time; }
uint64_t vnode::modified_time() const noexcept { return modif_time; }
bool vnode::rename(std::string const& n) { concrete_name = n; return true; }
const char *vnode::name() const { return concrete_name.c_str(); }
bool vnode::is_file() const noexcept { return false; }
bool vnode::is_directory() const noexcept { return false; }
bool vnode::is_device() const noexcept { return false; }
bool vnode::is_pipe() const noexcept { return false; }
bool vnode::is_mount() const noexcept { return false; }
void vnode::add_reference(tnode* ref) { refs.insert(ref); }
void vnode::rm_reference(tnode* ref) { if(refs.erase(ref)) { ref->invlnode(); } }
void vnode::prune_refs() { for(tnode* ref : refs) ref->invlnode(); refs.clear(); }
bool vnode::has_refs() const noexcept { return refs.size() != 0; }
size_t vnode::num_refs() const noexcept { return refs.size(); }
uid_t vnode::owner_uid() const noexcept { return uid_undef; }
gid_t vnode::owner_gid() const noexcept { return gid_undef; }
vnode::~vnode() { prune_refs(); }
file_vnode::file_vnode(std::string const& name, int vfd, uint64_t cid) : vnode(name, vfd, cid) {}
file_vnode::file_vnode(int vfd, uint64_t cid) : vnode(vfd, cid) {}
file_vnode::~file_vnode() = default;
bool file_vnode::is_file() const noexcept { return true; }
bool file_vnode::on_open() { return true; /* any action, like allocating a buffer, that needs to occur when a file is opened should be implemented via overriding this method */ }
void file_vnode::on_close() { /* any action, like freeing a buffer, that needs to occur when a file is closed should be implemented via overriding this method */ }
void file_vnode::force_write() { /* if applicable, treat all data in the file as dirty; does nothing in the default implementation */ }
directory_vnode::directory_vnode(std::string const& name, int vfd, uint64_t cid) : vnode(name, vfd, cid) {}
directory_vnode::directory_vnode(int vfd, uint64_t cid) : vnode(vfd, cid) {}
directory_vnode::~directory_vnode() = default;
tnode* directory_vnode::find(std::string const& name) { tnode_dir::iterator i = directory_tnodes.find(name); if(i != directory_tnodes.end()) { return i.base(); } return nullptr; }
tnode* directory_vnode::find_l(std::string const& what) { return find(what); }
tnode* directory_vnode::find_r(std::string const& what, std::set<vnode*>&) { return find(what); /* default implementation for FS without symlinks */ }
std::vector<std::string> directory_vnode::lsdir() const { std::vector<std::string> result{}; for(tnode const& tn : directory_tnodes) result.push_back(tn.name()); return result; }
size_t directory_vnode::readdir(std::vector<tnode*>& out_vec) { size_t result = 0UL; for(tnode& tn : directory_tnodes) { out_vec.push_back(std::addressof(tn)); ++result; } return result; }
uint64_t directory_vnode::num_subdirs() const noexcept { return subdir_count; }
uint64_t directory_vnode::num_files() const noexcept { return file_count; }
uint64_t directory_vnode::size() const noexcept { return directory_tnodes.size(); }
bool directory_vnode::is_directory() const noexcept { return true; }
bool directory_vnode::is_empty() const noexcept { return size() == 0; }
bool directory_vnode::relink(std::string const& oldn, std::string const& newn) { if(tnode* ptr = find(oldn)) { return unlink(oldn) && link(ptr, newn); } else return false; }
device_vnode::device_vnode(int fd, device_stream* dev_buffer, dev_t id) : file_vnode(fd, reinterpret_cast<uint64_t>(dev_buffer)), __dev_buffer(dev_buffer), __dev_id(id) { mode = 027666; }
device_vnode::device_vnode(std::string const& name, int fd, device_stream* dev_buffer, dev_t id) : file_vnode(name, fd, reinterpret_cast<uint64_t>(dev_buffer)), __dev_buffer(dev_buffer), __dev_id(id) { mode = 027666; }
device_vnode::~device_vnode() = default;
bool device_vnode::fsync() { return __dev_buffer->sync() == 0; }
bool device_vnode::is_device() const noexcept { return true; }
uint64_t device_vnode::size() const noexcept { return __dev_buffer->avail(); }
bool device_vnode::truncate() { return true; /* NOP for device nodes */ }
char* device_vnode::data() { return nullptr; /* Device nodes may not have proper data pointers */ }
bool device_vnode::grow(size_t) { return false; }
device_vnode::pos_type device_vnode::tell() const { return __dev_buffer->out_avail(); }
device_vnode::size_type device_vnode::write(const_pointer src, size_type n) { size_type result = __dev_buffer->write(n, src); if(result) sys_time(std::addressof(modif_time)); return result; }
device_vnode::size_type device_vnode::read(pointer dest, size_type n) { return __dev_buffer->read(dest, n); }
device_vnode::pos_type device_vnode::seek(off_type off, std::ios_base::seekdir way) { return __dev_buffer->seek(way, off, std::ios_base::out | std::ios_base::in); }
device_vnode::pos_type device_vnode::seek(pos_type pos) { return __dev_buffer->seek(pos, std::ios_base::out | std::ios_base::in); }
tnode::tnode(vnode* node, std::string const& name) : __my_node(node), __my_name(name) { if(__my_node) __my_node->refs.insert(this); }
tnode::tnode(vnode* node, const char* name) : __my_node(node), __my_name(name) { if(__my_node) __my_node->refs.insert(this); }
tnode::tnode(std::string name) : __my_node(nullptr), __my_name(name) {}
tnode::tnode(const char* name) : __my_node(nullptr), __my_name(name) {}
void tnode::rename(std::string const& n) { __my_name = n; }
void tnode::rename(const char* n) { rename(std::string(n, std::strlen(n))); }
const char* tnode::name() const { return __my_name.c_str(); }
vnode* tnode::ptr() noexcept { return __my_node; }
vnode const* tnode::ptr() const noexcept { return __my_node; }
vnode& tnode::ref() noexcept { return *__my_node; }
vnode const& tnode::ref() const noexcept { return *__my_node; }
vnode& tnode::operator*() noexcept { return *__my_node; }
vnode const& tnode::operator*() const noexcept { return *__my_node; }
vnode* tnode::operator->() noexcept { return __my_node; }
vnode const* tnode::operator->() const noexcept { return __my_node; }
file_vnode* tnode::as_file() { return dynamic_cast<file_vnode*>(__my_node); }
file_vnode const* tnode::as_file() const { return dynamic_cast<file_vnode const*>(__my_node); }
directory_vnode* tnode::as_directory() { return dynamic_cast<directory_vnode*>(__my_node); }
directory_vnode const* tnode::as_directory() const { return dynamic_cast<directory_vnode const*>(__my_node); }
device_vnode* tnode::as_device() { return dynamic_cast<device_vnode*>(__my_node); }
device_vnode const* tnode::as_device() const { return dynamic_cast<device_vnode const*>(__my_node); }
mount_vnode* tnode::as_mount() { return dynamic_cast<mount_vnode*>(__my_node); }
mount_vnode const* tnode::as_mount() const { return dynamic_cast<mount_vnode const*>(__my_node); }
bool tnode::is_file() const { return __my_node && __my_node->is_file(); }
bool tnode::is_directory() const { return __my_node && __my_node->is_directory(); }
bool tnode::is_device() const { return __my_node && __my_node->is_device(); }
bool tnode::is_pipe() const { return __my_node && __my_node->is_pipe(); }
bool tnode::is_mount() const { return __my_node && __my_node->is_mount(); }
void tnode::invlnode() noexcept { __my_node = nullptr; }
bool tnode::assign(vnode* n) noexcept { if(!__my_node) { __my_node = n; return true; } else return false; }
tnode mklink(tnode* original, std::string const& name) { return tnode(original->__my_node, name); }
pipe_vnode::pipe_vnode(std::string const& name, int vid, size_t id) : file_vnode(name, vid, 0), __pipe(id) { current_mode = std::ios_base::out; mode.t_regular = false; mode.t_fifo = true; }
pipe_vnode::pipe_vnode(std::string const& name, int vid) : file_vnode(name, vid, 0), __pipe() { current_mode = std::ios_base::in; mode.t_regular = false; mode.t_fifo = true; }
pipe_vnode::pipe_vnode(int vid, size_t id) : pipe_vnode("", vid, id) {}
pipe_vnode::pipe_vnode(int vid) : pipe_vnode("", vid) {}
pipe_vnode::~pipe_vnode() = default;
bool pipe_vnode::fsync() { return true; }
uint64_t pipe_vnode::size() const noexcept { return __pipe->size(); }
bool pipe_vnode::truncate() { return __pipe->truncate(); }
char* pipe_vnode::data() { return __pipe->data(); }
bool pipe_vnode::grow(size_t added) { return __pipe->grow(added); }
size_t pipe_vnode::pipe_id() const { return __pipe.get_id(); }
void pipe_vnode::on_close() { if(current_mode.in) __pipe->readers--; if(current_mode.out) __pipe->writers--; }
bool pipe_vnode::on_open() { if(current_mode.in) __pipe->readers++; if(current_mode.out) __pipe->writers++; return true; }
pipe_vnode::pos_type pipe_vnode::tell() const { return __pipe->tell(current_mode); }
pipe_vnode::size_type pipe_vnode::write(const_pointer src, size_type n) { if(__pipe->readers) return __pipe->sputn(src, n); else throw std::logic_error("[FS] broken pipe"); }
pipe_vnode::size_type pipe_vnode::read(pointer dest, size_type n) { if(__pipe->writers) return __pipe->sgetn(dest, n); else throw std::logic_error("[FS] broken pipe"); }
pipe_vnode::pos_type pipe_vnode::seek(off_type off, std::ios_base::seekdir way) { return __pipe->pubseekoff(off, way, current_mode); }
pipe_vnode::pos_type pipe_vnode::seek(pos_type pos) { return __pipe->pubseekpos(pos, current_mode); }
bool pipe_vnode::is_pipe() const noexcept { return true; }
mount_vnode::mount_vnode(int vfd, std::ext::dynamic_ptr<filesystem>&& fs) : directory_vnode(vfd, fs->get_dev_id()), mounted(std::move(fs)) {}
mount_vnode::~mount_vnode() = default;
bool mount_vnode::is_mount() const noexcept { return true; }
bool mount_vnode::fsync() try { mounted->pubsyncdirs(); return true; } catch(std::exception& e) { panic(e.what()); return false; }