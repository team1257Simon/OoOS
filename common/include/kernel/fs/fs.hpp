#ifndef __FILE_SYSTEM
#define __FILE_SYSTEM
#include "fs/fs_node.hpp"
#include "vector"
class fs_base
{
protected:
    typedef std::pair<folder_inode_base*, std::string> target_pair;
    virtual folder_inode_base* get_root_directory() = 0;
    virtual void dlfilenode(file_inode_base*) = 0;
    virtual void dldirnode(folder_inode_base*) = 0;
    virtual file_inode_base* mkfilenode(folder_inode_base* parent, std::string const& name) = 0;
    virtual folder_inode_base* mkdirnode(folder_inode_base* parent, std::string const& name) = 0;
    virtual const char* path_separator() const noexcept;
    virtual file_inode_base* open_fd(tnode*);
    virtual target_pair get_parent(std::string const& path, bool create);
    virtual void close_fd(file_inode_base*);
    virtual bool xunlink(folder_inode_base* parent, std::string const& what, bool ignore_nonexistent, bool dir_recurse);
    virtual tnode* xlink(target_pair ogpath, target_pair tgpath);
public:
    tnode* get_node(std::string const& path, bool create_parents = false);
    file_inode_base* open_file(std::string const& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
    folder_inode_base* get_folder(std::string const& path, bool create = true);
    tnode* link(std::string const& ogpath, std::string const& tgpath, bool create_parents = true);
    void close_file(file_inode_base* fd);
    bool unlink(std::string const& what, bool ignore_nonexistent = true, bool dir_recurse = false);
};
#endif