#include "fs/fs.hpp"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "stdexcept"
#include "errno.h"
#include "kdebug.hpp"
static inline timespec timestamp_to_timespec(time_t ts) { return { ts / 1000U, static_cast<long>(ts % 1000U) * 1000000L }; }
file_node* get_by_fd(filesystem* fsptr, task_ctx* ctx, int fd) { return (fd < 3) ? ctx->stdio_ptrs[fd] : fsptr->get_fd(fd); }
filesystem::filesystem() : device_nodes{}, current_open_files{}, next_fd{ 3 } {}
filesystem::~filesystem() = default;
void filesystem::__put_fd(file_node* fd) { if(static_cast<size_t>(fd->vid()) >= current_open_files.capacity()) current_open_files.reserve(static_cast<size_t>(fd->vid() + 1)); current_open_files.set_at(fd->vid(), fd); for(std::vector<file_node*>::iterator i = current_open_files.begin() + next_fd; i < current_open_files.end() && *i; i++, next_fd++); }
const char *filesystem::path_separator() const noexcept { return "/"; }
void filesystem::close_file(file_node* fd) { this->close_fd(fd); fd->rel_lock(); this->syncdirs(); }
void filesystem::close_fd(file_node* fd) { if(fd->is_device()) return; fd->seek(0); int id = fd->vid(); if(static_cast<size_t>(id) < current_open_files.size()) { current_open_files[id] = nullptr; next_fd = id; } }
file_node* filesystem::open_fd(tnode* node) { return node->as_file(); }
void filesystem::dldevnode(device_node* n) { n->prune_refs(); current_open_files[n->vid()] = nullptr; device_nodes.erase(*n); this->syncdirs(); }
file_node *filesystem::open_file(const char* path, std::ios_base::openmode mode) { return open_file(std::string(path), mode); }
file_node* filesystem::get_fd(int fd) { if(static_cast<size_t>(fd) < current_open_files.size()) { return current_open_files[fd]; } else return nullptr; }
device_node* filesystem::lndev(std::string const& where, vfs_filebuf_base<char>* what, int fd_hint, bool create_parents) { target_pair parent = this->get_parent(where, create_parents); if(parent.first->find(parent.second)) throw std::logic_error{ "cannot create link " + parent.second + " because it already exists" }; device_node* result = this->mkdevnode(parent.first, parent.second, what, fd_hint); this->__put_fd(result); return result; }
void filesystem::link_stdio(vfs_filebuf_base<char>* target) { current_open_files.push_back(this->mkdevnode(this->get_root_directory(), "stdin", target, 0)); current_open_files.push_back(this->mkdevnode(this->get_root_directory(), "stdout", target, 1)); current_open_files.push_back(this->mkdevnode(this->get_root_directory(), "stderr", target, 2)); }
tnode* filesystem::link(std::string const& ogpath, std::string const& tgpath, bool create_parents) { return this->xlink(this->get_parent(ogpath, false), this->get_parent(tgpath, create_parents)); }
bool filesystem::unlink(std::string const& what, bool ignore_nonexistent, bool dir_recurse) { target_pair parent = this->get_parent(what, false); return this->xunlink(parent.first, parent.second, ignore_nonexistent, dir_recurse); }
dev_t filesystem::get_dev_id() const noexcept { return this->xgdevid(); }
device_node* filesystem::mkdevnode(directory_node *parent, std::string const& name, vfs_filebuf_base<char>* dev, int fd_hint)
{
    device_node* result = device_nodes.emplace(name, fd_hint, dev).first.base();
    parent->add(result);
    while(current_open_files.size() > static_cast<size_t>(fd_hint) && current_open_files[fd_hint]) fd_hint++;
    result->fd = fd_hint;
    return result;
}
bool filesystem::xunlink(directory_node* parent, std::string const& what, bool ignore_nonexistent, bool dir_recurse)
{
    tnode* node = parent->find(what);
    if(!node) { if(!ignore_nonexistent) throw std::logic_error{ "cannot unlink " + what + " because it does not exist" }; else return false; }
    if(node->is_directory() && (*node)->num_refs() <= 1) { if(!node->as_directory()->is_empty() && !dir_recurse) throw std::logic_error{ "folder " + what + " cannot be deleted because it is not empty (call with dir_recurse = true to remove it anyway)" }; if(dir_recurse) for(std::string s : node->as_directory()->lsdir()) this->xunlink(node->as_directory(), s, true, true); }
    if(!parent->unlink(what)) return false;
    (*node)->rm_reference(node);
    if(!(*node)->has_refs()) { if(node->is_file()){ if(node->as_file()->is_device()) { this->dldevnode(dynamic_cast<device_node*>(node->as_file())); } else this->dlfilenode(node->as_file()); } if(node->is_directory()) this->dldirnode(node->as_directory()); }
    return true;
}
tnode* filesystem::xlink(target_pair ogparent, target_pair tgparent)
{
    tnode* node = ogparent.first->find(ogparent.second);
    if(!node) throw std::runtime_error{ std::string{ "path does not exist: " } + ogparent.first->name() + path_separator() + ogparent.second };
    if(tgparent.first->find(tgparent.second)) throw std::logic_error{ std::string{ "target " } + tgparent.first->name() + path_separator() + tgparent.second + " already exists" };
    if(!tgparent.first->link(node, tgparent.second)) throw std::runtime_error{ std::string{ "failed to create link: " } + tgparent.first->name() + path_separator() + tgparent.second };
    return tgparent.first->find(tgparent.second);
}
filesystem::target_pair filesystem::get_parent(std::string const& path, bool create)
{
    std::vector<std::string> pathspec = std::ext::split(path, this->path_separator());
    if(pathspec.empty()) throw std::logic_error{ "empty path" };
    directory_node* node = this->get_root_directory();
    for(size_t i = 0; i < pathspec.size() - 1; i++)
    {
        if(pathspec[i].empty()) continue;
        tnode* cur = node->find(pathspec[i]);
        if(!cur) 
        { 
            if(create) 
            {
                directory_node* created = this->mkdirnode(node, pathspec[i]);
                cur = node->add(created);
                node = created; 
            } 
            else { throw std::logic_error{ "path " + pathspec[i] + " does not exist (use get_dir(\".../" + pathspec[i] + "\", true) to create it)" }; } 
        }
        else if(cur->is_directory()) node = cur->as_directory();
        else throw std::logic_error{ "path is invalid because entry " + pathspec[i] + " is a file" };
    }
    return target_pair(std::piecewise_construct, std::forward_as_tuple(node), std::forward_as_tuple(pathspec.back()));
}
file_node* filesystem::open_file(std::string const& path, std::ios_base::openmode mode)
{
    target_pair parent = get_parent(path, false);
    tnode* node = parent.first->find(parent.second);
    if(node && node->is_directory()) throw std::logic_error{ "path " + path + " exists and is a directory" };
    if(!node) 
    {
        if(!mode.out) throw std::runtime_error{ "file not found: " + path }; 
        if(file_node* created = mkfilenode(parent.first, parent.second)) 
        {
            created->mode.read_group = created->mode.read_owner = created->mode.read_others = mode.in;
            created->mode.write_group = created->mode.write_owner = created->mode.write_others = mode.out;
            node = parent.first->add(created);
        }
        else throw std::runtime_error{ "failed to create file: " + path }; 
    }
    if(!node->as_file()->chk_lock()) { throw std::runtime_error{ "file " + path + " is in use" }; }
    node->as_file()->acq_lock();
    file_node* result = this->open_fd(node);
    this->__put_fd(result);
    return result;
}
file_node* filesystem::get_file(std::string const& path)
{
    target_pair parent = this->get_parent(path, false);
    if(tnode* node{ parent.first->find(parent.second) }) { return open_fd(node); }
    else throw std::runtime_error{ "file not found: " + path };
}
directory_node* filesystem::get_dir(std::string const& path, bool create)
{
    if(path.empty()) return this->get_root_directory(); // empty path or "/" refers to root directory
    target_pair parent = this->get_parent(path, create);
    tnode* node = parent.first->find(parent.second);
    if(!node) 
    { 
        if(create) 
        {
            directory_node* cn = this->mkdirnode(parent.first, parent.second);
            if(!cn) throw std::runtime_error{ "failed to create " + path };
            node = parent.first->add(cn);
            return node->as_directory(); 
        } 
        else throw std::logic_error{ "path " + path + " does not exist (use get_dir(\"" + path + "\", true) to create it)" }; 
    }
    else if (node->is_file()) throw std::logic_error{ "path " + path + " exists and is a file" };
    else return node->as_directory();
}
static inline void __stat_init(fs_node* n, filesystem* fsptr, stat* st) 
{
    new (translate_user_pointer(st)) stat
    { 
        .st_dev = fsptr->get_dev_id(), 
        .st_ino = n->cid(), 
        .st_mode = n->mode, 
        .st_nlink = n->num_refs(), 
        .st_uid = 0,
        .st_gid = 0, 
        .st_rdev = n->is_device() ? 1U : 0, 
        .st_size = static_cast<long>(n->size()),
        .st_atim = timestamp_to_timespec(n->create_time),
        .st_mtim = timestamp_to_timespec(n->modif_time), 
        .st_ctim = timestamp_to_timespec(n->modif_time), 
        .st_blksize = 512,
        .st_blocks = div_roundup(n->size(), 512) 
    };  
}
extern "C"
{
    int syscall_open(char* name, int flags, ...)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        uint8_t smallflags{ static_cast<uint8_t>(flags) };
        try { if(file_node* n{ fsptr->open_file(static_cast<const char*>(translate_user_pointer(name)), std::ios_base::openmode(smallflags)) }) return n->vid(); } catch(std::exception& e) { panic(e.what()); return -ENOENT; }
        return -EINVAL;
    }
    int syscall_close(int fd)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { fsptr->close_file(n); return 0; } else return EBADF; } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_write(int fd, char* ptr, int len)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { n->write(translate_user_pointer(ptr), len); n->fsync(); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_read(int fd, char* ptr, int len)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { n->read(translate_user_pointer(ptr), len); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_link(char* old, char* __new)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        try { return fsptr->link(old, __new) != nullptr ? 0 : -ENOENT; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_lseek(int fd, long offs, int way) 
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { return n->seek(offs, way == 0 ? std::ios_base::beg : way == 1 ? std::ios_base::cur : std::ios_base::end) >= 0 ? 0 : -EIO; } return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_unlink(char* name)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { return fsptr->unlink(static_cast<const char*>(translate_user_pointer(name))) ? 0 : -ENOENT; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_isatty(int fd)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) return n->is_device() ? 1 : 0; else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_fstat(int fd, stat* st)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        try{ if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { __stat_init(n, fsptr, st); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_stat(const char* restrict name, stat* restrict st)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        name = translate_user_pointer(name);
        try{ if(file_node* n = fsptr->get_file(name)) { __stat_init(n, fsptr, st); return 0; } else if(directory_node* n{ fsptr->get_dir(name, false) }) { __stat_init(n, fsptr, st); return 0; } } catch(std::logic_error& e) { panic(e.what()); return -ENOTDIR; } catch(std::runtime_error& e) { panic(e.what()); return -ENOENT; }
        return -EINVAL;
    }
    int syscall_fchmod(int fd, mode_t m)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        try{ if(file_node* n{ get_by_fd(fsptr, current_active_task()->self, fd) }) { n->mode = m; return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_chmod(const char* name, mode_t m)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return -ENOSYS;
        name = translate_user_pointer(name);
        try { if(file_node* n = fsptr->get_file(name)) { n->mode = m; return 0; } else return -EISDIR; } catch(std::exception& e) { panic(e.what()); return -ENOENT; }
        return -EINVAL;
    }
}