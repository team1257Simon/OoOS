#ifndef __FILE_SYSTEM
#define __FILE_SYSTEM
/*
 * The base filesystem vnode structs (vnode, file_vnode and directory_vnode) are the abstract base from which the specific filesystem's vnodes will extend.
 * There is a special node type for devices (such as serial ports) that should work with any sort of filesystem.
 * Similarly, the abstract filesystem class is essentially the vfs interface; any concrete implementor is the actual file system.
*/
#include <string>
#include <functional>
#include <arch/keyboard_stdin.hpp>
#include <bits/ios_base.hpp>
#include <bits/hash_set.hpp>
#include <vector>
#include <set>
#include <fs/simplex_pipe.hpp>
#include <fs/block_device.hpp>
#include <device_registry.hpp>
#include <ext/dynamic_streambuf.hpp>
#include <ext/delegate_ptr.hpp>
#include <ext/dynamic_ptr.hpp>
#include <sys/stat.h>
#include <sys/fcntl.h>
typedef std::ext::delegate_ptr<simplex_pipe> pipe_handle;
struct user_info;
enum class permission_set : int
{
	OTHERS	= 0,
	GROUP	= 3,
	OWNER	= 6
};
enum permission_check : uint8_t
{
	CHK_NONE	= 0UC,
	CHK_EXECUTE	= 0b001UC,
	CHK_READ	= 0b010UC,
	CHK_RX		= CHK_READ	| CHK_EXECUTE,
	CHK_WRITE	= 0b100UC,
	CHK_WX		= CHK_WRITE	| CHK_EXECUTE,
	CHK_RW		= CHK_READ	| CHK_WRITE,
	CHK_RWX		= CHK_READ	| CHK_WRITE	| CHK_EXECUTE,
};
struct file_mode
{
	bool exec_others	: 1;
	bool write_others	: 1;
	bool read_others	: 1;
	bool exec_group		: 1;
	bool write_group	: 1;
	bool read_group		: 1;
	bool exec_owner		: 1;
	bool write_owner	: 1;
	bool read_owner		: 1;
	bool sticky			: 1;
	bool set_gid		: 1;
	bool set_uid		: 1;
	bool t_fifo			: 1;
	bool t_chardev		: 1;
	bool t_directory	: 1;
	bool t_regular		: 1;
	constexpr file_mode(uint16_t i) noexcept :
		exec_others		{ NZ(i & 0000001) },
		write_others	{ NZ(i & 0000002) },
		read_others		{ NZ(i & 0000004) },
		exec_group		{ NZ(i & 0000010) },
		write_group		{ NZ(i & 0000020) },
		read_group		{ NZ(i & 0000040) },
		exec_owner		{ NZ(i & 0000100) },
		write_owner		{ NZ(i & 0000200) },
		read_owner		{ NZ(i & 0000400) },
		sticky			{ NZ(i & 0001000) },
		set_gid			{ NZ(i & 0002000) },
		set_uid			{ NZ(i & 0004000) },
		t_fifo			{ NZ(i & 0010000) },
		t_chardev		{ NZ(i & 0020000) },
		t_directory		{ NZ(i & 0040000) },
		t_regular		{ NZ(i & 0100000) }
						{}
	constexpr operator uint16_t() const noexcept
	{
		if consteval
		{
			return uint16_t
			(
				(exec_others	? 0000001U : 0) |
				(write_others	? 0000002U : 0) |
				(read_others	? 0000004U : 0) |
				(exec_group		? 0000010U : 0) |
				(write_group	? 0000020U : 0) |
				(read_group		? 0000040U : 0) |
				(exec_owner		? 0000100U : 0) |
				(write_owner	? 0000200U : 0) |
				(read_owner		? 0000400U : 0) |
				(sticky			? 0001000U : 0) |
				(set_gid		? 0002000U : 0) |
				(set_uid		? 0004000U : 0) |
				(t_fifo			? 0010000U : 0) |
				(t_chardev		? 0020000U : 0) |
				(t_directory	? 0040000U : 0) |
				(t_regular		? 0100000U : 0)
			);
		}
		else { return std::bit_cast<uint16_t>(*this); }
	}
	constexpr operator mode_t() const noexcept { return static_cast<mode_t>(static_cast<uint16_t>(*this)); }
	constexpr operator mode_t&() & noexcept { return addr_t(this).deref<mode_t>(); }
	constexpr bool is_symlink() const noexcept { return t_regular && t_chardev && !t_fifo && !t_directory; }
	constexpr bool is_socket() const noexcept { return t_regular && t_directory && !t_fifo && !t_chardev; }
	constexpr bool is_blockdev() const noexcept { return t_chardev && t_directory && !t_regular && !t_fifo; }
	constexpr bool is_directory() const noexcept { return t_directory && !t_regular && !t_chardev && !t_fifo; }
	constexpr bool is_chardev() const noexcept { return t_chardev && !t_regular && !t_directory && !t_fifo; }
	constexpr bool is_regular() const noexcept { return t_regular && !t_directory && !t_chardev && !t_fifo; }
	constexpr bool is_fifo() const noexcept { return t_fifo && !t_regular && !t_directory && !t_chardev; }
	constexpr bool is_type_invalid() const noexcept { return (t_fifo && (t_directory || t_chardev || t_regular)) || (t_directory + t_chardev + t_regular) > 2; }
	constexpr uint8_t permission_bits(permission_set which) const noexcept { return static_cast<uint8_t>((static_cast<uint16_t>(*this) >> static_cast<int>(which)) & 07UC); }
} __pack;
struct disk_block { uint64_t block_number; char* data_buffer; bool dirty = false; size_t chain_len = 1U; };
class tnode;
struct vnode
{
	virtual int vid() const noexcept;					// virtual ID (fd number); this is a temporary identifier for persistent filesystems
	virtual void vid(int) noexcept;						// change the FD number for a cached node
	virtual uint64_t cid() const noexcept;				// concrete id; this can be a start sector or anything persistent if applicable (ramfs just gives a self-pointer)
	virtual uint64_t created_time() const noexcept;		// time created
	virtual uint64_t modified_time() const noexcept;	// time last modified
	virtual bool rename(std::string const&);			// change the concrete (i.e. on-disk for persistent fs) name
	virtual const char* name() const;					// get the concrete (i.e. on-disk for persistent fs) name; this may be an empty string (ext, for instance, only uses names for tnodes)
	virtual bool is_file() const noexcept;				// equivalent to (dynamic_cast<file_vnode*>(this) != nullptr)
	virtual bool is_directory() const noexcept;			// equivalent to (dynamic_cast<directory_vnode*>(this) != nullptr)
	virtual bool is_device() const noexcept;			// equivalent to (dynamic_cast<device_vnode*>(this) != nullptr)
	virtual bool is_pipe() const noexcept;				// equivalent to (dynamic_cast<pipe_vnode*>(this) != nullptr)
	virtual bool is_mount() const noexcept;				// equivalent to (dynamic_cast<mount_vnode*>(this) != nullptr)
	virtual uint64_t size() const noexcept = 0;			// size in bytes (for files) or concrete entries (for directories)
	virtual bool fsync() = 0;							// sync to disc, if applicable
	virtual bool truncate() = 0;						// clear all data and reset the size to 0
	virtual uid_t owner_uid() const noexcept;
	virtual gid_t owner_gid() const noexcept;
	virtual ~vnode();
	file_mode mode = 0774US;
	std::set<tnode*> refs{};
	int fd;
	uint64_t real_id;
	uint64_t create_time;
	uint64_t modif_time;
	std::string concrete_name;
	vnode(int vfd, uint64_t cid);
	vnode(std::string const& name, int vfd, uint64_t cid);
	vnode(vnode const&) = delete;
	vnode& operator=(vnode const&) = delete;
	vnode(vnode&&) = default;
	vnode& operator=(vnode&&) = default;
	void add_reference(tnode* ref);						// records the pointed tnode as referencing this node (also calls add_referencing_tnode)
	void rm_reference(tnode* ref);						// removes the record of the pointed tnode from the reference list (also calls remove_referencing_tnode)
	void prune_refs();									// equivalent to calling rm_reference on every node in the reference list; used by filesystems like fat32 that do not support hard links
	bool has_refs() const noexcept;						// equivalent to (num_refs() != 0)
	size_t num_refs() const noexcept;					// the number of tnodes that link to the filesystem object represented by this node
	bool check_permissions(user_info const& user, permission_check what);	// true if and only if the user has all specified permissions to this object
	friend class tnode;
	friend constexpr std::strong_ordering operator<=>(vnode const& a, vnode const& b) noexcept { return a.real_id <=> b.real_id; }
	friend constexpr std::strong_ordering operator<=>(vnode const& a, uint64_t b) noexcept { return a.real_id <=> b; }
	friend constexpr std::strong_ordering operator<=>(uint64_t a, vnode const& b) noexcept { return a <=> b.real_id; }
};
class fd_map : public std::hash_set<vnode*, int, cast_t<int, uint64_t>, equals_t, std::allocator<vnode*>, access_t<vnode, int, &vnode::fd>>
{
	using __base = std::hash_set<value_type, key_type, hasher, key_equal, allocator_type, __key_extract>;
public:
	fd_map();
	vnode* find_fd(int i) noexcept;
	int add_fd(vnode* node);
};
class file_vnode : public vnode
{
public:
	typedef std::char_traits<char>										traits_type;
	typedef decltype(std::declval<char*>() - std::declval<char*>())		difference_type;
	typedef decltype(sizeof(char))										size_type;
	typedef typename traits_type::pos_type								pos_type;
	typedef typename traits_type::off_type								off_type;
	typedef typename std::__impl::__buf_ptrs<char>::__pointer			pointer;
	typedef typename std::__impl::__buf_ptrs<char>::__const_pointer		const_pointer;
	std::ios_base::openmode current_mode					= (std::ios_base::in | std::ios_base::out);
	virtual size_type write(const_pointer src, size_type n)	= 0;
	virtual size_type read(pointer dest, size_type n)		= 0;
	virtual pos_type seek(off_type, std::ios_base::seekdir)	= 0;
	virtual pos_type seek(pos_type)							= 0;
	virtual pos_type tell() const							= 0;
	virtual char* data()									= 0;
	virtual bool grow(size_t)								= 0;
	virtual bool is_file() const noexcept final override;
	virtual void force_write();
	virtual void on_close();
	virtual bool on_open();
	file_vnode(std::string const& name, int vfd, uint64_t cid);
	file_vnode(int vfd, uint64_t cid);
	virtual ~file_vnode();
};
class directory_vnode;
class device_vnode;
struct mount_vnode;
class tnode
{
	vnode* __my_node;
	std::string __my_name;
public:
	tnode(vnode*, std::string const&);
	tnode(vnode*, const char*);
	tnode(std::string);
	tnode(const char*);
	void rename(std::string const&);
	void rename(const char*);
	const char* name() const;
	vnode* ptr() noexcept;
	vnode const* ptr() const noexcept;
	vnode& ref() noexcept;
	vnode const& ref() const noexcept;
	vnode& operator*() noexcept;
	vnode const& operator*() const noexcept;
	vnode* operator->() noexcept;
	vnode const* operator->() const noexcept;
	bool is_file() const;
	bool is_directory() const;
	bool is_mount() const;
	bool is_device() const;
	bool is_pipe() const;
	file_vnode* as_file();
	file_vnode const* as_file() const;
	directory_vnode* as_directory();
	directory_vnode const* as_directory() const;
	device_vnode* as_device();
	device_vnode const* as_device() const;
	mount_vnode* as_mount();
	mount_vnode const* as_mount() const;
	void invlnode() noexcept;
	bool assign(vnode* n) noexcept;
	bool check_permissions(user_info const& user, permission_check what);
	friend tnode mklink(tnode* original, std::string const& name);
	constexpr operator bool() const noexcept { return bool(__my_node); }
	friend constexpr bool operator==(tnode const& __this, tnode const& __that) { return __this.__my_name == __that.__my_name && __this.__my_node == __that.__my_node; }
	friend constexpr std::strong_ordering operator<=>(tnode const& __this, tnode const& __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.__my_name.compare(__that.__my_name)); }
	friend constexpr std::strong_ordering operator<=>(tnode const& __this, std::string const& __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.__my_name.compare(__that)); }
	friend constexpr std::strong_ordering operator<=>(std::string const& __this, tnode const&	__that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.compare(__that.__my_name)); }
};
typedef std::set<tnode> tnode_dir;
struct directory_vnode : public vnode
{
	tnode_dir directory_tnodes;
	size_t file_count;
	size_t subdir_count;
	virtual bool link(tnode*, std::string const&)	= 0;
	virtual bool unlink(std::string const&)			= 0;
	virtual tnode* add(vnode*)						= 0;
	virtual tnode* find(std::string const&);
	virtual tnode* find_l(std::string const&);
	virtual tnode* find_r(std::string const&, std::set<vnode*>&);
	virtual uint64_t num_files() const noexcept;
	virtual uint64_t num_subdirs() const noexcept;
	virtual std::vector<std::string> lsdir() const;
	virtual size_t readdir(std::vector<tnode*>& out_vec);
	virtual bool is_directory() const noexcept final override;
	virtual uint64_t size() const noexcept override;
	virtual bool is_empty() const noexcept;
	virtual bool relink(std::string const& oldn, std::string const& newn);
	directory_vnode(std::string const& name, int vfd, uint64_t cid);
	directory_vnode(int vfd, uint64_t cid);
	virtual ~directory_vnode();
};
class device_vnode : public file_vnode
{
	using file_vnode::traits_type;
	using file_vnode::difference_type;
	using file_vnode::size_type;
	using file_vnode::pos_type;
	using file_vnode::off_type;
	using file_vnode::pointer;
	using file_vnode::const_pointer;
	device_stream* __dev_buffer;
	dev_t __dev_id;
public:
	virtual size_type write(const_pointer src, size_type n) override;
	virtual size_type read(pointer dest, size_type n) override;
	virtual pos_type seek(off_type, std::ios_base::seekdir) override;
	virtual pos_type seek(pos_type) override;
	virtual pos_type tell() const override;
	virtual bool fsync() override;
	virtual bool is_device() const noexcept final override;
	virtual uint64_t size() const noexcept override;
	virtual bool truncate() override;
	virtual char* data() override;
	virtual bool grow(size_t) override;
	virtual bool on_open() override;
	device_vnode(std::string const& name, int fd, device_stream* dev_buffer, dev_t id);
	device_vnode(int fd, device_stream* dev_buffer, dev_t id);
	virtual ~device_vnode();
	constexpr dev_t get_device_id() const noexcept { return __dev_id; }
};
class pipe_vnode : public virtual file_vnode
{
	using file_vnode::traits_type;
	using file_vnode::difference_type;
	using file_vnode::size_type;
	using file_vnode::pos_type;
	using file_vnode::off_type;
	using file_vnode::pointer;
	using file_vnode::const_pointer;
	pipe_handle __pipe;
public:
	virtual size_type write(const_pointer src, size_type n) override;
	virtual size_type read(pointer dest, size_type n) override;
	virtual pos_type seek(off_type, std::ios_base::seekdir) override;
	virtual pos_type seek(pos_type) override;
	virtual pos_type tell() const override;
	virtual bool fsync() override;
	virtual bool is_pipe() const noexcept final override;
	virtual uint64_t size() const noexcept override;
	virtual bool truncate() override;
	virtual char* data() override;
	virtual bool grow(size_t) override;
	virtual bool on_open() override;
	virtual void on_close() override;
	size_t pipe_id() const;
	pipe_vnode(std::string const& name, int vid, size_t cid);
	pipe_vnode(std::string const& name, int vid);
	pipe_vnode(int vid, size_t cid);
	pipe_vnode(int vid);
	virtual ~pipe_vnode();
};
typedef std::hash_set<pipe_vnode, int, cast_t<int, uint64_t>, equals_t, std::allocator<pipe_vnode>, access_t<vnode, int, &vnode::fd>> pipe_map;
struct pipe_pair { pipe_vnode* in; pipe_vnode* out; };
class filesystem
{
protected:
	typedef std::pair<directory_vnode*, std::string> target_pair;
	pipe_map pipes;
	fd_map current_open_files;
	int next_fd;
	block_device* blockdev;
public:
	virtual void dlfilenode(file_vnode*)																= 0;
	virtual void dldevnode(device_vnode*)																= 0;
	virtual void dldirnode(directory_vnode*)															= 0;
	virtual file_vnode* mkfilenode(directory_vnode*, std::string const&)								= 0;
	virtual directory_vnode* mkdirnode(directory_vnode*, std::string const&)							= 0;
	virtual device_vnode* mkdevnode(directory_vnode* parent, std::string const& name, dev_t id, int fd)	= 0;
	virtual void syncdirs()																				= 0;
	virtual dev_t xgdevid() const noexcept																= 0;
	virtual directory_vnode* get_root_directory()														= 0;
	virtual pipe_pair mkpipe(directory_vnode* parent, std::string const& name);
	virtual pipe_pair mkpipe();
	virtual void on_close(file_vnode*);
	virtual bool xunlink(directory_vnode* parent, std::string const& what, bool ignore_nonexistent, bool dir_recurse);
	virtual tnode* xlink(target_pair ogpath, target_pair tgpath);
	virtual target_pair get_parent(directory_vnode* node, std::string const& path, bool create);
	virtual void dlpipenode(vnode*);
	void register_fd(vnode* node);
	target_pair get_parent(std::string const& path, bool create);
	virtual const char* path_separator() const noexcept;
	virtual device_vnode* lndev(std::string const& where, int fd, dev_t id, bool create_parents = true);
	virtual file_vnode* on_open(tnode*);
	virtual file_vnode* on_open(tnode*, std::ios_base::openmode);
	virtual file_vnode* open_file(std::string const& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, bool create = true);
	virtual size_t block_size();
	virtual directory_vnode* open_directory(std::string const& path, bool create = true);
	filesystem();
	virtual ~filesystem();
	bool write_blockdev(uint64_t lba_dest, const void* src, size_t sectors);
	bool read_blockdev(void* dest, uint64_t lba_src, size_t sectors);
	void tie_block_device(block_device* dev);
	device_vnode* tie_char_device(std::string const& where, ooos::keyboard_interface const& dev, int fd = 0, bool create_parents = true);
	ooos::keyboard_stdin* get_stdin_backend(ooos::keyboard_interface const& dev);
	vnode* find_node(std::string const& path, bool ignore_links = false, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
	void create_node(directory_vnode* parent, std::string const& path, mode_t mode, dev_t dev = 0U);
	void create_pipe(int fds[2]);
	vnode* get_fd_node(int fd);
	file_vnode* get_file(int fd);
	file_vnode* open_file(const char* path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out, bool create = true);
	file_vnode* get_file(std::string const& path);
	file_vnode* get_file_or_null(std::string const& path);
	directory_vnode* get_directory(int fd);
	directory_vnode* get_directory_or_null(std::string const& path, bool create = true) noexcept;
	void close_file(file_vnode* fd);
	dev_t get_dev_id() const noexcept;
	std::string get_path_separator() const noexcept;
	tnode* link(std::string const& ogpath, std::string const& tgpath, bool create_parents = true);
	bool unlink(std::string const& what, bool ignore_nonexistent = true, bool dir_recurse = false);
	void pubsyncdirs();
};
class default_device_impl_fs : public filesystem
{
protected:
	std::set<device_vnode> device_nodes{};
	virtual device_vnode* mkdevnode(directory_vnode* parent, std::string const& name, dev_t id, int fd);
	virtual void dldevnode(device_vnode*);
public:
	default_device_impl_fs();
	virtual ~default_device_impl_fs();
};
struct mount_vnode : public directory_vnode
{
	std::ext::dynamic_ptr<filesystem> mounted;
	mount_vnode(int vfd, std::ext::dynamic_ptr<filesystem>&& fs);
	virtual bool is_mount() const noexcept final override;
	virtual bool fsync() override;
	virtual ~mount_vnode();
};
filesystem* create_task_vfs();
filesystem* get_task_vfs();
extern "C"
{
	int syscall_open(char* name, int flags, ...);							// int open(char* name, int flags, ...);
	int syscall_close(int fd);												// int close(int fd);
	int syscall_write(int fd, char* ptr, int len);							// int write(int fd, char* ptr, int len)
	int syscall_read(int fd, char* ptr, int len);							// int read(int fd, char* ptr, int len);
	int syscall_lseek(int fd, long offs, int way);							// int lseek(int fd, off_t offs, int way);
	int syscall_link(char* restrict old, char* restrict __new);				// int link(char* restrict src, char* restrict target);
	int syscall_unlink(char* name);											// int unlink(char* name);
	int syscall_isatty(int fd);												// int isatty(int fd);
	int syscall_fstat(int fd, struct stat* st);								// int fstat(int fd, struct stat* st);
	int syscall_stat(const char* restrict name, struct stat* restrict st);	// int stat(const char* restrict name, struct stat* restrict st);
	int syscall_fchmod(int fd, mode_t m);									// int fchmod(int fd, mode_t m);
	int syscall_chmod(const char* name, mode_t m);							// int chmod(const char* name, mode_t m);
	int syscall_mkdir(const char* path, mode_t m);							// int mkdir(const char* path, mode_t m);
	int syscall_lstat(const char* restrict name, struct stat* restrict st); // int lstat(const char* restrict name, struct stat* restrict st);
	int syscall_mknod(const char* name, mode_t mode, dev_t dev);			// int mknod(const char* name, mode_t mode, dev_t dev);
	int syscall_mknodat(int fd, const char* name, mode_t mode, dev_t dev);	// int mknodat(int fd, const char* name, mode_t mode, dev_t dev);
	int syscall_pipe(int* out);												// int pipe(int out[2]);
}
#endif