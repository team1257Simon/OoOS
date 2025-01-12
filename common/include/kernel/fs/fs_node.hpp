#ifndef __FS_NODE
#define __FS_NODE
#include "string"
#include "functional"
#include "bits/ios_base.hpp"
class tnode_base;
struct inode_base
{
    virtual int vid() const noexcept = 0; // virtual ID (fd number)
    virtual uint64_t cid() const noexcept = 0; // concrete id (numeric identifier for the file in the concrete fs)
    virtual uint64_t created_time() const noexcept = 0; // time created
    virtual uint64_t modified_time() const noexcept = 0; // time last modified
    virtual uint64_t size() const noexcept = 0; // size in bytes (for files) or concrete entries (for folders)
    virtual bool is_file() const noexcept = 0;
    virtual bool is_folder() const noexcept = 0;
    virtual const char* name() const = 0; // get the concrete (i.e. on-disk for persistent fs) name
    virtual bool rename(std::string const&) = 0; // change the concrete (i.e. on-disk for persistent fs) name
    constexpr inode_base() noexcept = default;
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
    constexpr file_inode_base() = default;
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
    virtual tnode_base* xfind(std::string const&) = 0;
    virtual bool xlink(tnode_base*, std::string const&) = 0;
    virtual bool xunlink(std::string const&) = 0;
    virtual uint64_t xgnfiles() const noexcept = 0;
    virtual uint64_t xgnfolders() const noexcept = 0;
public:
    constexpr folder_inode_base() = default;
    virtual bool is_file() const noexcept final override;
    virtual bool is_folder() const noexcept final override;
    virtual uint64_t size() const noexcept override;
    virtual bool relink(std::string const& oldn, std::string const& newn);
    tnode_base* find(std::string const& fname);
    bool link(tnode_base* ptr, std::string const& name);
    bool unlink(std::string const& name);
    uint64_t num_files() const noexcept;
    uint64_t num_folders() const noexcept;
};
class tnode_base
{
    inode_base* __my_node;
    std::string __my_name;
public:
    tnode_base(inode_base*, std::string const&);
    tnode_base(inode_base*, const char*);
    tnode_base(tnode_base const&) = default;
    tnode_base(tnode_base&&) = default;
    tnode_base& operator=(tnode_base const&) = default;
    tnode_base& operator=(tnode_base&&) = default;
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
    file_inode_base* as_file();
    file_inode_base const* as_file() const;
    folder_inode_base* as_folder();
    folder_inode_base const* as_folder() const;
};
#endif