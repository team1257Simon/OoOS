#ifndef __FS_EXT
#define __FS_EXT
#include "fs/fs.hpp"
struct ext_superblock
{
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t reserved_blocks;
    uint32_t unallocated_blocks;
    uint32_t unallocated_inodes;
    uint32_t sb_block_index;
    uint32_t block_size_shift;      // blocksize = 1024 << block_size_shift
    uint32_t fragment_size_shift;   // fragmentsize = 1024 << fragment_size_shift
    uint32_t blocks_per_group;
    uint32_t fragmets_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_write_time;
    uint16_t mounts_since_last_fsck;
    uint16_t mount_fsck_cutoff;
    uint16_t magic;
    uint16_t fs_state;
    uint16_t err_action;
    uint16_t v_minor;
    uint32_t last_fsck_time;
    uint32_t time_fsck_cutoff;
    uint32_t os_id;
    uint32_t v_major;
    uint16_t rsv_block_uid;
    uint16_t rsv_block_gid;
    uint32_t first_non_reserved_inode;
    uint16_t inode_size;
    uint16_t sb_backup_block_group;
    uint32_t optional_features;
    uint32_t required_features;         // fetures that must be supported or else we must abort the mount
    uint32_t write_required_Features;   // features that must be supported or else the fs is read-only
    uint8_t fs_uuid[16];
    char volume_name[16];
    char last_mount_path[64];
    uint32_t compression_algorithm_id;
    uint8_t preallocate_file_block_cnt;
    uint8_t preallocate_dir_block_cnt;
    uint16_t expansion_reserved_entries;
    uint8_t journal_uuid[16];
    uint32_t journal_inode;
    uint32_t journal_dev_num;
    uint32_t orphan_inode_list_head;
    uint32_t hash_seed[4];
    uint8_t dir_hash_algorithm_id;
    bool journal_inode_blocks_and_size;
    uint16_t group_descriptor_size;
    uint32_t mount_options;
    uint32_t first_metablock_group;
    uint32_t create_time;
    uint32_t journal_inode_backup[17];
    uint32_t inode_count_hi;
    uint32_t reserved_blocks_hi;
    uint32_t unallocated_blocks_hi;
    uint16_t inode_min_size;
    uint16_t inode_min_reserve_size;
    uint32_t flags;
    uint16_t raid_logical_blocks_per_disk;
    uint16_t multi_mount_protection_wait;
    uint64_t multi_mount_protection_block;
    uint32_t raid_block_rw_thresh;
    uint8_t groups_per_flex_shift;      // as above, shift 1024 by this
    uint8_t metadata_checksum_algo_id;
    uint8_t encryption_version;
    uint8_t pad0;
    uint64_t cumulative_kb_written;
    uint32_t active_snapshot_inode;
    uint32_t active_snapshot_seq_id;
    uint64_t active_snapshot_reserved_blocks;
    uint32_t disk_snapshot_list_head_inode;
    uint32_t num_errors;
    uint32_t first_error_time;
    uint32_t first_error_inode;
    uint64_t first_error_block;
    char first_error_fn[32];
    uint32_t first_error_line;
    uint32_t last_error_time;
    uint32_t last_error_inode;
    uint64_t last_error_block;
    char last_error_fn[32];
    uint32_t last_error_line;
    char extended_mount_options[64];
    uint32_t user_quota_inode;
    uint32_t group_quota_inode;
    uint32_t overhead_blocks;
    uint64_t backup_sb_block_groups;
    uint8_t encryption_types[4];
    uint8_t string2key_salt[16];
    uint32_t lost_found_dir_inode;
    uint32_t project_quota_tracker_inode;
    uint32_t uuid_checksum;
    uint8_t last_write_time_hi;
    uint8_t last_mount_time_hi;
    uint8_t create_time_hi;
    uint8_t last_fsck_time_hi;
    uint8_t first_error_time_hi;
    uint8_t last_error_time_hi;
    uint8_t first_errno;
    uint8_t last_errno;
    uint16_t filename_charset_encoding;
    uint16_t filename_charset_flags;
    uint8_t pad1[380];
    uint32_t checksum;  // crc32c(superblock)
} __pack;
enum required_feature_flags : uint32_t
{
    compression         = 0x000001,
    dirent_type         = 0x000002,
    journal_replay      = 0x000004,
    journal_device      = 0x000008,
    metablock_groups    = 0x000010,
    file_extents        = 0x000040,
    x64_features        = 0x000080,
    multimount_protect  = 0x000100,
    flex_groups         = 0x000200,
    extended_attributes = 0x000400,
    dirent_data         = 0x001000,
    metadata_seed_in_sb = 0x020000,
    inline_data         = 0x080000,
    enctyption          = 0x100000,
    case_folding        = 0x200000
};
enum opt_feature_flags : uint32_t
{
    preallocate_blocks  = 0x0001,
    imagic_inodes       = 0x0002,
    use_journal         = 0x0004,
    ex_inode_attributes = 0x0008,
    resize              = 0x0010,
    dir_hash_index      = 0x0020,
    backup_superblock   = 0x0200,
    static_inode_nums   = 0x0800
};
struct block_group_descriptor
{
    uint32_t block_usage_bitmap_block_idx;
    uint32_t inode_usage_bitmap_block_idx;
    uint32_t inode_table_start_block;
    uint16_t num_unallocated_blocks;
    uint16_t num_unallocated_inodes;
    uint16_t num_directories;
    uint16_t block_group_features;
    uint32_t snapshot_exclude_block;
    uint16_t block_usage_bmp_checkum;
    uint16_t inode_usage_bmp_checksum;
    uint16_t free_inodes;
    uint16_t group_checksum;    // crc16(sb_uuid+group+desc)
    uint32_t block_usage_bitmap_block_idx_hi;
    uint32_t inode_usage_bitmap_block_idx_hi;
    uint32_t inode_table_start_block_hi;
    uint16_t num_unallocated_blocks_hi;
    uint16_t num_unallocated_inodes_hi;
    uint16_t num_directories_hi;
    uint32_t snapshot_exclude_block_hi;
    uint16_t block_usage_bmp_checkum_hi;
    uint16_t inode_usage_bmp_checksum_hi;
    uint16_t free_inodes_hi;
    uint32_t unused;
} __pack;
struct ext_inode
{
    file_mode mode;
    uint16_t uid;
    uint32_t size_lo;
    uint32_t accessed_time;
    uint32_t created_time;
    uint32_t modified_time;
    uint32_t deletion_time;
    uint16_t gid;
    uint16_t referencing_dirents;
    uint16_t size_sectors;
    uint32_t flags;
    uint32_t os_specific_1;
    uint32_t direct_blocks[12];
    uint32_t singly_indirect_bp;
    uint32_t doubly_indirect_bp;
    uint32_t triply_indirect_bp;
    uint32_t generation_number;
    uint32_t file_acl_block;
    uint32_t dir_acl_block; // for a file, this will instead be the high bits of the file size
    uint32_t fragment_block;
    uint32_t os_specific_2[3];
} __pack;
struct ext_dir_entry
{
    uint32_t inode_idx;
    uint16_t entry_size;
    uint8_t name_len;
    uint8_t type_ind;   // if "directory entries have file type byte" is not set, this is instead the high 8-bits of the name length
    char name[];
} __pack;
enum ext_inode_flags : uint32_t
{
    secure_delete = 0x00001,
    retain_copy_on_delete = 0x00002,
    file_compression = 0x00004,
    sync_unbuffered_updates= 0x00008,
    immutable = 0x00010,
    append_only = 0x00020,
    exclude_from_dump = 0x00040,
    no_access_time_update = 0x00080,
    hash_indexed_dir = 0x10000,
    afs_directory = 0x20000,
    journal_file_data = 0x40000
};
struct ext_mmp
{
    uint32_t magic; // 0x004D4D50
    uint32_t sequence;
    uint64_t updated_time;
    char system_hostname[64];
    char mount_path[32];
    uint16_t check_interval;
    uint16_t padding[453];
    uint32_t checksum;  // crc32c(uuid+mmp_block_number)
} __pack;
constexpr __be32 jbd2_magic = 0xC03B3998_be32;
constexpr size_t jbd2_checksum_size_dwords = (32 / sizeof(uint32_t));
struct jbd2_header
{
    __be32 magic;
    __be32 blocktype;
    __be32 sequence;
};
struct jbd2_commit_header
{
    __be32 magic;
    __be32 blocktype;
    __be32 sequence;
    uint8_t checksum_type;
    uint8_t checksum_size;
    uint8_t padding[2];
    __be32 checksum[jbd2_checksum_size_dwords];
    __be64 commit_seconds;
    __be32 commit_nanos;
};
struct jbd_block_tag
{
    __be32 block_number;
    __be32 flags;
    __be32 block_number_hi;
    __be32 checksum;        // crc32c(uuid+seq+block)
};
struct jbd_block_tail { __be32 block_checksum; /* crc32c(uuid+descr_block) */ }; 
struct jbd_revoke_header { jbd2_header header; __be32 block_bytes_used; };
struct ext_journal_superblock
{
    jbd2_header header;
    __be32 journal_block_size;
    __be32 journal_block_count;
    __be32 first_info_block;
    __be32 first_transaction_expected;
    __be32 start_block;
    __be32 journal_errno;
    __be32 required_features;
    __be32 optional_features;
    __be32 readonly_features;
    uint8_t uuid[16];
    __be32 fs_count;
    __be32 jsb_copy_block;
    __be32 jbpt_max; // currently unused
    __be32 dbpt_max; // currently unused
    uint8_t checksum_algorithm_id;
    uint8_t pad0[3];
    __be32 fast_commit_blocks;
    __be32 log_head_block;
    uint8_t pad1[160];
    __be32 checksum;
    uint8_t fs_uuids[768];
};
class extfs;
struct ext_block
{
    uint32_t block_number;  // the block index on disk
    uint8_t* data_start;    // where the data has been stored in memory
    size_t block_size;      // this will be the same as the value in the superblock
};
struct ext_vnode : public vfs_filebuf_base<char>
{
    virtual int __ddwrite() override;
    virtual std::streamsize __ddread(std::streamsize n) override;
    virtual std::streamsize __ddrem() override;
    virtual std::streamsize __overflow(std::streamsize n) override;
    extfs* parent_fs;
    ext_inode* on_disk_node;
    std::vector<ext_block> block_data;
    ext_vnode(extfs* parent, uint32_t inode_number);
};
class ext_file_vnode : public ext_vnode, public file_node
{
    size_t __last_read_block_idx;
public:
    using file_node::traits_type;
    using file_node::difference_type;
    using file_node::size_type;
    using file_node::pos_type;
    using file_node::off_type;
    using file_node::pointer;
    using file_node::const_pointer;
    virtual size_type write(const_pointer src, size_type n) override;
    virtual size_type read(pointer dest, size_type n) override;
    virtual pos_type seek(off_type off, std::ios_base::seekdir way) override;
    virtual pos_type seek(pos_type pos) override;
    virtual bool fsync() override;
    virtual uint64_t size() const noexcept override;
    virtual pos_type tell() const;
    ext_file_vnode(extfs* parent, uint32_t inode_number, int fd);
};
constexpr size_t sb_sectors = (sizeof(ext_superblock) / physical_block_size);
class ext_directory_vnode : public ext_vnode, public directory_node
{
    tnode_dir __my_dir;
public:
    virtual tnode* find(std::string const&) override;
    virtual bool link(tnode* original, std::string const& target) override;
    virtual tnode* add(fs_node* n) override;
    virtual bool unlink(std::string const& name) override;
    virtual uint64_t num_files() const noexcept override;
    virtual uint64_t num_folders() const noexcept override;
    virtual std::vector<std::string> lsdir() const override;
    virtual bool fsync() override;
};
class extfs : public filesystem
{
    std::set<ext_file_vnode> __file_nodes;
    std::set<ext_directory_vnode> __dir_nodes;
    ext_superblock* __my_sb;
    // ...
protected:
    virtual directory_node* get_root_directory() override;
    virtual void dlfilenode(file_node* fd) override;
    virtual void dldirnode(directory_node* dd) override;
    virtual void syncdirs() override;
    virtual file_node* mkfilenode(directory_node* parent, std::string const& name) override;
    virtual directory_node* mkdirnode(directory_node* parent, std::string const& name) override;
    virtual dev_t xgdevid() const noexcept override;
    virtual file_node* open_fd(tnode*) override;
public:
    extfs(uint64_t start_lba);
    ~extfs();
    // ...
};
#endif