#ifndef __FS_NODE
#define __FS_NODE
#include "string"
#include "functional"
#include "bits/ios_base.hpp"
#include "set"
class tnode;
struct inode_base
{
    virtual int vid() const noexcept; // virtual ID (fd number)
    virtual uint64_t cid() const noexcept; // concrete id (numeric identifier for the file in the concrete fs)
    virtual uint64_t created_time() const noexcept; // time created
    virtual uint64_t modified_time() const noexcept; // time last modified
    virtual bool rename(std::string const&); // change the concrete (i.e. on-disk for persistent fs) name
    virtual const char* name() const; // get the concrete (i.e. on-disk for persistent fs) name
    virtual uint64_t size() const noexcept = 0; // size in bytes (for files) or concrete entries (for folders)
    virtual bool is_file() const noexcept = 0;
    virtual bool is_folder() const noexcept = 0;
    virtual bool fsync() = 0; // Sync to disc, if applicable
    friend constexpr auto operator<=>(inode_base const& a, inode_base const& b) -> decltype(std::declval<uint64_t>() <=> std::declval<uint64_t>()) { return a.cid() <=> b.cid(); }
    virtual ~inode_base();
    void prune_refs();
    friend class tnode;
protected:
    std::string real_name;
    int fd;
    uint64_t real_id;
    uint64_t create_time;
    uint64_t modif_time;
    std::set<tnode*> refs{};
    inode_base(std::string const& name, int vfd, uint64_t cid);
    // Move-assign and construct only.
    inode_base(inode_base const&) = delete;
    inode_base& operator=(inode_base const&) = delete;
    inode_base(inode_base&&) = default;
    inode_base& operator=(inode_base&&) = default;
};
class file_inode_base : public inode_base
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
protected:
    virtual size_type xwrite(const_pointer src, size_type n) = 0;
    virtual size_type xread(pointer dest, size_type n) = 0;
    virtual pos_type xseekl(off_type, std::ios_base::seekdir) = 0;
    virtual pos_type xseekp(pos_type) = 0;
public:
    file_inode_base(std::string const& name, int vfd, uint64_t cid);
    virtual bool is_file() const noexcept final override;
    virtual bool is_folder() const noexcept final override;
    virtual bool chk_lock() const noexcept;
    virtual void acq_lock();
    virtual void rel_lock();
    pos_type seek(off_type offs, std::ios_base::seekdir way);
    pos_type seek(pos_type pos);
    size_type write(const_pointer src, size_type n);
    size_type read(pointer dest, size_type n);
};
class folder_inode_base : public inode_base 
{
protected:
    virtual tnode* xfind(std::string const&) = 0;
    virtual bool xlink(tnode*, std::string const&) = 0;
    virtual tnode* xadd(inode_base*, std::string const&) = 0;
    virtual bool xunlink(std::string const&) = 0;
    virtual uint64_t xgnfiles() const noexcept = 0;
    virtual uint64_t xgnfolders() const noexcept = 0;
public:
    folder_inode_base(std::string const& name, uint64_t cid);
    virtual bool is_file() const noexcept final override;
    virtual bool is_folder() const noexcept final override;
    virtual uint64_t size() const noexcept override;
    virtual bool relink(std::string const& oldn, std::string const& newn);
    tnode* find(std::string const& fname);
    bool link(tnode* ptr, std::string const& name);
    tnode* add(inode_base* node);
    bool unlink(std::string const& name);
    uint64_t num_files() const noexcept;
    uint64_t num_folders() const noexcept;
};
class tnode
{
    inode_base* __my_node;
    std::string __my_name;
public:
    tnode(inode_base*, std::string const&);
    tnode(inode_base*, const char*);
    void rename(std::string const&);
    void rename(const char*);
    const char* name() const;
    inode_base& operator*() noexcept;
    inode_base const& operator*() const noexcept;
    inode_base* operator->() noexcept;
    inode_base const* operator->() const noexcept;
    bool if_file(std::function<bool(file_inode_base&)> const& action);
    bool if_folder(std::function<bool(folder_inode_base&)> const& action);
    bool if_file(std::function<bool(file_inode_base const&)> const& action) const;
    bool if_folder(std::function<bool(folder_inode_base const&)> const& action) const;
    bool is_file() const;
    bool is_folder() const;
    file_inode_base* as_file();
    file_inode_base const* as_file() const;
    folder_inode_base* as_folder();
    folder_inode_base const* as_folder() const;
    constexpr operator bool() const noexcept { return bool(__my_node); }
    void invlnode() noexcept;
    friend constexpr auto operator<=>(tnode const& __this, tnode const& __that) noexcept -> decltype(std::__detail::__char_traits_cmp_cat<std::string::traits_type>(0)) { return __this.__my_name <=> __that.__my_name; }
    template<std::convertible_to<std::string> ST> friend constexpr auto operator<=>(tnode const& __this, ST const& __that) noexcept -> decltype(std::__detail::__char_traits_cmp_cat<std::string::traits_type>(0)) { return __this.__my_name <=> __that; }
    template<std::convertible_to<std::string> ST> friend constexpr auto operator<=>(ST const& __this, tnode const& __that) noexcept -> decltype(std::__detail::__char_traits_cmp_cat<std::string::traits_type>(0)) { return __this <=> __that.__my_name; }
    friend constexpr bool operator==(tnode const& __this, tnode const& __that) noexcept { return __this.__my_name == __that.__my_name; }
    template<std::convertible_to<std::string> ST> friend constexpr bool operator==(tnode const& __this, ST const& __that) noexcept { return __this.__my_name == __that; }
    template<std::convertible_to<std::string> ST> friend constexpr bool operator==(ST const& __this, ST const& __that) noexcept { return __this == __that.__my_name; }
    friend tnode mklink(tnode* original, std::string const& name);
};
struct node_cmp
{
    constexpr bool operator()(std::string const& a, tnode const& b) const noexcept { return a < b; }
    constexpr bool operator()(tnode const& a, std::string const& b) const noexcept { return a < b; }
    constexpr bool operator()(tnode const& a, tnode const& b) const noexcept { return a < b; }
};
typedef std::set<tnode, node_cmp> tnode_dir;
#endif