#include "fs/fs.hpp"
#include "algorithm"
#include "stdexcept"
const char *fs_base::path_separator() const noexcept { return "/"; }
void fs_base::close_file(file_inode_base* fd) { this->close_fd(fd); fd->rel_lock(); }
void fs_base::close_fd(file_inode_base*) {}
file_inode_base *fs_base::open_fd(tnode* n) { return n->as_file(); }
folder_inode_base *fs_base::__traverse(std::vector<std::string> const &pathspec)
{
    if(pathspec.empty()) throw std::runtime_error{ "empty path" };
    folder_inode_base* n = this->get_root_directory();
    std::vector<std::string>::const_iterator ed = (pathspec.end()--);
    for(std::vector<std::string>::const_iterator i = pathspec.begin(); i != ed; i++) { tnode* c = n->find(*i); if(!(c && c->if_folder([&](folder_inode_base& next) -> bool { n = std::addressof(next); return true; }))) return nullptr; }
    return n;
}
tnode *fs_base::get_node(std::string const &path)
{
    std::vector<std::string> pathspec = std::ext::split(path, this->path_separator());
    if(folder_inode_base* n = __traverse(pathspec)) { return n->find(path.back()); }
    else throw std::runtime_error{ "no such path: " + path };
}
file_inode_base* fs_base::open_file(std::string const &path, std::ios_base::openmode mode)
{
    std::vector<std::string> pathspec = std::ext::split(path, this->path_separator());
    folder_inode_base* n = __traverse(pathspec);
    if(!n) throw std::runtime_error{ "no such path: " + path };
    tnode* f = n->find(path.back());
    if(f && f->is_folder()) throw std::runtime_error{ "path " + path + " is a directory" };
    if(!f) { if(!mode.out) throw std::runtime_error{ "file not found: " + path }; if(file_inode_base* created = mkfilenode(n, pathspec.back())) f = n->add(created); else throw std::runtime_error{ "failed to create file: " + path }; }
    if(!f->as_file()->chk_lock()) { throw std::runtime_error("file " + path + " is in use"); }
    f->as_file()->acq_lock();
    return this->open_fd(f);
}