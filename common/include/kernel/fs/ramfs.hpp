#ifndef __RAMFS
#define __RAMFS
// Note that by definition the vnodes for ramfs are also technically inodes.
#include "kernel/fs/fs.hpp"
#include "ext/dynamic_queue_streambuf.hpp"
constexpr dev_t ramfs_magic	= 0x0000C001U;
class ramfs_directory_vnode : public directory_vnode
{
public:
	virtual bool link(tnode* original, std::string const& alias) override;
	virtual bool unlink(std::string const& what) override;
	virtual tnode* add(vnode*) override;
	virtual bool truncate() override;
	ramfs_directory_vnode(std::string const& name, int fd);
	virtual bool fsync() override;
};
class ramfs_file_vnode final : std::ext::dynamic_queue_streambuf<char>, public file_vnode
{
	using file_vnode::traits_type;
	using file_vnode::difference_type;
	using file_vnode::size_type;
	using file_vnode::pos_type;
	using file_vnode::off_type;
	using file_vnode::pointer;
	using file_vnode::const_pointer;
public:
	virtual size_type write(const_pointer src, size_type n) override;
	virtual size_type read(pointer dest, size_type n) override;
	virtual pos_type seek(off_type, std::ios_base::seekdir) override;
	virtual pos_type seek(pos_type) override;
	virtual pos_type tell() const;
	virtual char* data() override;
	ramfs_file_vnode(std::string const& name, int fd);
	virtual bool fsync() override;
	virtual uint64_t size() const noexcept override;
	virtual bool grow(size_t added) override;
	virtual bool truncate() override;
};
class ramfs final : public filesystem
{
	ramfs_directory_vnode __root_dir;
	std::set<ramfs_file_vnode> __file_nodes;
	std::set<ramfs_directory_vnode> __directory_nodes;
protected:
	virtual directory_vnode* get_root_directory() override;
	virtual void dlfilenode(file_vnode* fd) override;
	virtual void dldirnode(directory_vnode* dd) override;
	virtual void syncdirs() override;
	virtual file_vnode* mkfilenode(directory_vnode* parent, std::string const& name) override;
	virtual directory_vnode* mkdirnode(directory_vnode* parent, std::string const& name) override;
	virtual dev_t xgdevid() const noexcept override;
public:
	ramfs();
};
#endif