#include "fs/fs.hpp"
#include "algorithm"
#include "stdexcept"
const char *fs_base::path_separator() const noexcept { return "/"; }
void fs_base::close_file(file_inode_base* fd) { this->close_fd(fd); fd->rel_lock(); }
void fs_base::close_fd(file_inode_base* fd) { fd->seek(0); }
file_inode_base *fs_base::open_fd(tnode* node) { return node->as_file(); }
bool fs_base::xunlink(folder_inode_base *parent, std::string const &what, bool ignore_nonexistent, bool dir_recurse)
{
    tnode* node = parent->find(what);
    if(!node) { if(!ignore_nonexistent) throw std::logic_error{ "cannot unlink " + what + " because it does not exist" }; else return false; }
    if(node->is_folder() && (*node)->num_refs() <= 1) 
    {
        if(!node->as_folder()->is_empty() && !dir_recurse) throw std::logic_error{"folder " + what + " cannot be deleted because it is not empty (call with dir_recurse = true to remove it anyway)" };
        if(dir_recurse) for(std::string s : node->as_folder()->lsdir()) this->xunlink(node->as_folder(), s, true, true);
    }
    if(!parent->unlink(what)) return false;
    (*node)->unregister_reference(node);
    if(!(*node)->has_refs())
    {
        if(node->is_file()) this->dlfilenode(node->as_file());
        if(node->is_folder()) this->dldirnode(node->as_folder());
    }
    return true;
}
tnode *fs_base::xlink(target_pair ogparent, target_pair tgparent)
{
    tnode* node = ogparent.first->find(ogparent.second);
    if(!node) throw std::runtime_error{ std::string{ "path does not exist: " } + ogparent.first->name() + path_separator() + ogparent.second };
    if(tgparent.first->find(tgparent.second)) throw std::logic_error{ std::string{ "target " } + tgparent.first->name() + path_separator() + tgparent.second + " already exists" };
    if(!tgparent.first->link(node, tgparent.second)) throw std::runtime_error{ std::string{ "failed to create link: " } + tgparent.first->name() + path_separator() + tgparent.second };
    return tgparent.first->find(tgparent.second);
}
fs_base::target_pair fs_base::get_parent(std::string const& path, bool create)
{
    std::vector<std::string> pathspec = std::ext::split(path, this->path_separator());
    if(pathspec.empty()) throw std::logic_error{ "empty path" };
    folder_inode_base* node = this->get_root_directory();
    for(size_t i = 0; i < pathspec.size() - 1; i++)
    {
        if(pathspec[i].empty()) continue;
        tnode* cur = node->find(pathspec[i]);
        if(!cur)
        {
            if(create) { cur = node->add(this->mkdirnode(node, pathspec[i])); node = cur->as_folder(); }
            else throw std::runtime_error{ "folder " + pathspec[i] + " does not exist (use get_folder(\".../" + pathspec[i] + "\", true) to create it)" };
        }
        else if(cur->is_folder()) node = cur->as_folder();
        else throw std::logic_error{ "path is invalid because entry " + pathspec[i] + " is a file" };
    }
    return std::make_pair(node, pathspec.back());
}
tnode *fs_base::get_node(std::string const& path, bool create_parents)
{
    target_pair parent = this->get_parent(path, create_parents);
    if(parent.first) { return parent.first->find(parent.second); }
    else throw std::runtime_error{ "no such path: " + path };
}
file_inode_base* fs_base::open_file(std::string const& path, std::ios_base::openmode mode)
{
    target_pair parent = this->get_parent(path, false);
    tnode* node = parent.first->find(parent.second);
    if(node && node->is_folder()) throw std::logic_error{ "path " + path + " exists and is a folder" };
    if(!node) { if(!mode.out) throw std::runtime_error{ "file not found: " + path }; if(file_inode_base* created = mkfilenode(parent.first, parent.second)) node = parent.first->add(created); else throw std::runtime_error{ "failed to create file: " + path }; }
    if(!node->as_file()->chk_lock()) { throw std::runtime_error{ "file " + path + " is in use" }; }
    node->as_file()->acq_lock();
    return this->open_fd(node);
}
folder_inode_base *fs_base::get_folder(std::string const& path, bool create)
{
    if(path.empty()) return this->get_root_directory(); // empty path or "/" refers to root directory
    target_pair parent = this->get_parent(path, create);
    tnode* node = parent.first->find(parent.second);
    if(!node)
    {
        if(create) { node = parent.first->add(this->mkdirnode(parent.first, parent.second)); return node->as_folder(); }
        else throw std::runtime_error{ "path " + path + " does not exist (use get_folder(\"" + path + "\", true) to create it)" };
    }
    else if (node->is_file()) throw std::logic_error{ "path " + path + " exists and is a file" };
    else return node->as_folder();
}
tnode *fs_base::link(std::string const& ogpath, std::string const& tgpath, bool create_parents) { return this->xlink( this->get_parent(ogpath, false), this->get_parent(tgpath, create_parents)); }
bool fs_base::unlink(std::string const &what, bool ignore_nonexistent, bool dir_recurse) { target_pair parent = this->get_parent(what, false); return this->xunlink(parent.first, parent.second, ignore_nonexistent, dir_recurse); }