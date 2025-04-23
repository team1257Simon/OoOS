#include "fs/fs.hpp"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "stdexcept"
#include "errno.h"
#include "kdebug.hpp"
file_node* get_by_fd(filesystem* fsptr, task_ctx* ctx, int fd) { return (fd < 3) ? ctx->stdio_ptrs[fd] : fsptr->get_fd(fd); }
filesystem::filesystem() : device_nodes{}, current_open_files{}, next_fd{ 3 } {}
filesystem::~filesystem() = default;
void filesystem::__put_fd(file_node* fd) { if(static_cast<size_t>(fd->vid()) >= current_open_files.capacity()) current_open_files.reserve(static_cast<size_t>(fd->vid() + 1)); current_open_files.set_at(fd->vid(), fd); for(std::vector<file_node*>::iterator i = current_open_files.begin() + next_fd; i < current_open_files.end() && *i; i++, next_fd++); }
const char *filesystem::path_separator() const noexcept { return "/"; }
void filesystem::close_file(file_node* fd) { close_fd(fd); fd->rel_lock(); syncdirs(); }
void filesystem::close_fd(file_node* fd) { if(fd->is_device()) return; fd->seek(0); int id = fd->vid(); if(static_cast<size_t>(id) < current_open_files.size()) { current_open_files[id] = nullptr; next_fd = id; } }
file_node* filesystem::open_fd(tnode* node) { return node->as_file(); }
void filesystem::dldevnode(device_node* n) { n->prune_refs(); current_open_files[n->vid()] = nullptr; device_nodes.erase(*n); syncdirs(); }
file_node* filesystem::open_file(const char* path, std::ios_base::openmode mode) { return open_file(std::string(path), mode); }
file_node* filesystem::get_fd(int fd) { if(static_cast<size_t>(fd) < current_open_files.size()) { return current_open_files[fd]; } else return nullptr; }
device_node* filesystem::lndev(std::string const& where, device_node::device_buffer* what, int fd_hint, bool create_parents) { target_pair parent = get_parent(where, create_parents); if(parent.first->find(parent.second)) throw std::logic_error{ "cannot create link " + parent.second + " because it already exists" }; device_node* result = this->mkdevnode(parent.first, parent.second, what, fd_hint); this->__put_fd(result); return result; }
void filesystem::link_stdio(device_node::device_buffer* target) { current_open_files.push_back(mkdevnode(get_root_directory(), "stdin", target, 0)); current_open_files.push_back(mkdevnode(get_root_directory(), "stdout", target, 1)); current_open_files.push_back(this->mkdevnode(this->get_root_directory(), "stderr", target, 2)); }
std::string filesystem::get_path_separator() const noexcept { return std::string(path_separator()); }
tnode* filesystem::link(std::string const& ogpath, std::string const& tgpath, bool create_parents) { return xlink(get_parent(ogpath, false), get_parent(tgpath, create_parents)); }
bool filesystem::unlink(std::string const& what, bool ignore_nonexistent, bool dir_recurse) { target_pair parent = get_parent(what, false); return xunlink(parent.first, parent.second, ignore_nonexistent, dir_recurse); }
dev_t filesystem::get_dev_id() const noexcept { return this->xgdevid(); }
directory_node *filesystem::get_dir_nothrow(std::string const& path, bool create) noexcept { try { return get_dir(path, create); } catch(...) { return nullptr; } }
device_node* filesystem::mkdevnode(directory_node* parent, std::string const& name, device_node::device_buffer* dev, int fd_hint)
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
    if(!(*node)->has_refs()) { if(node->is_file()){ if(node->as_file()->is_device()) { dldevnode(dynamic_cast<device_node*>(node->as_file())); } else dlfilenode(node->as_file()); } if(node->is_directory()) dldirnode(node->as_directory()); }
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
    std::vector<std::string> pathspec = std::ext::split(path, path_separator());
    if(pathspec.empty()) throw std::logic_error{ "empty path" };
    directory_node* node = get_root_directory();
    for(size_t i = 0; i < pathspec.size() - 1; i++)
    {
        if(pathspec[i].empty()) continue;
        tnode* cur = node->find(pathspec[i]);
        if(!cur) 
        {
            if(create) 
            {
                directory_node* created = mkdirnode(node, pathspec[i]);
                cur = node->add(created);
                node = created; 
            } 
            else { throw std::out_of_range{ "path " + pathspec[i] + " does not exist (use get_dir(\".../" + pathspec[i] + "\", true) to create it)" }; } 
        }
        else if(cur->is_directory()) node = cur->as_directory();
        else throw std::invalid_argument{ "path is invalid because entry " + pathspec[i] + " is a file" };
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
    file_node* result = open_fd(node);
    __put_fd(result);
    return result;
}
file_node* filesystem::get_file(std::string const& path)
{
    target_pair parent = get_parent(path, false);
    if(tnode* node{ parent.first->find(parent.second) }) { return open_fd(node); }
    else throw std::runtime_error{ "file not found: " + path };
}
directory_node* filesystem::get_dir(std::string const& path, bool create)
{
    if(path.empty()) return get_root_directory(); // empty path or "/" refers to root directory
    target_pair parent = get_parent(path, create);
    tnode* node = parent.first->find(parent.second);
    if(!node) 
    { 
        if(create) 
        {
            directory_node* cn = mkdirnode(parent.first, parent.second);
            if(!cn) throw std::runtime_error{ "failed to create " + path };
            node = parent.first->add(cn);
            return node->as_directory(); 
        } 
        else throw std::out_of_range{ "path " + path + " does not exist (use get_dir(\"" + path + "\", true) to create it)" }; 
    }
    else if(node->is_file()) throw std::invalid_argument{ "path " + path + " exists and is a file" };
    else return node->as_directory();
}