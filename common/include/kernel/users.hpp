#ifndef __USER_INFO
#define __USER_INFO
#include "sys/types.h"
#include "fs/fs.hpp"
constexpr size_t username_max_len               = 32;
constexpr size_t crypto_setting_len             = 29;
constexpr size_t crypto_hash_len                = 31;
constexpr uint16_t create_group                 = 0x0001;
constexpr uint16_t create_user                  = 0x0002;
constexpr uint16_t delete_group                 = 0x0004;
constexpr uint16_t delete_user                  = 0x0008;
constexpr uint16_t change_group_capabilities    = 0x0010;
constexpr uint16_t change_user_capabilities     = 0x0020;
constexpr uint16_t alter_group_membership       = 0x0040;
constexpr uint16_t change_file_ownership        = 0x0080;
constexpr uint16_t create_environment_variable  = 0x0100;
constexpr uint16_t delete_environment_variable  = 0x0200;
constexpr uint16_t change_file_modes            = 0x0400;
constexpr uint16_t change_directory_modes       = 0x0800;
constexpr uint16_t install_module               = 0x1000;
constexpr uint16_t remove_module                = 0x2000;
constexpr uint16_t start_daemon                 = 0x4000;
constexpr uint16_t stop_daemon                  = 0x8000;
constexpr uint32_t user_file_magic              = 0xC001B015;
constexpr size_t   user_file_block_size         = 4096UZ;
struct user_credentials
{
    char        user_login_name[username_max_len];
    char        crypto_setting_str[crypto_setting_len];
    char        password_hash_str[crypto_hash_len];
    time_t      last_pw_change_time;
    time_t      pw_change_require_interval;
    time_t      pw_change_warning_interval;
    uint32_t    login_require_interval;
    uint32_t    checksum;           // crc32c(user_credentials+sb_uuid)
} __pack;
struct user_capabilities
{
    uint32_t    thread_quota;
    uint32_t    process_quota;
    uint32_t    system_permissions;
    uint32_t    checksum;           // crc32c(user_capabilities+sb_uuid)
} __pack;
enum user_file_block_type : uint32_t
{
    BT_NULL         = 0,
    BT_INDEX        = 1,
    BT_USER_INFO    = 2,
    BT_CREDENTIALS  = 3,
    BT_CAPABILITIES = 4,
    BT_STRING       = 5,
    BT_UID_INDEX    = 6
};
struct user_file_index_entry
{
    size_t block_idx;               // the block index into the file (blocks of size 512, not FS blocks; the block index zero points to the superblock)
    uint32_t type;                  // the type of entry found in the block (null means the entry is empty)
    uint32_t checksum;              // crc32c(entry+sb_uuid)
} __pack;
struct user_file_object_ptr
{
    size_t entry_block_idx;         // the file block in which the object is found
    size_t offset_in_block;         // the index of the object in its block
} __pack;
struct user_file_block_tail { uint32_t block_checksum; /* crc32c(sb_uuid+block) */ } __pack;
struct user_persisted_info
{
    uid_t                   uid;
    gid_t                   gid;
    user_file_object_ptr    credentials_ptr;
    user_file_object_ptr    capabilities_ptr;
    user_file_object_ptr    contact_string_ptr;
    user_file_object_ptr    home_dir_ptr;
    uint32_t                checksum;                   // crc32c(sb_uuid+user_persisted_info)
} __pack;
constexpr size_t usable_block_size = user_file_block_size - sizeof(user_file_block_tail);
constexpr size_t ptrs_per_lookup_block = user_file_block_size / sizeof(user_file_object_ptr);
constexpr size_t creds_per_block = usable_block_size / sizeof(user_credentials);
constexpr size_t uinfos_per_block = usable_block_size / sizeof(user_persisted_info);
constexpr size_t caps_per_block = usable_block_size / sizeof(user_capabilities);
constexpr size_t idx_per_block = usable_block_size / sizeof(user_file_index_entry);
struct user_file_superblock
{
    uint32_t                magic;
    time_t                  last_update_time;
    guid_t                  system_uuid;                // generated for each system when the file is created
    gid_t                   superuser_gid;
    uid_t                   superuser_uid;
    uid_t                   first_available_uid;
    gid_t                   first_available_gid;
    size_t                  total_blocks;               // the size of the file in 4096-byte blocks (regardless of the filesystem's logical block size)
    uint32_t                lookup_table_blocks;        // this many of the first blocks after the superblock are uid quick-lookup table data
    uint32_t                lookup_table_checksum;      // crc32c(system_uuid+lookup_table)
    uint32_t                sb_checksum;                // crc32c(superblock)
    user_file_object_ptr    next_available_info_slot;
    user_file_object_ptr    next_available_cred_slot;
    user_file_object_ptr    next_available_caps_slot;
    user_file_object_ptr    next_available_string_slot;
    user_file_object_ptr    next_available_index_slot;
    user_file_index_entry   idx0[];                     // remaining superblock space is used for index entries
} __pack;
union [[gnu::may_alias]] user_file_block_data
{
    user_file_index_entry       index_data      [idx_per_block];
    user_credentials            cred_data       [creds_per_block];
    user_capabilities           capability_data [caps_per_block];
    user_persisted_info         user_data       [uinfos_per_block];
    user_file_object_ptr        uid_lookup_data [ptrs_per_lookup_block];
    user_file_superblock        superblock;
    struct
    {
        char                    string_data     [usable_block_size];
        user_file_block_tail    tail;
    };
} __pack;
struct user_info
{
    uid_t                   uid;
    gid_t                   gid;
    user_credentials        credentials;
    user_capabilities       capabilities;
    std::string             contact;
    std::string             home_directory;
};
class user_file
{
    file_node& __file;
    std::unordered_map<std::string, uid_t> __username_map;
protected:
    user_file_block_data* blocks();
    size_t num_blocks();
    user_file_superblock& get_superblock();
    addr_t resolve_object_ptr(user_file_object_ptr const& p, user_file_block_type type);
    user_persisted_info& get_user_data(uid_t uid);
public:
    user_file(file_node& file);
    void get_user_info(user_info* out);
};
#endif