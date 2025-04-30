#include "users.hpp"
#include "stdexcept"
using std::addressof;
user_file::user_file(file_node& file) : __file(file), __username_map{ 256 } {}
user_file_block_data* user_file::blocks() { return reinterpret_cast<user_file_block_data*>(__file.data()); }
size_t user_file::num_blocks() { return __file.size() / user_file_block_size; }
user_file_superblock& user_file::get_superblock() { return blocks()[0].superblock; }
addr_t user_file::resolve_object_ptr(user_file_object_ptr const& p, user_file_block_type type)
{
    if(!p.entry_block_idx) return nullptr;
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
    case BT_UID_INDEX:
        return addressof(block.uid_lookup_data[p.offset_in_block]);
    default:
        return nullptr;
    }
}
user_persisted_info& user_file::get_user_data(uid_t uid)
{
    if(uid > get_superblock().lookup_table_blocks * ptrs_per_lookup_block)
        throw std::out_of_range{ "no user with that ID exists yet" };
    user_file_object_ptr& obj_ptr = blocks()[uid / ptrs_per_lookup_block].uid_lookup_data[uid % ptrs_per_lookup_block];
    if(!obj_ptr.entry_block_idx)
        throw std::out_of_range{ "no user with that ID exists" };
    return resolve_object_ptr(obj_ptr, BT_USER_INFO).ref<user_persisted_info>();
}
void user_file::get_user_info(user_info* out)
{
    if(!out) throw std::invalid_argument{ "output must not be null" };
    user_persisted_info& info = get_user_data(out->uid);
    user_credentials* cred = resolve_object_ptr(info.credentials_ptr, BT_CREDENTIALS);
    user_capabilities* caps = resolve_object_ptr(info.capabilities_ptr, BT_CAPABILITIES);
    char* home = resolve_object_ptr(info.home_dir_ptr, BT_STRING);
    char* contact = resolve_object_ptr(info.contact_string_ptr, BT_STRING);
    if(!contact) contact = "";
    if(!cred || !caps || !home) throw std::runtime_error{ "user data is invalid" };
    new(out) user_info
    {
        .uid            { info.uid },
        .gid            { info.gid },
        .credentials    { *cred },
        .capabilities   { *caps },
        .contact        { contact, std::strnlen(contact, 256) },
        .home_directory { home, std::strnlen(home, 256) }
    };
}