#ifndef __FS_EXT
#define __FS_EXT
#include "fs/fs.hpp"
#include "bits/stl_queue.hpp"
#include "map"
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
    uint32_t required_features;             // fetures that must be supported or else we must abort the mount
    uint32_t read_only_optional_features;   // features that must be supported or else the fs is read-only
    guid_t fs_uuid;                         // whenever an extfs checksum references "uuid" it's referring to this
    char volume_name[16];
    char last_mount_path[64];
    uint32_t compression_algorithm_id;
    uint8_t preallocate_file_block_cnt;
    uint8_t preallocate_dir_block_cnt;
    uint16_t expansion_reserved_entries;
    guid_t journal_uuid;
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
    uint32_t block_count_hi;
    uint32_t reserved_blocks_hi;
    uint32_t unallocated_blocks_hi;
    uint16_t inode_min_size;
    uint16_t inode_min_reserve_size;
    uint32_t flags;
    uint16_t raid_logical_blocks_per_disk;
    uint16_t multi_mount_protection_wait;
    uint64_t multi_mount_protection_block;
    uint32_t raid_block_rw_thresh;
    uint8_t groups_per_flex_shift;      // as above, but shift from 1 instead of 1024 (or right-shift the number of groups to get the number of flex groups)
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
    uint32_t uuid_checksum; // crc32c(~0, uuid)
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
enum read_only_opt_flags : uint32_t
{
    sparse_super        = 0x00001,
    large_file          = 0x00002,
    btree_dir           = 0x00004,
    huge_file           = 0x00008,
    gdt_csum            = 0x00010,
    dir_nlink           = 0x00020,
    extra_isize         = 0x00040,
    quota               = 0x00100,
    bigalloc            = 0x00200,
    metadata_csum       = 0x00400,
    replica             = 0x00800,
    readonly_always     = 0x01000,
    project_quotas      = 0x02000,
    verity_inodes       = 0x08000,
    orphans_present     = 0x10000
};
struct block_group_descriptor
{
    uint32_t block_usage_bitmap_block_idx;
    uint32_t inode_usage_bitmap_block_idx;
    uint32_t inode_table_start_block;
    uint16_t unallocated_blocks;
    uint16_t free_inodes;
    uint16_t num_directories;
    uint16_t block_group_features;
    uint32_t snapshot_exclude_block;
    uint16_t block_usage_bmp_checkum;
    uint16_t inode_usage_bmp_checksum;
    uint16_t unused_inodes;
    uint16_t group_checksum;    // crc16(sb_uuid+group+desc) or crc32c(sb_uuid+group+desc) depending on features
    uint32_t block_usage_bitmap_block_idx_hi;
    uint32_t inode_usage_bitmap_block_idx_hi;
    uint32_t inode_table_start_block_hi;
    uint16_t free_blocks_hi;
    uint16_t free_inodes_hi;
    uint16_t num_directories_hi;
    uint16_t unused_inodes_hi;    
    uint32_t snapshot_exclude_block_hi;
    uint16_t block_usage_bmp_checkum_hi;
    uint16_t inode_usage_bmp_checksum_hi;
    uint32_t unused;
} __pack;
constexpr uint16_t ext_extent_magic = 0xF30A;
struct ext_extent_header
{
    uint16_t magic{ ext_extent_magic };
    uint16_t entries;
    uint16_t max_entries;
    uint16_t depth;
    uint32_t generation;
} __pack;
struct ext_extent_index
{
    uint32_t file_node_start;
    uint32_t next_level_block_lo;
    uint16_t next_level_block_hi;
    uint16_t unused;
} __pack;
struct ext_extent_leaf
{
    uint32_t file_node_start;
    uint16_t extent_size;
    uint16_t extent_start_hi;
    uint32_t extent_start_lo;
} __pack;
union ext_extent_node
{
    ext_extent_index idx;
    ext_extent_leaf leaf;
} __pack;
struct ext_extent_tail { uint32_t checksum; } __pack;
union ext_node_extent_root
{
    struct
    {
        ext_extent_header header;
        ext_extent_node root_nodes[4];
    } __pack ext4_extent;
    struct
    {
        uint32_t direct_blocks[12];
        uint32_t singly_indirect_block;
        uint32_t doubly_indirect_block;
        uint32_t triply_indirect_block;
    } __pack legacy_extent;
    char link_target[60]{};   // either a symlink target or device ID
} __pack;
struct ext_inode
{
    file_mode mode;
    uint16_t uid;
    uint32_t size_lo;
    uint32_t accessed_time;
    uint32_t changed_time;
    uint32_t modified_time;
    uint32_t deletion_time;
    uint16_t gid;
    uint16_t referencing_dirents;
    uint32_t blocks_count_lo;
    uint32_t flags;
    uint32_t os_specific_1;             // unused for now
    ext_node_extent_root block_info;    // either a list of block pointers (direct and indirect) or an extent tree
    uint32_t version_lo;
    uint32_t file_acl_block;
    uint32_t size_hi;
    uint32_t fragment_block;
    uint32_t blocks_count_hi;
    uint16_t uid_hi;
    uint16_t gid_hi;
    uint16_t checksum_lo;               // crc32c(uuid+inode_number+inode)
    uint16_t os_specific_3;             // unused for now
    uint16_t extra_isize;
    uint16_t checksum_hi;               // high-order 16 bits of the inode checksum
    uint32_t changed_time_hi;
    uint32_t mod_time_extra;
    uint32_t access_time_hi;
    uint32_t created_time;
    uint32_t created_time_hi;
    uint32_t version_hi;
    uint32_t proj_id;
} __pack;
struct ext_dir_entry
{
    uint32_t inode_idx;
    uint16_t entry_size;
    uint8_t name_len;
    uint8_t type_ind;   // if "directory entries have file type byte" is not set, this is instead the high 8-bits of the name length
    char name[];
} __pack;
enum ext_dirent_type
{
    dti_unknown = 0,
    dti_regular = 1,
    dti_dir = 2,
    dti_chardev = 3,
    dti_blockdev = 4,
    dti_fifo = 5,
    dti_socket = 6,
    dti_symlink = 7
};
struct ext_dx_entry { uint32_t hash; uint32_t node_dirfile_block; } __pack;
enum ext_dx_hash_type : uint8_t
{
    hx_legacy = 0,
    hx_half_md4 = 1,
    hx_tea = 2,
    hx_unsigned_legacy = 3,
    hx_unsigned_half_md4 = 4,
    hx_unsigned_tea = 5,
    hx_siphahs = 6
};
struct ext_dx_root
{
    uint32_t inode_idx;
    uint16_t dot_entry_size{ 12 };
    uint8_t dot_name_len{ 1 };
    uint8_t dot_type_ind{ dti_dir };
    char dot[4]{ '.', '\0', '\0', '\0' };
    uint32_t parent_inode_idx;
    uint16_t dotdot_entry_size;
    uint8_t dotdot_name_len{ 2 };
    uint8_t dotdot_type_ind{ dti_dir };
    char dotdot[4]{ '.', '.', '\0', '\0' };
    uint32_t zero{ 0U };
    ext_dx_hash_type hash_type;
    uint8_t info_len{ 8 };
    uint8_t htree_depth;
    uint8_t unused_flags{ 0 };
    uint16_t limit;
    uint16_t count;
    uint32_t start_file_block;
    ext_dx_entry entries[];
} __pack;
struct ext_dx_node
{
    uint32_t fake_inode{ 0U };
    uint16_t fake_len;
    uint8_t fake_name_len{ 0 };
    uint8_t fake_type{ 0 };
    uint16_t limit;
    uint16_t count;
    uint32_t start_file_block;
    ext_dx_entry entries[];
} __pack;
struct ext_dx_tail { uint32_t dt_reserved; uint32_t dt_checksum; } __pack;
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
    journal_file_data = 0x40000,
    use_extents = 0x80000
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
class extfs;
struct ext_vnode;
struct ext_node_extent_tree;
constexpr __be32 jbd2_magic = 0xC03B3998_be32;
constexpr size_t jbd2_checksum_size_dwords = (32 / sizeof(uint32_t));
typedef unsigned int transaction_id;
struct jbd2_header
{
    __be32 magic = jbd2_magic;
    __be32 blocktype;
    __be32 sequence;
};
enum jbd_block_type : uint32_t
{
    descriptor = 0x1U,
    commit = 0x2U,
    sbv1 = 0x3U,
    sbv2 = 0x4U,
    revocation = 0x5U
};
enum jbd_feature_flags
{
    revocation_records = 0x1,
    x64_support = 0x2,
    async_commit = 0x4,
    csum_v2 = 0x8,
    csum_v3 = 0x10,
    fast_commits = 0x20
};
enum jbd_block_flags
{
    escape = 0x1,
    same_uuid = 0x2,
    last_block = 0x8
};
enum ext_jbd2_mode
{
    ordered,
    journal,
    writeback
};
struct jbd2_commit_header
{
    jbd2_header header{ .blocktype = __be32(commit) };
    uint8_t checksum_type;
    uint8_t checksum_size;
    uint8_t padding[2];
    __be32 checksum[jbd2_checksum_size_dwords];     // crc32c(uuid+commit_block)
    __be64 commit_seconds;
    __be32 commit_nanos;
};
struct jbd2_block_tag3
{
    __be32 block_number;
    __be32 flags;
    __be32 block_number_hi;
    __be32 checksum;        // crc32c(uuid+seq+block)
};
struct jbd2_block_tag
{
    __be32 block_number;
    __be16 checksum;        // crc32c(uuid+seq+block) low 16 bits
    __be16 flags;
    __be32 block_number_hi;
};
struct jbd2_block_tail { __be32 block_checksum; /* crc32c(uuid+descr_block) */ }; 
struct jbd2_revoke_header { jbd2_header header{ .blocktype = __be32(revocation) }; __be32 block_bytes_used; };
struct jbd2_superblock
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
    __be32 readonly_features;   // currently unused
    guid_t uuid;                // "uuid" for jbd2 structure checksums means this field, NOT the ext superblock uuid
    __be32 fs_using_count;
    __be32 jsb_dyn_su_copy_block;
    __be32 jbpt_max;            // currently unused
    __be32 dbpt_max;            // currently unused
    uint8_t checksum_algorithm_id;
    uint8_t pad0[3];
    __be32 fast_commit_blocks;
    __be32 log_head_block;
    uint32_t pad1[40];
    __be32 checksum;            // crc32c(superblock)
    guid_t fs_using_ids[48];
};
struct jbd2_transaction
{
    std::vector<disk_block> data_blocks;
    transaction_id id;
    bool execute_and_complete(extfs* fs_ptr);    // actually do the transaction.
};
typedef std::ext::resettable_queue<jbd2_transaction> jbd2_transaction_queue;
struct cached_extent_node 
{ 
    off_t blk_offset;
    ext_vnode* tracked_node;
    uint16_t depth;
    std::map<uint64_t, off_t> next_level_extents{};
    disk_block* block();
    cached_extent_node(disk_block* bptr, ext_vnode* tracked_node, uint16_t depth);
    size_t nl_recurse_legacy(ext_node_extent_tree* parent, uint64_t start_file_block);
    bool nl_recurse_ext4(ext_node_extent_tree* parent, uint64_t start_file_block);
    bool push_extent_recurse_legacy(ext_node_extent_tree* parent, disk_block* blk);
    bool push_extent_recurse_ext4(ext_node_extent_tree* parent, disk_block* blk);
};
struct ext_node_extent_tree
{
    ext_vnode* tracked_node;
    uint16_t base_depth{};
    size_t total_extent{};
    std::vector<cached_extent_node> tracked_extents{};        // the actual extent map objects are allocated here
    std::map<uint64_t, off_t> base_extent_level{};
    bool has_init{ false };
    bool parse_legacy();
    bool parse_ext4();
    bool push_extent_legacy(disk_block* blk);
    bool push_extent_ext4(disk_block* blk);
    bool ext4_root_overflow();
    off_t cached_node_pos(cached_extent_node const& n);
    off_t cached_node_pos(cached_extent_node const* n);
    cached_extent_node* get_cached(off_t which);
    ext_node_extent_tree(ext_vnode* tracked);
    ext_node_extent_tree();
    ~ext_node_extent_tree();
};
struct ext_vnode : public vfs_filebuf_base<char>
{
    std::vector<disk_block> block_data{}; // all the actual data blocks are recorded here
    std::vector<disk_block> cached_metadata{}; // all metadata blocks, such as extent / indirect block pointers, that are part of the node are cached here
    size_t last_checked_block_idx{};
    uint32_t inode_number;
    extfs* parent_fs;
    ext_inode* on_disk_node;
    ext_node_extent_tree extents;
    ext_vnode(extfs* parent, uint32_t inode_number, ext_inode* inode);
    ext_vnode(extfs* parent, uint32_t inode_number);
    ext_vnode() = default;
    virtual ~ext_vnode();
    virtual int __ddwrite() override;
    virtual bool initialize();
    void mark_write(void* pos);
    void update_block_ptrs();
    bool expand_buffer(size_t added_bytes);
    bool init_extents();
    uint64_t next_block();
    size_t block_of_data_ptr(size_t offs);
};
struct jbd2 : public ext_vnode
{
    jbd2_superblock* sb;
    bool has_init{ false };
    jbd2_transaction_queue active_transactions{};
    std::vector<disk_block> replay_blocks{};
    uint32_t first_open_block{ 1U };  // the value in the superblock is only valid when the journal is empty, so just track the value here (if we boot to a non-empty journal we'll figure this value during the replay)
    bool create_txn(ext_vnode* changed_node);           // this overload is for files and directories (only needed in full journal and writeback mode)
    bool create_txn(std::vector<disk_block> const&);    // this overload can be used directly for metadata (needed in all modes)
    bool need_escape(disk_block const& bl);
    bool clear_log();
    bool read_log();
    bool ddread();
    bool ddwrite();
    void parse_next_log_entry(std::vector<disk_block>::iterator& i);
    off_t desc_tag_create(disk_block const& bl, void* where, uint32_t seq, bool is_first = false, bool is_last = false);
    size_t desc_tag_size(bool same_uuid);
    size_t tags_per_block();
    bool write_block(disk_block const& bl);
    bool execute_pending_txns();
    void free_buffers(std::vector<disk_block>& bufs);
    char* allocate_block_buffer();
    uint32_t calculate_sb_checksum();
    ~jbd2();
    jbd2() = default;
    jbd2(extfs* parent, uint32_t inode);
    virtual bool initialize() override;
};
class ext_file_vnode : public ext_vnode, public file_node
{
public:
    using file_node::traits_type;
    using file_node::difference_type;
    using file_node::size_type;
    using file_node::pos_type;
    using file_node::off_type;
    using file_node::pointer;
    using file_node::const_pointer;
    virtual std::streamsize __overflow(std::streamsize n) override;
    virtual std::streamsize __ddread(std::streamsize n) override;
    virtual std::streamsize __ddrem() override;
    virtual size_type write(const_pointer src, size_type n) override;
    virtual size_type read(pointer dest, size_type n) override;
    virtual pos_type seek(off_type off, std::ios_base::seekdir way) override;
    virtual pos_type seek(pos_type pos) override;
    virtual bool fsync() override;
    virtual uint64_t size() const noexcept override;
    virtual pos_type tell() const;
    virtual bool initialize() override;
    virtual ~ext_file_vnode();
    ext_file_vnode(extfs* parent, uint32_t inode_number, int fd);
    ext_file_vnode(extfs* parent, uint32_t inode_number, ext_inode* inode_data, int fd);
};
constexpr size_t sb_sectors = (sizeof(ext_superblock) / physical_block_size);
constexpr off_t sb_off = (1024L / physical_block_size);
struct dirent_idx { unsigned block_num; unsigned block_offs; };
class ext_directory_vnode : public ext_vnode, public directory_node
{
    tnode_dir __my_dir;
    std::map<tnode*, dirent_idx> __dir_index{};
    bool __initialized{};
    size_t __n_subdirs{};
    size_t __n_files{};
    bool __parse_entries(size_t bs);
    bool __seek_available_entry(size_t name_len);
    void __write_dir_entry(ext_vnode* vnode, ext_dirent_type type, const char* name, size_t name_len);
    ext_dir_entry* __current_ent();
public:
    bool add_dir_entry(ext_vnode* vnode, ext_dirent_type type, const char* name, size_t name_len);
    virtual std::streamsize __overflow(std::streamsize n) override;
    virtual std::streamsize __ddread(std::streamsize n) override;
    virtual std::streamsize __ddrem() override;
    virtual tnode* find(std::string const&) override;
    virtual bool link(tnode* original, std::string const& target) override;
    virtual tnode* add(fs_node* n) override;
    virtual bool unlink(std::string const& name) override;
    virtual uint64_t num_files() const noexcept override;
    virtual uint64_t num_subdirs() const noexcept override;
    virtual std::vector<std::string> lsdir() const override;
    virtual bool fsync() override;
    virtual bool initialize() override;
    constexpr bool has_init() const noexcept { return __initialized; }
    ext_directory_vnode(extfs* parent, uint32_t inode_number);
    ext_directory_vnode(extfs* parent, uint32_t inode_number, ext_inode* inode_data);
};
struct ext_block_group
{
    extfs* parent_fs;
    block_group_descriptor* descr;
    disk_block inode_usage_bmp;
    disk_block blk_usage_bmp;
    disk_block inode_block;
    bool has_available_inode();
    bool has_available_blocks();
    bool has_available_blocks(size_t n);
    void alter_available_blocks(int64_t diff);
    void decrement_inode_ct();
    void increment_inode_ct();
    void compute_checksums(size_t);
    ext_block_group(extfs* parent, block_group_descriptor* desc);
    ~ext_block_group();
    ext_block_group(ext_block_group const&) = delete;
    ext_block_group& operator=(ext_block_group const&) = delete;
    ext_block_group(ext_block_group&& that);
};
class extfs : public filesystem
{
protected:    
    std::set<ext_file_vnode> file_nodes;
    std::set<ext_directory_vnode> dir_nodes;
    std::vector<ext_block_group> block_groups;
    ext_superblock* sb;
    block_group_descriptor* blk_group_descs;
    directory_node* root_dir;
    size_t num_blk_groups;
    uint64_t superblock_lba;
    jbd2 fs_journal;
    disk_block bg_table_block{ 1UL, nullptr };
    friend struct ext_block_group;
    bool initialized{ false };
    virtual directory_node* get_root_directory() override;
    virtual void dlfilenode(file_node* fd) override;
    virtual void dldirnode(directory_node* dd) override;
    virtual void syncdirs() override;
    virtual file_node* mkfilenode(directory_node* parent, std::string const& name) override;
    virtual directory_node* mkdirnode(directory_node* parent, std::string const& name) override;
    virtual dev_t xgdevid() const noexcept override;
    virtual file_node* open_fd(tnode* fd) override;
    ext_jbd2_mode journal_mode() const;
    bool read_hd(void* dest, uint64_t lba_src, size_t sectors);
    bool write_hd(uint64_t lba_dest, const void* src, size_t sectors);
    uint32_t claim_inode();
    bool release_inode(uint32_t num);
    void release_blocks(uint64_t start, size_t num);
public:
    char* allocate_block_buffer();
    void free_block_buffer(disk_block& bl);
    void allocate_block_buffer(disk_block& bl);
    size_t block_size();
    size_t inodes_per_block();
    uint32_t inode_pos_in_group(uint32_t inode_num);
    size_t sectors_per_block();
    size_t blocks_per_group();
    size_t inodes_per_group();
    uint64_t block_to_lba(uint64_t block);
    uint64_t group_num_for_inode(uint32_t inode);
    uint64_t inode_to_block(uint32_t inode);
    disk_block* claim_blocks(ext_vnode* requestor, size_t how_many);
    disk_block* claim_metadata_block(ext_node_extent_tree* requestor);
    off_t inode_block_offset(uint32_t inode);
    ext_inode* get_inode(uint32_t inode_num);
    bool write_to_disk(disk_block const& bl);
    bool write_unbuffered(disk_block const& bl);
    bool read_from_disk(disk_block& bl);
    bool read_unbuffered(disk_block& bl);
    bool persist_group_metadata(size_t group_num);    
    bool persist_inode(uint32_t inode_num);
    bool persist(ext_file_vnode* n);
    bool persist(ext_directory_vnode* n);
    fs_node* dirent_to_vnode(ext_dir_entry* de);
    void initialize();
    constexpr bool has_init() const { return initialized; }
    extfs(uint64_t volume_start_lba);
    ~extfs();
};
template<typename ... Ts> uint32_t blk_crc32(disk_block const& blk, size_t block_size, Ts const* ... rem) { return crc32_calc(blk.data_buffer, block_size * blk.chain_len, crc32c(rem...)); }
#endif