#ifndef __FILE_SYSTEM
#define __FILE_SYSTEM
/*
 * The base filesystem node structs (inode, file_node and directory_node) are essentially vnodes.
 * There is a special node type for devices (such as serial ports) that should work with any sort of filesystem.
 * Similarly, the abstract filesystem class is essentially the vfs; any concrete implementor is the actual file system.
*/
#include "string"
#include "functional"
#include "bits/ios_base.hpp"
#include "vector"
#include "set"
#include "kernel/fs/vfs_filebuf_base.hpp"
#include "sys/stat.h"
struct file_mode
{
    bool exec_others  : 1;
    bool write_others : 1;
    bool read_others  : 1;
    bool              : 1;
    bool exec_group   : 1;
    bool write_group  : 1;
    bool read_group   : 1;
    bool              : 1;
    bool exec_owner   : 1;
    bool write_owner  : 1;
    bool read_owner   : 1;
    bool              : 1;
    bool is_sticky    : 1;
    bool is_gid       : 1;
    bool is_uid       : 1;
    bool              : 1;
    bool is_fifo      : 1;
    bool is_chardev   : 1;
    bool is_dir       : 1;
    bool              : 1;
    bool reg          : 1;
    bool              : 3;
    uint8_t           : 8;
    constexpr file_mode(uint32_t i) noexcept : 
        exec_others     { NZ(i & 0x000001) }, 
        write_others    { NZ(i & 0x000002) }, 
        read_others     { NZ(i & 0x000004) },
        exec_group      { NZ(i & 0x000010) }, 
        write_group     { NZ(i & 0x000020) },
        read_group      { NZ(i & 0x000040) },
        exec_owner      { NZ(i & 0x000100) },
        write_owner     { NZ(i & 0x000200) },
        read_owner      { NZ(i & 0x000400) },
        is_sticky       { NZ(i & 0x001000) },
        is_gid          { NZ(i & 0x002000) },
        is_uid          { NZ(i & 0x004000) },
        is_fifo         { NZ(i & 0x010000) },
        is_chardev      { NZ(i & 0x020000) },
        is_dir          { NZ(i & 0x040000) },
        reg             { NZ(i & 0x100000) }
                        {}
    constexpr operator uint32_t() const noexcept 
    {
        return uint32_t
        (
            (exec_others    ? 0x000001U : 0) |
            (write_others   ? 0x000002U : 0) |
            (read_others    ? 0x000004U : 0) |
            (exec_group     ? 0x000010U : 0) |
            (write_group    ? 0x000020U : 0) |
            (read_group     ? 0x000040U : 0) |
            (exec_owner     ? 0x000100U : 0) |
            (write_owner    ? 0x000200U : 0) |
            (read_owner     ? 0x000400U : 0) |
            (is_sticky      ? 0x001000U : 0) |
            (is_gid         ? 0x002000U : 0) |
            (is_uid         ? 0x004000U : 0) |
            (is_fifo        ? 0x010000U : 0) |
            (is_chardev     ? 0x020000U : 0) |
            (is_dir         ? 0x040000U : 0) |
            (reg            ? 0x100000U : 0)
        );
    }
};
class tnode;
struct fs_node
{
    virtual int vid() const noexcept; // virtual ID (fd number); this is a temporary identifier for persistent filesystems
    virtual void vid(int) noexcept; // change the FD number for a cached node
    virtual uint64_t cid() const noexcept; // concrete id; this can be a start sector or anything persistent if applicable (ramfs just gives a self-pointer)
    virtual uint64_t created_time() const noexcept; // time created
    virtual uint64_t modified_time() const noexcept; // time last modified
    virtual bool rename(std::string const&); // change the concrete (i.e. on-disk for persistent fs) name
    virtual const char* name() const; // get the concrete (i.e. on-disk for persistent fs) name
    virtual bool is_file() const noexcept;
    virtual bool is_folder() const noexcept;
    virtual bool is_device() const noexcept;
    virtual uint64_t size() const noexcept = 0; // size in bytes (for files) or concrete entries (for folders)
    virtual bool fsync() = 0; // Sync to disc, if applicable
    virtual ~fs_node();
    int fd;
    file_mode mode{ 0x0774U };   
    uint64_t real_id;
    uint64_t create_time;
    uint64_t modif_time;
    std::string concrete_name;
    std::set<tnode*> refs{};
    fs_node(std::string const& name, int vfd, uint64_t cid);
    // Move-assign and move-construct only; no copying allowed
    fs_node(fs_node const&) = delete;
    fs_node& operator=(fs_node const&) = delete;
    fs_node(fs_node&&) = default;
    fs_node& operator=(fs_node&&) = default;
    void unregister_reference(tnode* ref);
    void prune_refs();
    bool has_refs() const noexcept;
    size_t num_refs() const noexcept;
    friend class tnode;
    friend constexpr std::strong_ordering operator<=>(fs_node const& a, fs_node const& b) noexcept { return a.real_id <=> b.real_id; }
    friend constexpr std::strong_ordering operator<=>(fs_node const& a, uint64_t b) noexcept { return a.real_id <=> b; }
    friend constexpr std::strong_ordering operator<=>(uint64_t a, fs_node const& b) noexcept { return a <=> b.real_id; }
};
class file_node : public fs_node
{
    spinlock_t __my_lock{};
public:
    typedef std::char_traits<char>                                      traits_type;
    typedef decltype(std::declval<char*>() - std::declval<char*>())     difference_type;
    typedef decltype(sizeof(char))                                      size_type;
    typedef typename traits_type::pos_type                              pos_type;
    typedef typename traits_type::off_type                              off_type;
    typedef typename std::__impl::__buf_ptrs<char>::__ptr_type          pointer;
    typedef typename std::__impl::__buf_ptrs<char>::__const_ptr_type    const_pointer;
    virtual size_type write(const_pointer src, size_type n) = 0;
    virtual size_type read(pointer dest, size_type n) = 0;
    virtual pos_type seek(off_type, std::ios_base::seekdir) = 0;
    virtual pos_type seek(pos_type) = 0;
    virtual pos_type tell() const = 0;
    file_node(std::string const& name, int vfd, uint64_t cid);
    virtual bool is_file() const noexcept final override;
    virtual bool chk_lock() const noexcept;
    virtual void acq_lock();
    virtual void rel_lock();
};
struct directory_node : public fs_node 
{
    virtual tnode* find(std::string const&) = 0;
    virtual bool link(tnode*, std::string const&) = 0;
    virtual tnode* add(fs_node*) = 0;
    virtual bool unlink(std::string const&) = 0;
    virtual uint64_t num_files() const noexcept = 0;
    virtual uint64_t num_folders() const noexcept = 0;
    virtual std::vector<std::string> lsdir() const = 0;
    directory_node(std::string const& name, uint64_t cid);
    virtual bool is_folder() const noexcept final override;
    virtual uint64_t size() const noexcept override;
    virtual bool is_empty() const noexcept;
    virtual bool relink(std::string const& oldn, std::string const& newn);
};
class device_node final : public file_node
{
    using file_node::traits_type;
	using file_node::difference_type;
	using file_node::size_type;
	using file_node::pos_type;
	using file_node::off_type;
	using file_node::pointer;
	using file_node::const_pointer;
    vfs_filebuf_base<char>* __my_device;
public:
    virtual size_type write(const_pointer src, size_type n) override;
    virtual size_type read(pointer dest, size_type n) override;
    virtual pos_type seek(off_type, std::ios_base::seekdir) override;
    virtual pos_type seek(pos_type) override;
    virtual pos_type tell() const override;
    device_node(std::string const& name, int fd, vfs_filebuf_base<char>* dev_buffer);
    virtual bool fsync() override;
    virtual bool is_device() const noexcept final override;
    virtual uint64_t size() const noexcept override;
};
class tnode
{
    fs_node* __my_node;
    std::string __my_name;
public:
    tnode(fs_node*, std::string const&);
    tnode(fs_node*, const char*);
    tnode(std::string);
    tnode(const char*);
    void rename(std::string const&);
    void rename(const char*);
    const char* name() const;
    fs_node* ptr() noexcept;
    fs_node const* ptr() const noexcept;
    fs_node& ref() noexcept;
    fs_node const& ref() const noexcept;
    fs_node& operator*() noexcept;
    fs_node const& operator*() const noexcept;
    fs_node* operator->() noexcept;
    fs_node const* operator->() const noexcept;
    bool if_file(std::function<bool(file_node&)> const& action);
    bool if_folder(std::function<bool(directory_node&)> const& action);
    bool if_device(std::function<bool(device_node&)> const& action);
    bool if_file(std::function<bool(file_node const&)> const& action) const;
    bool if_folder(std::function<bool(directory_node const&)> const& action) const;
    bool if_device(std::function<bool(device_node const&)> const& action) const;
    bool is_file() const;
    bool is_folder() const;
    bool is_device() const;
    file_node* as_file();
    file_node const* as_file() const;
    directory_node* as_folder();
    directory_node const* as_folder() const;
    device_node* as_device();
    device_node const* as_device() const;
    constexpr operator bool() const noexcept { return bool(__my_node); }
    void invlnode() noexcept;
    bool assign(fs_node* n) noexcept;
    friend tnode mklink(tnode* original, std::string const& name);
    friend bool operator==(tnode const& __this, tnode const& __that) { return __this.__my_name == __that.__my_name && __this.__my_node == __that.__my_node; }
    friend constexpr std::strong_ordering operator<=>(tnode const& __this, tnode const& __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.__my_name.compare(__that.__my_name)); }
    friend constexpr std::strong_ordering operator<=>(tnode const& __this, std::string const& __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.__my_name.compare(__that)); }
    friend constexpr std::strong_ordering operator<=>(std::string const& __this, tnode const&  __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.compare(__that.__my_name)); }
};
typedef std::set<tnode> tnode_dir;
class filesystem
{
protected:
    typedef std::pair<directory_node*, std::string> target_pair;
    std::set<device_node> device_nodes{};
    std::vector<file_node*> current_open_files{};
    int next_fd;
    virtual directory_node* get_root_directory() = 0;
    virtual void dlfilenode(file_node*) = 0;
    virtual void dldirnode(directory_node*) = 0;
    virtual file_node* mkfilenode(directory_node*, std::string const&) = 0;
    virtual directory_node* mkdirnode(directory_node*, std::string const&) = 0;
    virtual void syncdirs() = 0;
    virtual dev_t xgdevid() const noexcept = 0;
    virtual const char* path_separator() const noexcept;
    virtual file_node* open_fd(tnode*);
    virtual void close_fd(file_node*);
    virtual bool xunlink(directory_node* parent, std::string const& what, bool ignore_nonexistent, bool dir_recurse);
    virtual tnode* xlink(target_pair ogpath, target_pair tgpath);
    virtual target_pair get_parent(std::string const& path, bool create);    
    void dldevnode(device_node*);
    device_node* mkdevnode(directory_node*, std::string const&, vfs_filebuf_base<char>*, int fd_number_hint);
public:
    void link_stdio(vfs_filebuf_base<char>* target);
    device_node* lndev(std::string const& where, vfs_filebuf_base<char>* what, int fd_number_hint, bool create_parents = true);
    file_node* get_fd(int fd);      
    file_node* open_file(std::string const& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
    file_node* get_file(std::string const& path);
    directory_node* get_dir(std::string const& path, bool create = true);
    void close_file(file_node* fd);
    dev_t get_dev_id() const noexcept;
    tnode* link(std::string const& ogpath, std::string const& tgpath, bool create_parents = true);
    bool unlink(std::string const& what, bool ignore_nonexistent = true, bool dir_recurse = false);
    filesystem();
private:
    void __put_fd(file_node* result);
};
filesystem* get_fs_instance();
extern "C"
{
    int syscall_open(char* name, int flags, ...);
    int syscall_close(int fd);
    int syscall_write(int fd, char* ptr, int len);
    int syscall_read(int fd, char* ptr, int len);
    int syscall_lseek(int fd, long offs, int way);
    int syscall_link(char* old, char* __new);
    int syscall_unlink(char* name);
    int syscall_isatty(int fd);
    int syscall_fstat(int fd, struct stat* st);
    int syscall_stat(const char* restrict name, struct stat* restrict st);
    int syscall_fchmod(int fd, mode_t m);
    int syscall_chmod(const char* name, mode_t m);
}
#endif