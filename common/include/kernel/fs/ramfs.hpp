#ifndef __RAMFS
#define __RAMFS
#include "kernel/fs/fs.hpp"
#include "ext/dynamic_queue_streambuf.hpp"
constexpr dev_t ramfs_magic = 0xC001;
class ramfs_directory_node : public directory_node
{
public:
    virtual bool link(tnode* original, std::string const& alias) override;
    virtual bool unlink(std::string const& what) override;
    virtual tnode* add(fs_node*) override;
    virtual bool truncate() override;
    ramfs_directory_node(std::string const& name, int fd);
    virtual bool fsync() override;
};
class ramfs_file_node final : std::ext::dynamic_queue_streambuf<char>, public file_node
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
    virtual char* data() override;
    ramfs_file_node(std::string const& name, int fd);
    virtual bool fsync() override;
    virtual uint64_t size() const noexcept override;
    virtual bool grow(size_t added) override;
    virtual bool truncate() override;
};
class ramfs final : public filesystem
{
    ramfs_directory_node __root_dir;
    std::set<ramfs_file_node> __file_nodes;
    std::set<ramfs_directory_node> __directory_nodes;
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