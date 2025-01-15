#ifndef __FILE_SYSTEM
#define __FILE_SYSTEM
#include "fs/fs_node.hpp"
#include "vector"
class fs_base
{
    folder_inode_base* __traverse(std::vector<std::string> const& pathspec);
protected:
    virtual const char* path_separator() const noexcept;
    virtual folder_inode_base* get_root_directory() = 0;
    virtual void dlfilenode(file_inode_base*) = 0;
    virtual void dldirnode(folder_inode_base*) = 0;
    virtual file_inode_base* open_fd(tnode*);
    virtual void close_fd(file_inode_base*);
    virtual file_inode_base* mkfilenode(folder_inode_base* parent, std::string const& name) = 0;
    virtual folder_inode_base* mkdirnode(folder_inode_base* parent, std::string const& name) = 0;
public:
    virtual tnode* get_node(std::string const& path);
    virtual file_inode_base* open_file(std::string const& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
    virtual folder_inode_base* get_folder(std::string const& path, bool create = true);
    virtual void close_file(file_inode_base* fd);
};
#endif