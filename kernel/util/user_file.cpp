#include "users.hpp"
#include "stdexcept"
#include "ow-crypt.h"
#include "arch/entro_amd64.hpp"
using std::addressof;
char __user_file_instance_data[sizeof(user_file)]{};
user_file* user_file::__instance = reinterpret_cast<user_file*>(__user_file_instance_data);
user_file::user_file(file_node& file) : __file(file), __username_map{ 256 }, __uid_map{ 256 }, __has_init{ false } {}
user_file& user_file::get() { return *__instance; }
user_file_block_data* user_file::blocks() { return reinterpret_cast<user_file_block_data*>(__file.data()); }
size_t user_file::num_blocks() { return __file.size() / user_file_block_size; }
user_file_superblock& user_file::get_superblock() { return blocks()[0].superblock; }
bool user_file::init_instance(file_node& file) { return (new(__instance) user_file(file))->__initialize(); }
user_file_object_ptr& user_file::__next_object_slot(user_file_block_type type)
{
    user_file_superblock& sb = get_superblock();
    switch(type)
    {
    case BT_INDEX:
        return sb.next_available_index_slot;
    case BT_USER_INFO:
        return sb.next_available_info_slot;
    case BT_CREDENTIALS:
        return sb.next_available_cred_slot;
    case BT_CAPABILITIES:
        return sb.next_available_caps_slot;
    case BT_STRING:
        return sb.next_available_string_slot;
    default:
        throw std::invalid_argument{ "no slot indicator of that type" };
    }
}
void user_file::__write_block_csum(size_t idx)
{
    user_file_superblock& sb    = get_superblock();
    user_file_block_data& data  = blocks()[idx];
    data.tail.block_checksum    = 0;
    uint32_t sb_csum            = crc32c(crc32c(sb.system_uuid), data);
    data.tail.block_checksum    = sb_csum;
}
bool user_file::__verify_block_csum(size_t idx)
{
    uint32_t sb_seed            = crc32c(get_superblock().system_uuid);
    user_file_block_data& data  = blocks()[idx];
    uint32_t checkval           = data.tail.block_checksum;
    data.tail.block_checksum    = 0;
    uint32_t csum               = crc32c(sb_seed, data);
    data.tail.block_checksum    = checkval;
    return csum == checkval;
}
void user_file::add_block(user_file_block_type type)
{
    if(!__file.grow(user_file_block_size)) throw std::runtime_error{ "couldn't get disk space" };
    else
    {
        user_file_superblock& sb    = get_superblock();
        user_file_index_entry& ent  = resolve_object_ptr(sb.next_available_index_slot, BT_INDEX).ref<user_file_index_entry>();
        ent.block_idx               = sb.total_blocks++;
        ent.type                    = type;
        ent.checksum                = 0;
        uint32_t csum               = crc32c(crc32c(sb.system_uuid), ent);
        ent.checksum                = csum;
        size_t idx_slot             = sb.next_available_index_slot.entry_block_idx;
        if(idx_slot) __write_block_csum(idx_slot);
        advance_object_slot(BT_INDEX);
    }
}
void user_file::advance_object_slot(user_file_block_type type)
{
    user_file_superblock& sb = get_superblock();
    if(type == BT_STRING) // this will be called only when the string needs a new block
    {
        sb.next_available_string_slot.offset_in_block = 0;
        sb.next_available_string_slot.entry_block_idx = sb.total_blocks;
        add_block(BT_STRING);
        __write_block_csum(0UZ);
    }
    else try
    {
        user_file_object_ptr& obj   = __next_object_slot(type);
        size_t entsz = entry_size(type);
        if((obj.offset_in_block + 1) * entsz < usable_block_size) obj.offset_in_block++;
        else
        {
            size_t block_num        = sb.total_blocks;
            obj.offset_in_block     = 0;
            obj.entry_block_idx     = block_num;
            add_block(type);        // need to do this after because resizing the buffer will otherwise create a dangling reference
        }
        __write_block_csum(0UZ);
    }
    catch(std::invalid_argument&) { panic("do not use this function directly if you don't know what you're doing"); }
}
addr_t user_file::resolve_object_ptr(user_file_object_ptr const& p, user_file_block_type type)
{
    if(!p.entry_block_idx && type != BT_INDEX) return nullptr;
    user_file_block_data& block = blocks()[p.entry_block_idx];
    switch(type)
    {
    case BT_INDEX:
        return addressof(block.index_data[p.offset_in_block]);
    case BT_USER_INFO:
        return addressof(block.user_data[p.offset_in_block]);
    case BT_CREDENTIALS:
        return addressof(block.cred_data[p.offset_in_block]);
    case BT_CAPABILITIES:
        return addressof(block.capability_data[p.offset_in_block]);
    case BT_STRING:
        return addressof(block.string_data[p.offset_in_block]);
    default:
        return nullptr;
    }
}
user_persisted_info& user_file::get_user_data(uid_t uid)
{
    if(__uid_map.contains(uid)) 
        return resolve_object_ptr(__uid_map[uid], BT_USER_INFO).ref<user_persisted_info>();
    throw std::invalid_argument{ "no such uid" };
}
void user_file::get_user_info(user_info& out)
{
    user_persisted_info& info   = get_user_data(out.uid);
    user_credentials* cred      = resolve_object_ptr(info.credentials_ptr, BT_CREDENTIALS);
    user_capabilities* caps     = resolve_object_ptr(info.capabilities_ptr, BT_CAPABILITIES);
    char* home                  = resolve_object_ptr(info.home_dir_ptr, BT_STRING);
    char* contact               = resolve_object_ptr(info.contact_string_ptr, BT_STRING);
    if(!cred || !caps || !home) throw std::runtime_error{ "user data is invalid" };
    new(addressof(out)) user_info
    {
        .uid            { info.uid },
        .gid            { info.gid },
        .credentials    { *cred },
        .capabilities   { *caps },
        .contact        { contact ? contact : "" },
        .home_directory { home, std::strnlen(home, 256) }
    };
}
void user_file::parse_index_entry(user_file_index_entry& ent)
{
    if(!__verify_block_csum(ent.block_idx)) klog("W: corruption in user account data file");
    else
    {
        if(ent.type == BT_INDEX)
        {
            user_file_index_entry* idx = blocks()[ent.block_idx].index_data;
            for(size_t i = 0; i < idx_per_block; i++) 
            {
                if(idx[i].type == BT_NULL)
                    break;
                parse_index_entry(idx[i]);
            }
        }
        else if(ent.type == BT_USER_INFO)
        {
            user_persisted_info* inf = blocks()[ent.block_idx].user_data;
            for(size_t i = 0; i < uinfos_per_block; i++)
            {
                if(inf[i].checksum == 0 && inf[i].uid == 0) break;
                user_credentials* cred = resolve_object_ptr(inf[i].credentials_ptr, BT_CREDENTIALS);
                if(!cred) klog("W: corruption detected in user data");
                else
                {
                    size_t blk_idx  = ent.block_idx;
                    uid_t uid       = inf[i].uid;
                    __uid_map.emplace(std::piecewise_construct, std::tuple<uid_t>(uid), std::tuple<size_t, size_t>(blk_idx, i));
                    size_t name_len = std::strnlen(cred->user_login_name, username_max_len);
                    __username_map.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(std::string(cred->user_login_name, name_len))), std::forward_as_tuple(uid));
                }
            }
        }
    }
}
void user_file::write_string(std::string const& str)
{
    size_t needed               = str.size() + 1;
    size_t rem                  = static_cast<size_t>(usable_block_size - get_superblock().next_available_string_slot.offset_in_block);
    if(needed > rem) 
        advance_object_slot(BT_STRING); // after this there shouldn't be a dangling reference problem
    user_file_superblock& sb    = get_superblock();
    user_file_object_ptr& slot  = sb.next_available_string_slot;
    char* target                = resolve_object_ptr(slot, BT_STRING);
    slot.offset_in_block        += needed;
    array_copy(target, str.c_str(), str.size());
    target[str.size()]          = '\0';
    __write_block_csum(slot.entry_block_idx);
    if(slot.offset_in_block >= usable_block_size) 
        advance_object_slot(BT_STRING);
}
void user_file::persist_user_info(user_info const& inf)
{
    user_file_superblock* sb    = addressof(get_superblock());
    uid_t uid                   = inf.uid;
    if(__uid_map.contains(uid)) { update_user_info(inf); }
    else
    {      
        user_file_object_ptr contact_str_slot, home_str_slot;
        size_t needed_home      = inf.home_directory.size() + 1;
        if(sb->next_available_string_slot.offset_in_block + needed_home > usable_block_size) { advance_object_slot(BT_STRING); sb = addressof(get_superblock()); }
        home_str_slot           = sb->next_available_string_slot;
        write_string(inf.home_directory);
        sb = addressof(get_superblock());
        if(!inf.contact.empty())
        {
            size_t needed_total = needed_home + inf.contact.size() + 1;
            if(sb->next_available_string_slot.offset_in_block + needed_total > usable_block_size) { advance_object_slot(BT_STRING); sb = addressof(get_superblock()); }
            contact_str_slot    = sb->next_available_string_slot;
            write_string(inf.contact);
            sb = addressof(get_superblock());
        }
        user_file_object_ptr caps_ptr = sb->next_available_caps_slot;
        user_file_object_ptr cred_ptr = sb->next_available_cred_slot;
        user_file_object_ptr info_ptr = sb->next_available_info_slot;
        new(resolve_object_ptr(caps_ptr, BT_CAPABILITIES)) user_capabilities(inf.capabilities);
        new(resolve_object_ptr(cred_ptr, BT_CREDENTIALS)) user_credentials(inf.credentials);
        __write_block_csum(caps_ptr.entry_block_idx);
        __write_block_csum(cred_ptr.entry_block_idx);
        user_persisted_info* persisted = new(resolve_object_ptr(info_ptr, BT_USER_INFO)) user_persisted_info
        {
            .uid                { inf.uid },
            .gid                { inf.gid },
            .credentials_ptr    { cred_ptr },
            .capabilities_ptr   { caps_ptr },
            .contact_string_ptr { contact_str_slot },
            .home_dir_ptr       { home_str_slot },
            .checksum           { 0 }  
        };
        uint32_t csum       = crc32c(crc32c(sb->system_uuid), *persisted);
        persisted->checksum = csum;
        __write_block_csum(info_ptr.entry_block_idx);
        advance_object_slot(BT_CAPABILITIES);
        advance_object_slot(BT_CREDENTIALS);
        advance_object_slot(BT_USER_INFO);
        __uid_map.emplace(std::piecewise_construct, std::tuple<uid_t>(uid), std::forward_as_tuple(info_ptr));
        size_t username_len = std::strnlen(inf.credentials.user_login_name, username_max_len);
        __username_map.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(std::string(inf.credentials.user_login_name, username_len))), std::forward_as_tuple(uid));
    }
    __file.force_write();
    if(!__file.fsync()) throw std::runtime_error{ "failed to write disk" };
}
void user_file::update_user_info(user_info const& inf)
{
    user_persisted_info& persisted  = resolve_object_ptr(__uid_map[inf.uid], BT_USER_INFO).ref<user_persisted_info>();
    user_capabilities* caps         = resolve_object_ptr(persisted.capabilities_ptr, BT_CAPABILITIES);
    user_credentials* creds         = resolve_object_ptr(persisted.credentials_ptr, BT_CREDENTIALS);
    if(!caps || !creds) 
        throw std::invalid_argument{ "user data has invalid pointers or is corrupted" };
    *caps                           = inf.capabilities;
    *creds                          = inf.credentials;
    __write_block_csum(persisted.capabilities_ptr.entry_block_idx);
    __write_block_csum(persisted.credentials_ptr.entry_block_idx);
}
void user_file::init_credentials(user_credentials& out, std::string const& username, std::string const& pw, time_t pw_change_interval, time_t pw_warning_interval, uint32_t login_req_interval)
{
    user_credentials* result = new(addressof(out)) user_credentials
    {
        .last_pw_change_time        = sys_time(nullptr),
        .pw_change_require_interval = pw_change_interval,
        .pw_change_warning_interval = pw_warning_interval,
        .login_require_interval     = login_req_interval,
        .checksum                   = 0U
    };
    if(!pw.empty())
    {
        std::string setting         = create_hash_setting_string();
        std::string hash_str        = create_crypto_string(pw, setting);
        array_copy(result->crypto_setting_str, setting.c_str(), crypto_setting_len);
        array_copy(result->password_hash_str, hash_str.c_str(), crypto_hash_len);
    }
    uint32_t csum       = crc32c(crc32c(get_superblock().system_uuid), *result);
    result->checksum    = csum;
}
void user_file::init_capabilities(user_capabilities& out, uint32_t perm_flags, uint32_t proc_quota, uint32_t thread_quota)
{
    user_capabilities* result = new(addressof(out)) user_capabilities
    {
        .thread_quota       { thread_quota },
        .process_quota      { proc_quota },
        .system_permissions { perm_flags },
        .checksum           { 0U }
    };
    uint32_t csum       = crc32c(crc32c(get_superblock().system_uuid), *result);
    result->checksum    = csum;
}
bool check_pw(user_info const& user, std::string const& pw)
{
    std::string setting_str(user.credentials.crypto_setting_str, crypto_setting_len);
    std::string check_str = create_crypto_string(pw, setting_str);
    return !std::strncmp(check_str.c_str(), user.credentials.password_hash_str, crypto_hash_len);
}
bool user_file::__initialize()
{
    if(__has_init) return true;
    try
    {
        if(__file.size())
        {
            user_file_index_entry* idx0 = blocks()[0].idx0;
            for(size_t i = 0; i < idx0_entries; i++)
            {
                if(idx0[i].type == BT_NULL)
                    break;
                parse_index_entry(idx0[i]);
            }
        }
        else
        {
            if(!__file.grow(user_file_block_size * 5)) { panic("failed to get disk space"); return false; }
            time_t ts = sys_time(nullptr);
            user_file_superblock* sb = new(addressof(get_superblock())) user_file_superblock
            {
                .magic                      { user_file_magic },
                .create_time                { ts },
                .last_update_time           { ts },
                .next_available_uid         { 100UL },
                .next_available_gid         { 100UL },
                .total_blocks               { 5UZ },
                .next_available_info_slot   { 1UZ, 0UZ },
                .next_available_cred_slot   { 2UZ, 0UZ },
                .next_available_caps_slot   { 3UZ, 0UZ },
                .next_available_string_slot { 4UZ, 0UZ },
                .next_available_index_slot  { 0UZ, initial_idx0_offset + 4 }
            };
            __fill_entropy(addressof(sb->system_uuid), sizeof(guid_t) / sizeof(uint64_t));
            uint32_t csum_seed          = crc32c(sb->system_uuid);
            user_file_index_entry* idx0 = blocks()[0].idx0;
            idx0[0].block_idx           = 1;
            idx0[0].type                = BT_USER_INFO;
            idx0[0].checksum            = 0;
            idx0[1].block_idx           = 2;
            idx0[1].type                = BT_CREDENTIALS;
            idx0[1].checksum            = 0;
            idx0[2].block_idx           = 3;
            idx0[2].type                = BT_CAPABILITIES;
            idx0[2].checksum            = 0;
            idx0[3].block_idx           = 4;
            idx0[3].type                = BT_STRING;
            idx0[3].checksum            = 0;
            uint32_t csum               = crc32c(csum_seed, idx0[0]);
            idx0[0].checksum            = csum;
            csum                        = crc32c(csum_seed, idx0[1]);
            idx0[1].checksum            = csum;
            csum                        = crc32c(csum_seed, idx0[2]);
            idx0[2].checksum            = csum;
            csum                        = crc32c(csum_seed, idx0[3]);
            idx0[3].checksum            = csum;
            __write_block_csum(0UZ);
            __file.force_write();
            if(!__file.fsync()) { panic("failed to write user file"); return false; }
        }
    }
    catch(std::exception& e) { panic(e.what()); return false; }
    return (__has_init = true);
}