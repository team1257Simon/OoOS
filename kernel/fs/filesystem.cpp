#include "fs/fs.hpp"
#include "algorithm"
#include "stdexcept"
#include "errno.h"
void filesystem::__put_fd(file_inode *fd) { if(static_cast<size_t>(fd->vid()) >= current_open_files.capacity()) current_open_files.reserve(static_cast<size_t>(fd->vid() + 1)); current_open_files[fd->vid()] = fd; for(std::vector<file_inode*>::iterator i = current_open_files.begin() + next_fd; i < current_open_files.end() && *i; i++, next_fd++); }
const char *filesystem::path_separator() const noexcept { return "/"; }
void filesystem::close_file(file_inode* fd) { this->close_fd(fd); fd->rel_lock(); }
void filesystem::close_fd(file_inode* fd) { fd->seek(0); int id = fd->vid(); if(static_cast<size_t>(id) < current_open_files.size()) { current_open_files[id] = nullptr; next_fd = id; } }
file_inode* filesystem::open_fd(tnode* node) { return node->as_file(); }
void filesystem::dldevnode(device_inode* n) { n->prune_refs(); device_nodes.erase(*n); this->syncdirs(); }
device_inode *filesystem::mkdevnode(folder_inode *parent, std::string const &name, vfs_filebuf_base<char> *dev) 
{
    device_inode* result = std::addressof(*(device_nodes.emplace(name, next_fd++, dev).first));
    parent->add(result);
    this->__put_fd(result);
    return result;
}
bool filesystem::xunlink(folder_inode* parent, std::string const& what, bool ignore_nonexistent, bool dir_recurse)
{
    tnode* node{ parent->find(what) };
    if(!node) { if(!ignore_nonexistent) throw std::logic_error{ "cannot unlink " + what + " because it does not exist" }; else return false; }
    if(node->is_folder() && (*node)->num_refs() <= 1) 
    {
        if(!node->as_folder()->is_empty() && !dir_recurse) throw std::logic_error{ "folder " + what + " cannot be deleted because it is not empty (call with dir_recurse = true to remove it anyway)" };
        if(dir_recurse) for(std::string s : node->as_folder()->lsdir()) this->xunlink(node->as_folder(), s, true, true);
    }
    if(!parent->unlink(what)) return false;
    (*node)->unregister_reference(node);
    if(!(*node)->has_refs())
    {
        if(node->is_file()){ if(node->as_file()->is_device()) { this->dldevnode(dynamic_cast<device_inode*>(node->as_file())); } else this->dlfilenode(node->as_file());  }
        if(node->is_folder()) this->dldirnode(node->as_folder());
    }
    return true;
}
tnode *filesystem::xlink(target_pair ogparent, target_pair tgparent)
{
    tnode* node{ ogparent.first->find(ogparent.second) };
    if(!node) throw std::runtime_error{ std::string{ "path does not exist: " } + ogparent.first->name() + path_separator() + ogparent.second };
    if(tgparent.first->find(tgparent.second)) throw std::logic_error{ std::string{ "target " } + tgparent.first->name() + path_separator() + tgparent.second + " already exists" };
    if(!tgparent.first->link(node, tgparent.second)) throw std::runtime_error{ std::string{ "failed to create link: " } + tgparent.first->name() + path_separator() + tgparent.second };
    return tgparent.first->find(tgparent.second);
}
filesystem::target_pair filesystem::get_parent(std::string const& path, bool create)
{
    std::vector<std::string> pathspec{ std::ext::split(path, this->path_separator()) };
    if(pathspec.empty()) throw std::logic_error{ "empty path" };
    folder_inode* node{ this->get_root_directory() };
    for(size_t i = 0; i < pathspec.size() - 1; i++)
    {
        if(pathspec[i].empty()) continue;
        tnode* cur = node->find(pathspec[i]);
        if(!cur)
        {
            if(create) { cur = node->add(this->mkdirnode(node, pathspec[i])); node = cur->as_folder(); }
            else{ throw std::runtime_error{ "folder " + pathspec[i] + " does not exist (use get_folder(\".../" + pathspec[i] + "\", true) to create it)" };}
        }
        else if(cur->is_folder()) node = cur->as_folder();
        else throw std::logic_error{ "path is invalid because entry " + pathspec[i] + " is a file" };
    }
    return std::make_pair(node, pathspec.back());
}
file_inode* filesystem::open_file(std::string const& path, std::ios_base::openmode mode)
{
    target_pair parent{ this->get_parent(path, false) };
    tnode* node{ parent.first->find(parent.second) };
    if(node && node->is_folder()) throw std::logic_error{ "path " + path + " exists and is a folder" };
    if(!node) 
    {
        if(!mode.out) throw std::runtime_error{ "file not found: " + path }; 
        if(file_inode* created{ mkfilenode(parent.first, parent.second) }) 
        {
            created->mode.read_group = created->mode.read_owner= created->mode.read_others = mode.in;
            created->mode.write_group = created->mode.write_owner = created->mode.write_others = mode.out;
            node = parent.first->add(created);
        }
        else throw std::runtime_error{ "failed to create file: " + path }; 
    }
    if(!node->as_file()->chk_lock()) { throw std::runtime_error{ "file " + path + " is in use" }; }
    node->as_file()->acq_lock();
    file_inode* result = this->open_fd(node);
    this->__put_fd(result);
    return result;
}
folder_inode *filesystem::get_folder(std::string const& path, bool create)
{
    if(path.empty()) return this->get_root_directory(); // empty path or "/" refers to root directory
    target_pair parent{ this->get_parent(path, create) };
    tnode* node{ parent.first->find(parent.second) };
    if(!node)
    {
        if(create) { node = parent.first->add(this->mkdirnode(parent.first, parent.second)); return node->as_folder(); }
        else throw std::runtime_error{ "path " + path + " does not exist (use get_folder(\"" + path + "\", true) to create it)" };
    }
    else if (node->is_file()) throw std::logic_error{ "path " + path + " exists and is a file" };
    else return node->as_folder();
}
file_inode* filesystem::get_fd(int fd) { if(static_cast<size_t>(fd) < current_open_files.size()) return current_open_files[fd]; else return nullptr; }
device_inode* filesystem::lndev(std::string const& where, vfs_filebuf_base<char>* what, bool create_parents) { target_pair parent{ this->get_parent(where, create_parents) }; if(parent.first->find(parent.second)) throw std::logic_error{ "cannot create link " + parent.second + " because it already exists" }; return this->mkdevnode(parent.first, parent.second, what); }
tnode *filesystem::link(std::string const& ogpath, std::string const& tgpath, bool create_parents) { return this->xlink( this->get_parent(ogpath, false), this->get_parent(tgpath, create_parents)); }
bool filesystem::unlink(std::string const &what, bool ignore_nonexistent, bool dir_recurse) { target_pair parent{ this->get_parent(what, false) }; return this->xunlink(parent.first, parent.second, ignore_nonexistent, dir_recurse); }
extern "C"
{
    int syscall_open(char *name, int flags, ...)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return ENOSYS;
        uint8_t smallflags{ static_cast<uint8_t>(flags) };
        try { if(file_inode* n{ fsptr->open_file(name, std::ios_base::openmode(smallflags)) }) return n->vid(); } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_close(int fd)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return ENOSYS;
        try { if(file_inode* n{ fsptr->get_fd(fd) }) { fsptr->close_file(n); return 0; } else return EBADF; } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_write(int fd, char *ptr, int len)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return ENOSYS;
        try { if(file_inode* n{ fsptr->get_fd(fd) }) { n->write(ptr, len); return 0; } else return EBADF; } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_read(int fd, char *ptr, int len)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return ENOSYS;
        try { if(file_inode* n{ fsptr->get_fd(fd) }) { n->read(ptr, len); return 0; } } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_link(char *old, char *__new)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return ENOSYS;
        try { return fsptr->link(old, __new) != nullptr ? 0 : ENOENT; } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_unlink(char *name)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return ENOSYS;
        try { return fsptr->unlink(name) ? 0 : ENOENT; } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_isatty(int fd)
    {
        filesystem* fsptr{ get_fs_instance() };
        if(!fsptr) return 0;
        try { if(file_inode* n{ fsptr->get_fd(fd) }) return n->is_device() ? 1 : 0; } catch(std::exception& e) { panic(e.what()); }
        return 0;
    }
}