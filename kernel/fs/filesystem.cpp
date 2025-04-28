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
void filesystem::register_fd(fs_node* node) { next_fd = current_open_files.add_fd(node) + 1; }
const char *filesystem::path_separator() const noexcept { return "/"; }
file_node* filesystem::open_file(const char* path, std::ios_base::openmode mode, bool create) { return open_file(std::string(path), mode, create); }
file_node* filesystem::on_open(tnode* node) { return node->as_file(); }
file_node* filesystem::get_file(int fd) { return dynamic_cast<file_node*>(current_open_files.find_fd(fd)); }
directory_node* filesystem::get_directory(int fd) { return dynamic_cast<directory_node*>(current_open_files.find_fd(fd)); }
tnode* filesystem::link(std::string const& ogpath, std::string const& tgpath, bool create_parents) { return xlink(get_parent(ogpath, false), get_parent(tgpath, create_parents)); }
dev_t filesystem::get_dev_id() const noexcept { return xgdevid(); }
void filesystem::pubsyncdirs() { syncdirs(); }
size_t filesystem::block_size() { return physical_block_size; }
filesystem::target_pair filesystem::get_parent(std::string const& path, bool create) { return get_parent(get_root_directory(), path, create); }
directory_node* filesystem::get_directory_or_null(std::string const& path, bool create) noexcept { try { return open_directory(path, create); } catch(...) { return nullptr; } }
fs_node* fd_map::find_fd(int i) noexcept
{
    iterator result = find(i);
    if(result != end()) { return *result; }
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
    if(fd && fd->is_file())
        on_close(fd);
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
device_node* filesystem::lndev(std::string const& where, int fd, dev_t id, bool create_parents)
{
    target_pair parent = get_parent(where, create_parents);
    if(parent.first->find(parent.second)) throw std::logic_error{ "cannot create link " + parent.second + " because it already exists" }; 
    device_node* result = mkdevnode(parent.first, parent.second, id, fd);
    register_fd(result);
    return result; 
}
bool filesystem::link_stdio(dev_t dev_id)
{
    if(!dreg[dev_id]) return false;    
    current_open_files.add_fd(lndev("/dev/stdin", dev_id, 0));
    current_open_files.add_fd(lndev("/dev/stdout", dev_id, 1));
    current_open_files.add_fd(lndev("/dev/stderr", dev_id, 2));
    return true;
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
device_node* filesystem::mkdevnode(directory_node* parent, std::string const& name, dev_t id, int fd)
{
    device_stream* dev = dreg[id];
    if(!dev) { throw std::invalid_argument{"no device found with that id"}; }
    device_node* result = device_nodes.emplace(name, fd, dev, id).first.base();
    parent->add(result);
    register_fd(result);
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
filesystem::target_pair filesystem::get_parent(directory_node* start, std::string const &path, bool create)
{
    std::vector<std::string> pathspec = std::ext::split(path, path_separator());
    for(size_t i = 0; i < pathspec.size() - 1; i++)
    {
        if(pathspec[i].empty()) continue;
        tnode* cur = start->find(pathspec[i]);
        if(!cur) 
        {
            if(create) 
            {
                directory_node* created = mkdirnode(start, pathspec[i]);
                cur = start->add(created);
                start = created; 
            } 
            else { throw std::out_of_range{ "path " + pathspec[i] + " does not exist (use open_directory(\".../" + pathspec[i] + "\", true) to create it)" }; } 
        }
        else if(cur->is_directory()) start = cur->as_directory();
        else throw std::invalid_argument{ "path is invalid because entry " + pathspec[i] + " is a file" };
    }
    return target_pair(std::piecewise_construct, std::forward_as_tuple(start), std::forward_as_tuple(pathspec.back()));
}
fs_node* filesystem::find_node(std::string const& path, bool ignore_links)
{
    try
    {
        target_pair parent = get_parent(path, false);
        tnode* tn = ignore_links ? parent.first->find_l(parent.second) : parent.first->find(parent.second);
        if(!tn) return nullptr;
        if(!current_open_files.contains(tn->ref().vid())) { register_fd(tn->ptr()); }
        if(tn->is_file()) { return on_open(tn); }
        else return tn->ptr();
    }
    catch(std::out_of_range&) { return nullptr; }
}
file_node* filesystem::open_file(std::string const& path, std::ios_base::openmode mode, bool create)
{
    target_pair parent = get_parent(path, false);
    tnode* node = parent.first->find(parent.second);
    if(node && node->is_directory()) throw std::logic_error{ "path " + path + " exists and is a directory" };
    if(!node)
    {
        if(!create) throw std::out_of_range{ "file not found: " + path }; 
        if(file_node* created = mkfilenode(parent.first, parent.second)) { node = parent.first->add(created); }
        else throw std::runtime_error{ "failed to create file: " + path }; 
    }
    file_node* result = on_open(node);
    register_fd(result);
    result->current_mode = mode;
    return result;
}
file_node* filesystem::get_file(std::string const& path)
{
    target_pair parent = get_parent(path, false);
    if(tnode* node = parent.first->find(parent.second))
    { 
        file_node* file = on_open(node);
        if(file) register_fd(file);
        return file;
    }
    else throw std::runtime_error{ "file not found: " + path };
}
directory_node* filesystem::open_directory(std::string const& path, bool create)
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
            register_fd(cn);
            return node->as_directory(); 
        } 
        else throw std::out_of_range{ "path " + path + " does not exist (use open_directory(\"" + path + "\", true) to create it)" }; 
    }
    else if(node->is_file()) throw std::invalid_argument{ "path " + path + " exists and is a file" };
    else { register_fd(node->ptr()); return node->as_directory(); }
}
void filesystem::create_node(directory_node* from, std::string const& path, mode_t mode, dev_t dev)
{
    if(!from) from = get_root_directory();
    target_pair parent = get_parent(from, path, false);
    file_mode m(mode);
    fs_node* result;
    if(m.is_directory()) { result = mkdirnode(parent.first, parent.second); result->mode = mode; }
    else if(m.is_chardev()) { result = mkdevnode(parent.first, parent.second, dev, next_fd++); result->mode = mode; }
    else { result = mkfilenode(parent.first, parent.second); result->mode = mode; }
    if(!result) { throw std::runtime_error{ "failed to create node at " + path }; }
    result->fsync();
}