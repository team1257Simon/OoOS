#include "fs/fs.hpp"
#include "sched/task_ctx.hpp"
#include "algorithm"
#include "stdexcept"
#include "errno.h"
#include "kdebug.hpp"
fd_map::fd_map() : __base(256) {}
file_node* get_by_fd(filesystem* fsptr, task_ctx* ctx, int fd) { return (fd < 3) ? ctx->stdio_ptrs[fd] : fsptr->get_file(fd); }
filesystem::filesystem() : device_nodes{}, current_open_files{}, next_fd{ 3 } {}
filesystem::~filesystem() = default;
std::string filesystem::get_path_separator() const noexcept { return std::string(path_separator()); }
fs_node* filesystem::get_fd_node(int fd) { return current_open_files.find_fd(fd); }
void filesystem::__put_fd(fs_node* node) { next_fd = current_open_files.add_fd(node) + 1; }
const char *filesystem::path_separator() const noexcept { return "/"; }
file_node* filesystem::open_file(const char* path, std::ios_base::openmode mode) { return open_file(std::string(path), mode); }
file_node* filesystem::on_open(tnode* node) { return node->as_file(); }
file_node* filesystem::get_file(int fd) { return dynamic_cast<file_node*>(current_open_files.find_fd(fd)); }
directory_node* filesystem::get_directory(int fd) { return dynamic_cast<directory_node*>(current_open_files.find_fd(fd)); }
tnode* filesystem::link(std::string const& ogpath, std::string const& tgpath, bool create_parents) { return xlink(get_parent(ogpath, false), get_parent(tgpath, create_parents)); }
dev_t filesystem::get_dev_id() const noexcept { return xgdevid(); }
directory_node* filesystem::get_directory_or_null(std::string const& path, bool create) noexcept { try { return get_directory(path, create); } catch(...) { return nullptr; } }
fs_node* fd_map::find_fd(int i) noexcept
{
    iterator result = find(i);
    if(result != end()) return *result;
    return nullptr;
}
int fd_map::add_fd(fs_node* node)
{
    while(contains(node->fd) && find_fd(node->fd) != node) node->fd++;
    insert(node);
    return node->fd;
}
void filesystem::close_file(file_node* fd) 
{ 
    on_close(fd);
    fd->rel_lock();
    syncdirs();
}
void filesystem::on_close(file_node* fd)
{
    if(fd->is_device()) return; 
    fd->seek(0); 
    current_open_files.erase(fd->vid()); 
}
void filesystem::dldevnode(device_node* n)
{
    n->prune_refs();
    current_open_files.erase(n->vid());
    device_nodes.erase(*n); 
    syncdirs(); 
}
device_node* filesystem::lndev(std::string const& where, device_node::device_buffer* what, int fd_hint, bool create_parents)
{
    target_pair parent = get_parent(where, create_parents);
    if(parent.first->find(parent.second)) throw std::logic_error{ "cannot create link " + parent.second + " because it already exists" }; 
    device_node* result = mkdevnode(parent.first, parent.second, what, fd_hint); 
    __put_fd(result);
    return result; 
}
void filesystem::link_stdio(device_node::device_buffer* target)
{
    current_open_files.add_fd(mkdevnode(get_root_directory(), "stdin", target, 0));
    current_open_files.add_fd(mkdevnode(get_root_directory(), "stdout", target, 1));
    current_open_files.add_fd(mkdevnode(get_root_directory(), "stderr", target, 2));
}
bool filesystem::unlink(std::string const& what, bool ignore_nonexistent, bool dir_recurse)
{
    directory_node* pdir;
    std::string fname;
    if(ignore_nonexistent) try 
    { 
        target_pair parent = get_parent(what, false); 
        pdir = parent.first;
        fname = parent.second;
    } 
    catch(...) { return false; }
    else
    {
        target_pair parent = get_parent(what, false); 
        pdir = parent.first;
        fname = parent.second;
    }
    return xunlink(pdir, fname, ignore_nonexistent, dir_recurse);
}
device_node* filesystem::mkdevnode(directory_node* parent, std::string const& name, device_node::device_buffer* dev, int fd_hint)
{
    device_node* result = device_nodes.emplace(name, fd_hint, dev).first.base();
    parent->add(result);
    __put_fd(result);
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
            else { throw std::out_of_range{ "path " + pathspec[i] + " does not exist (use get_directory(\".../" + pathspec[i] + "\", true) to create it)" }; } 
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
    file_node* result = on_open(node);
    __put_fd(result);
    return result;
}
file_node* filesystem::get_file(std::string const& path)
{
    target_pair parent = get_parent(path, false);
    if(tnode* node = parent.first->find(parent.second))
    { 
        file_node* file = on_open(node);
        if(file) __put_fd(file);
        return file;
    }
    else throw std::runtime_error{ "file not found: " + path };
}
directory_node* filesystem::get_directory(std::string const& path, bool create)
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
            __put_fd(cn);
            return node->as_directory(); 
        } 
        else throw std::out_of_range{ "path " + path + " does not exist (use get_directory(\"" + path + "\", true) to create it)" }; 
    }
    else if(node->is_file()) throw std::invalid_argument{ "path " + path + " exists and is a file" };
    else { __put_fd(node->ptr()); return node->as_directory(); }
}