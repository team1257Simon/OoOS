#ifndef __RAMFS
#define __RAMFS
#include "kernel/fs/fs.hpp"
#include "ext/dynamic_queue_streambuf.hpp"
constexpr dev_t ramfs_magic = 0xC001;
class ramfs_directory_inode : public directory_node
{
    tnode_dir __my_dir      {};
    size_t __file_count     {};
    size_t __subdir_count   {};
public:
    virtual tnode* find(std::string const& name) override;
    virtual bool link(tnode* original, std::string const& alias) override;
    virtual tnode* add(fs_node*) override;
    virtual bool unlink(std::string const& what) override;
    virtual uint64_t num_files() const noexcept override;
    virtual uint64_t num_subdirs() const noexcept override;
    virtual std::vector<std::string> lsdir() const override;
    virtual size_t readdir(std::vector<tnode*>& out_vec) override;
    virtual uint64_t size() const noexcept override;
    virtual bool truncate() override;
    ramfs_directory_inode(std::string const& name, int fd);
    virtual bool fsync() override;
};
class ramfs_file_inode final : std::ext::dynamic_queue_streambuf<char>, public file_node
{
	using file_node::traits_type;
	using file_node::difference_type;
	using file_node::size_type;
	using file_node::pos_type;
	using file_node::off_type;
	using file_node::pointer;
	using file_node::const_pointer;
public:
    virtual size_type write(const_pointer src, size_type n) override;
    virtual size_type read(pointer dest, size_type n) override;
    virtual pos_type seek(off_type, std::ios_base::seekdir) override;
    virtual pos_type seek(pos_type) override;
    virtual pos_type tell() const;
    ramfs_file_inode(std::string const& name, int fd);
    virtual bool fsync() override;
    virtual uint64_t size() const noexcept override;
    virtual bool truncate() override;
};
class ramfs final : public filesystem
{
    ramfs_directory_inode __root_dir;
    std::set<ramfs_file_inode> __file_nodes;
    std::set<ramfs_directory_inode> __directory_nodes;
protected:
    virtual directory_node* get_root_directory() override;
    virtual void dlfilenode(file_node* fd) override;
    virtual void dldirnode(directory_node* dd) override;
    virtual void syncdirs() override;
    virtual file_node* mkfilenode(directory_node* parent, std::string const& name) override;
    virtual directory_node* mkdirnode(directory_node* parent, std::string const& name) override;
    virtual dev_t xgdevid() const noexcept override;
public:
    ramfs();
};
#endif