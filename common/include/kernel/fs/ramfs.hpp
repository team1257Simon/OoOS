#ifndef __RAMFS
#define __RAMFS
#include "fs/fs.hpp"
#include "fs/generic_binary_buffer.hpp"
class ramfs_folder_inode : public folder_inode
{
    tnode_dir __my_dir{};
    size_t __my_file_cnt{};
    size_t __my_subdir_cnt{};
public:
    virtual tnode* find(std::string const& name) override;
    virtual bool link(tnode* original, std::string const& alias) override;
    virtual tnode* add(inode*) override;
    virtual bool unlink(std::string const& what) override;
    virtual uint64_t num_files() const noexcept override;
    virtual uint64_t num_folders() const noexcept override;
    virtual std::vector<std::string> lsdir() const;
    ramfs_folder_inode(std::string const& name);
    virtual bool fsync() override;
};
class ramfs_file_inode final : generic_binary_buffer<char>, public file_inode
{
	using file_inode::traits_type;
	using file_inode::difference_type;
	using file_inode::size_type;
	using file_inode::pos_type;
	using file_inode::off_type;
	using file_inode::pointer;
	using file_inode::const_pointer;
public:
    virtual size_type write(const_pointer src, size_type n) override;
    virtual size_type read(pointer dest, size_type n) override;
    virtual pos_type seek(off_type, std::ios_base::seekdir) override;
    virtual pos_type seek(pos_type) override;
    ramfs_file_inode(std::string const& name, int fd);
    virtual bool fsync() override;
    virtual uint64_t size() const noexcept override;
};
class ramfs final : public filesystem
{
    ramfs_folder_inode __root_dir{ "" };
    std::set<ramfs_file_inode> __file_nodes{};
    std::set<ramfs_folder_inode> __folder_nodes{};
protected:
    virtual folder_inode* get_root_directory() override;
    virtual void dlfilenode(file_inode* fd) override;
    virtual void dldirnode(folder_inode* dd) override;
    virtual void syncdirs() override;
    virtual file_inode* mkfilenode(folder_inode* parent, std::string const& name) override;
    virtual folder_inode* mkdirnode(folder_inode* parent, std::string const& name) override;
public:
    ramfs() = default;
};
#endif