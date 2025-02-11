#ifndef __FILE_SYSTEM
#define __FILE_SYSTEM
/*
 * The base inode structs (inode, file_inode and folder_inode) are essentially vnodes.
 * There is a special node type for devices (such as serial ports) that should work with any sort of filesystem.
 * Similarly, the abstract filesystem class is essentially the vfs; any concrete implementor is the actual file system.
*/
#include "string"
#include "functional"
#include "bits/ios_base.hpp"
#include "vector"
#include "set"
#include "fs/vfs_filebuf_base.hpp"
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
struct inode
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
    virtual ~inode();
    int fd;
    uint64_t real_id;
    uint64_t create_time;
    uint64_t modif_time;
    std::string concrete_name;
    std::set<tnode*> refs{};
    file_mode mode{ 0x0774u };
    inode(std::string const& name, int vfd, uint64_t cid);
    // Move-assign and move-construct only; no copying allowed
    inode(inode const&) = delete;
    inode& operator=(inode const&) = delete;
    inode(inode&&) = default;
    inode& operator=(inode&&) = default;
    void unregister_reference(tnode* ref);
    void prune_refs();
    bool has_refs() const noexcept;
    size_t num_refs() const noexcept;
    friend class tnode;
    friend constexpr std::strong_ordering operator<=>(inode const& a, inode const& b) noexcept { return a.real_id <=> b.real_id; }
    friend constexpr std::strong_ordering operator<=>(inode const& a, uint64_t b) noexcept { return a.real_id <=> b; }
    friend constexpr std::strong_ordering operator<=>(uint64_t a, inode const& b) noexcept { return a <=> b.real_id; }
};
class file_inode : public inode
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
    file_inode(std::string const& name, int vfd, uint64_t cid);
    virtual bool is_file() const noexcept final override;
    virtual bool chk_lock() const noexcept;
    virtual void acq_lock();
    virtual void rel_lock();
};
struct folder_inode : public inode 
{
    virtual tnode* find(std::string const&) = 0;
    virtual bool link(tnode*, std::string const&) = 0;
    virtual tnode* add(inode*) = 0;
    virtual bool unlink(std::string const&) = 0;
    virtual uint64_t num_files() const noexcept = 0;
    virtual uint64_t num_folders() const noexcept = 0;
    virtual std::vector<std::string> lsdir() const = 0;
    folder_inode(std::string const& name, uint64_t cid);
    virtual bool is_folder() const noexcept final override;
    virtual uint64_t size() const noexcept override;
    virtual bool is_empty() const noexcept;
    virtual bool relink(std::string const& oldn, std::string const& newn);
};
class device_inode final : public file_inode
{
    using file_inode::traits_type;
	using file_inode::difference_type;
	using file_inode::size_type;
	using file_inode::pos_type;
	using file_inode::off_type;
	using file_inode::pointer;
	using file_inode::const_pointer;
    vfs_filebuf_base<char>* __my_device;
public:
    virtual size_type write(const_pointer src, size_type n) override;
    virtual size_type read(pointer dest, size_type n) override;
    virtual pos_type seek(off_type, std::ios_base::seekdir) override;
    virtual pos_type seek(pos_type) override;
    device_inode(std::string const& name, int fd, vfs_filebuf_base<char>* dev_buffer);
    virtual bool fsync() override;
    virtual bool is_device() const noexcept final override;
    virtual uint64_t size() const noexcept override;
};
class tnode
{
    inode* __my_node;
    std::string __my_name;
public:
    tnode(inode*, std::string const&);
    tnode(inode*, const char*);
    tnode(std::string);
    tnode(const char*);
    void rename(std::string const&);
    void rename(const char*);
    const char* name() const;
    inode* ptr() noexcept;
    inode const* ptr() const noexcept;
    inode& ref() noexcept;
    inode const& ref() const noexcept;
    inode& operator*() noexcept;
    inode const& operator*() const noexcept;
    inode* operator->() noexcept;
    inode const* operator->() const noexcept;
    bool if_file(std::function<bool(file_inode&)> const& action);
    bool if_folder(std::function<bool(folder_inode&)> const& action);
    bool if_device(std::function<bool(device_inode&)> const& action);
    bool if_file(std::function<bool(file_inode const&)> const& action) const;
    bool if_folder(std::function<bool(folder_inode const&)> const& action) const;
    bool if_device(std::function<bool(device_inode const&)> const& action) const;
    bool is_file() const;
    bool is_folder() const;
    bool is_device() const;
    file_inode* as_file();
    file_inode const* as_file() const;
    folder_inode* as_folder();
    folder_inode const* as_folder() const;
    device_inode* as_device();
    device_inode const* as_device() const;
    constexpr operator bool() const noexcept { return bool(__my_node); }
    void invlnode() noexcept;
    bool assign(inode* n) noexcept;
    friend tnode mklink(tnode* original, std::string const& name);
    friend bool operator==(tnode const& __this, tnode const& __that) { return __this.__my_name == __that.__my_name && __this.__my_node == __that.__my_node; }
    friend constexpr std::strong_ordering operator<=>(tnode const& __this, tnode const& __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.__my_name.compare(__that.__my_name)); }
    friend constexpr std::strong_ordering operator<=>(tnode const& __this, std::string const& __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.__my_name.compare(__that)); }
    friend constexpr std::strong_ordering operator<=>(std::string const& __this, tnode const&  __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(__this.compare(__that.__my_name)); }
};
typedef std::set<tnode> tnode_dir;
class filesystem
{
    void __put_fd(file_inode* result);
protected:
    typedef std::pair<folder_inode*, std::string> target_pair;
    int next_fd;
    std::set<device_inode> device_nodes;
    std::vector<file_inode*> current_open_files;    
    virtual folder_inode* get_root_directory() = 0;
    virtual void dlfilenode(file_inode*) = 0;
    virtual void dldirnode(folder_inode*) = 0;
    virtual file_inode* mkfilenode(folder_inode*, std::string const&) = 0;
    virtual folder_inode* mkdirnode(folder_inode*, std::string const&) = 0;
    virtual void syncdirs() = 0;
    virtual dev_t xgdevid() const noexcept = 0;
    virtual const char* path_separator() const noexcept;
    virtual file_inode* open_fd(tnode*);
    virtual target_pair get_parent(std::string const& path, bool create);
    virtual void close_fd(file_inode*);
    virtual bool xunlink(folder_inode* parent, std::string const& what, bool ignore_nonexistent, bool dir_recurse);
    virtual tnode* xlink(target_pair ogpath, target_pair tgpath);
    void dldevnode(device_inode*);
    device_inode* mkdevnode(folder_inode*, std::string const&, vfs_filebuf_base<char>*, int fd_number_hint);
public:
    void link_stdio(vfs_filebuf_base<char>* target);
    device_inode* lndev(std::string const& where, vfs_filebuf_base<char>* what, int fd_number_hint, bool create_parents = true);
    file_inode* get_fd(int fd);      
    file_inode* open_file(std::string const& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
    file_inode* get_file(std::string const& path);
    folder_inode* get_folder(std::string const& path, bool create = true);
    void close_file(file_inode* fd);
    dev_t get_dev_id() const noexcept;
    tnode* link(std::string const& ogpath, std::string const& tgpath, bool create_parents = true);
    bool unlink(std::string const& what, bool ignore_nonexistent = true, bool dir_recurse = false);
    filesystem();    
};
template<typename T> concept fs_type = std::derived_from<T, filesystem>;
template<fs_type FT> struct fs_container_alloc_helper
{
    typedef std::allocator<FT> allocator_type;
    [[nodiscard]] static filesystem* allocate() { return allocator_type{}.allocate(1); }
    static void deallocate(filesystem* ptr) noexcept { if(FT* ptr2 = dynamic_cast<FT*>(ptr)) allocator_type{}.deallocate(ptr2, 1); }
};
class fs_ptr
{
    typedef filesystem* (*fs_alloc_func)();
    typedef void (*fs_dealloc_func)(filesystem*);
    filesystem* __my_ptr{};
    fs_alloc_func __alloc_fn{};
    fs_dealloc_func __dealloc_fn{};
public:
    typedef filesystem* pointer;
    typedef filesystem const* const_pointer;
    typedef filesystem& reference;
    typedef filesystem const& const_reference;
    constexpr fs_ptr() noexcept = default;
    constexpr fs_ptr(fs_ptr&&) = default;
    constexpr fs_ptr& operator=(fs_ptr&&) = default;
    ~fs_ptr() { if(this->__my_ptr && this->__dealloc_fn) this->__dealloc_fn(this->__my_ptr); }
    template<fs_type FT> constexpr void create()
    {
        using helper = fs_container_alloc_helper<FT>;
        if(this->__my_ptr && this->__dealloc_fn) this->__dealloc_fn(this->__my_ptr);
        this->__alloc_fn = &helper::allocate;
        this->__dealloc_fn = &helper::deallocate;
        this->__my_ptr = this->__alloc_fn();
        new (this->__my_ptr) FT();
    }
        template<fs_type FT, typename ... Args> requires std::constructible_from<FT, Args...> 
    constexpr fs_ptr(std::allocator<FT> const& alloc, Args&& ... args) : 
        __my_ptr{ std::construct_at(alloc.allocate(1), std::forward<Args>(args)...) }, 
        __dealloc_fn{ &fs_container_alloc_helper<FT>::deallocate } 
        {}
    constexpr operator pointer() noexcept { return __my_ptr; }
    constexpr operator const_pointer() const noexcept { return __my_ptr; }
    constexpr reference operator*() noexcept { return *__my_ptr; }
    constexpr const_reference operator*() const noexcept { return *__my_ptr; }
    constexpr pointer operator->() noexcept { return __my_ptr; }
    constexpr const_pointer operator->() const noexcept { return __my_ptr; }
    constexpr operator bool() const noexcept { return bool(__my_ptr); }
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