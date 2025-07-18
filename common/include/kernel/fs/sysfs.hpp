#ifndef __SYSFS
#define __SYSFS
/**
 * While sysfs isn't technically a proper filesystem, it is a part of the filesystem and uses many of the same concepts.
 * It's how the various configurations and other system objects (such as user account info) are persisted.
 * A given object will be tagged in the directory file with a name up to 23 characters long.
 * This limit allows directory entries to be precisely 32 bytes in length and still include a checksum.
 * Each object's index node specifies a type code (such as user info, network config, etc) and points to an extent tree.
 * The extent trees are essentially simplified versions of the ext4 extent system.
 * All extent nodes beyond those in the index itself are stored in a separate file, with non-leaf nodes pointing to an ordinal within that file.
 * Leaf nodes, of course, point to blocks in the data file itself.
 * Eventually, I might expand sysfs to support a sort-of registry that applications and drivers can also use.
 */
#include "fs/fs.hpp"
#include "sys/errno.h"
#include "typeindex"
#include "unordered_map"
constexpr uint32_t sysfs_magic                  = 0xA11C0DED;
constexpr uint32_t sysfs_extent_magic           = 0xB16F11E5;
constexpr uint32_t sysfs_directory_magic        = 0xCA11ED17;
constexpr uint32_t sysfs_index_magic            = 0xD1617A15;
constexpr size_t sysfs_extent_branch_size       = 128UZ;
constexpr size_t sysfs_data_block_size          = 1024UZ;
constexpr size_t sysfs_object_name_size_max     = 23UZ;
enum class sysfs_object_type : uint8_t
{
    NONE,
    GENERAL_CONFIG,
    USER_INFO,
    NET_CONFIG,
    // ...
};
struct sysfs_extent_entry 
{
    uint32_t ordinal;   // x: nth block in this sequence -> (n+x)th block in data entry
    uint32_t length;    // length in blocks if this is a leaf
    size_t start;       // points to a block in the data file if the depth is 0; otherwise points to another branch
};
struct sysfs_data_block
{ 
    char data[sysfs_data_block_size];
    constexpr operator char*() & noexcept { return data; }
    constexpr operator const char*() const& noexcept { return data; }
};
struct sysfs_extent_branch
{
    constexpr static size_t num_entries = (sysfs_extent_branch_size - (sizeof(uint64_t) + sizeof(uint64_t))) / sizeof(sysfs_extent_entry);
    uint64_t depth;                 
    sysfs_extent_entry entries[num_entries];
    uint64_t checksum;              // 32-bit checksum, but we had extra space
    constexpr sysfs_extent_entry& operator[](size_t i) & { return entries[i]; }
    constexpr sysfs_extent_entry const& operator[](size_t i) const& { return entries[i]; }
};
struct __pack sysfs_inode
{
    sysfs_object_type type;
    uint16_t root_depth;
    size_t size_bytes;
    sysfs_extent_entry extent_root;
    uint32_t checksum;
};
struct __pack sysfs_dir_entry
{
    uint32_t inode_number;
    sysfs_object_type object_type;
    char object_name[sysfs_object_name_size_max];
    uint32_t checksum;
};
struct __pack sysfs_index_file
{
    uint32_t magic = sysfs_index_magic;
    char backup_file_name[16];
    size_t total_inodes;
    uint32_t header_checksum;
    sysfs_inode inodes[];
};
struct __pack sysfs_extents_file
{
    uint32_t magic = sysfs_extent_magic;
    char backup_file_name[16];
    size_t total_branches;
    uint32_t header_checksum;
    sysfs_extent_branch branches[];
};
struct __pack sysfs_directory_file
{
    uint32_t magic = sysfs_directory_magic;
    char backup_file_name[16];
    size_t total_entries;
    uint32_t header_checksum;
    sysfs_dir_entry entries[];
};
struct __pack sysfs_data_file_header
{
    uint32_t magic = sysfs_magic;
    char backup_file_name[16];
    size_t total_size;
    uint32_t header_checksum;
};
class sysfs;
struct sysfs_vnode;
class sysfs_extent_tree
{
    constexpr static size_t __start_pos     = sysfs_extent_branch::num_entries / 2;
    constexpr static size_t __start_incr    = div_to_nearest(sysfs_extent_branch::num_entries, 4U);
    sysfs_vnode& __managed_vnode;
    size_t __total_extent;
    size_t __stored_leaf_index;
    sysfs_inode& __inode();
    sysfs_inode const& __inode() const;
    sysfs_extent_branch& __root();
    sysfs_extent_branch const& __root() const;
    sysfs_extent_entry& __root_first();
    sysfs_extent_entry const& __root_first() const;
    sysfs_extent_entry const& __find_from(sysfs_extent_branch const& b, size_t idx, size_t pos = __start_pos, size_t incr = __start_incr) const;
    void __overflow_root();
    sysfs_extent_branch& __next_leaf_branch();
public:
    sysfs_extent_tree(sysfs_vnode& n);
    void push(uint16_t n_blocks);
    sysfs_extent_entry const& operator[](size_t i) const;
    size_t total_extent() const;
};
struct sysfs_vnode : std::ext::dynamic_streambuf<char>
{
    sysfs_inode& inode();
    sysfs_inode const& inode() const;
    sysfs& parent();
    uint32_t inode_number() const;
    void init();
    sysfs_vnode(sysfs& sysfs_parent, uint32_t inode_num);
protected:
    friend class sysfs_extent_tree;
    using __base = std::ext::dynamic_streambuf<char>;
    sysfs& parent_fs;
    uint32_t ino;
    sysfs_extent_tree extent_tree;
    std::vector<size_t> dirty_blocks;
    virtual int write_dev() override;
    virtual std::streamsize on_overflow(std::streamsize n) override;
    virtual std::streamsize sector_size() const override;
    virtual std::streamsize xsputn(char const* s, std::streamsize n) override;
};
struct sysfs_file_ptrs
{
    file_node* data_file;
    file_node* index_file;
    file_node* extents_file;
    file_node* directory_file;
};
struct sysfs_backup_filenames
{
    char data_backup_file_name[16];
    char index_backup_file_name[16];
    char extents_backup_file_name[16];
    char directory_backup_file_name[16];
};
class sysfs
{
    file_node& __data_file;
    file_node& __index_file;
    file_node& __extents_file;
    file_node& __directory_file;
    std::unordered_map<std::string, uint32_t> __directory_map;
    sysfs_index_file& __index();
    sysfs_directory_file& __dir();
    sysfs_extents_file& __extents();
    sysfs_data_file_header& __header();
    sysfs_data_block& __block(size_t num);
    size_t __num_blocks() const;
public:
    sysfs(sysfs_file_ptrs const& files);
    void init_blank(sysfs_backup_filenames const& bak);
    void write_data(size_t start_block, const char* data, size_t n);
    void read_data(char* out, size_t start_block, size_t n);
    bool sync();
    sysfs_extent_branch& get_extent_branch(size_t idx);
    sysfs_extent_branch const& get_extent_branch(size_t idx) const;
    sysfs_inode& get_inode(size_t ino);
    sysfs_inode const& get_inode(size_t ino) const;
    sysfs_dir_entry& get_dir_entry(size_t num);
    sysfs_dir_entry const& get_dir_entry(size_t num) const;
    uint32_t add_extent_branch();
    uint32_t add_blocks(uint16_t how_many);
    uint32_t add_inode();
    uint32_t add_directory_entry();
    uint32_t find_node(std::string const& name);
    int dir_add_object(std::string const& name, uint32_t ino);
    sysfs_extent_branch& extend_to_leaf(size_t from_idx, uint32_t ordinal);
    std::pair<sysfs_extent_branch*, size_t> next_available_extent_entry(size_t from_idx);
};
#endif