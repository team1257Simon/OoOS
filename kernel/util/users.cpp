#include "users.hpp"
#include "ow-crypt.h"
static char account_manager_space[sizeof(user_accounts_manager)]{};
user_accounts_manager* user_accounts_manager::__instance{};
static inline sysfs_vnode& get_global_account_info(sysfs& s);
void user_info::compute_csum() { checksum = crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(this), offsetof(user_info, checksum)); }
bool user_info::verify_csum() const { return checksum == crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(this), offsetof(user_info, checksum)); }
user_accounts_manager::user_accounts_manager(sysfs& config_src) : __sysfs(config_src), __table(config_src, "user_accounts", sysfs_object_type::USER_INFO), __global_info(get_global_account_info(config_src)) {}
user_accounts_manager::user_accounts_manager(sysfs& config_src, uint32_t table_ino) : __sysfs(config_src), __table(config_src.open(table_ino)), __global_info(get_global_account_info(config_src)) {}
user_accounts_manager* user_accounts_manager::get_instance() { return __instance; }
bool user_accounts_manager::user_exists(std::string const& username) { return __table.contains(username.c_str()); }
bool user_accounts_manager::can_create_user(std::string const& name) { return name.size() < username_max_len && !user_exists(name); }
global_info_handle user_accounts_manager::global_configs() { return global_info_handle(__global_info); }
user_handle user_accounts_manager::get_user(std::string const& name) { return __table.get(name.c_str()); }
bool user_accounts_manager::persist() { return __sysfs.sync(); }
bool user_info::check_pw(std::string const& pw) const
{
    std::string setting_str(credentials.crypto_setting_str, crypto_setting_len);
    std::string check_str = create_crypto_string(pw, setting_str);
    return !std::strncmp(check_str.c_str(), credentials.password_hash_str, crypto_hash_len);
}
static inline sysfs_vnode& get_global_account_info(sysfs& s)
{
    uint32_t n  = s.find_node("accounts_meta");
    if(n) return s.open(n);
    n           = s.mknod("accounts_meta", sysfs_object_type::GENERAL_CONFIG);
    if(!n) throw std::runtime_error("[account config] failed to create accounts metadata object");
    return s.open(n);
}
void user_accounts_manager::init_instance(sysfs& config_src)
{
    uint32_t n          = config_src.find_node("user_accounts");
    if(n) __instance    = new(account_manager_space) user_accounts_manager(config_src, n);
    else
    {
        __instance                          = new(account_manager_space) user_accounts_manager(config_src);
        user_info root_info
        {
            .uid            { 0UL },
            .gid            { 0UL },
            .credentials    { .user_login_name      { "root" } },
            .capabilities   { .system_permissions   { ~0UL } },
            .home_directory { "/root" }
        };
        user_handle root_handle             = __instance->__table.add(root_info).first;
        global_accounts_info& inf = *__instance->__global_info;
        inf.next_gid = 100UL;
        inf.next_uid = 1000UL;
        inf.num_users++;
        inf.num_groups++;
        sys_time(std::addressof(inf.last_updated));
        root_handle->compute_csum();
        __instance->__global_info.commit_object();
        config_src.sync();
    }
}
user_handle user_accounts_manager::create_account(std::string const& name, std::string const& password, uint64_t permission_flags)
{
    if(!can_create_user(name)) throw std::invalid_argument("[account config] username " + name + " already exists or is invalid");
    std::string crypto_settings = create_hash_setting_string();
    std::string pass_hash = create_crypto_string(password, crypto_settings);
    std::string homedir = "/home/" + name;
    global_accounts_info& inf = *__instance->__global_info;
    user_info user
    {
        .uid                            { inf.next_uid },
        .gid                            { inf.next_gid },
        .credentials
        {
            .login_require_interval     { inf.login_require_interval_default },
            .forced_logout_interval     { inf.forced_logout_interval_default },
            .pw_change_require_interval { inf.pw_change_require_interval_default },
            .pw_change_warning_interval { inf.pw_change_warning_interval_default }
        },
        .capabilities
        {
            .thread_quota               { inf.thread_quota_default },
            .process_quota              { inf.process_quota_default },
            .disk_quota                 { inf.disk_quota_default },
            .system_permissions         { permission_flags }
        }
    };
    std::strncpy(user.credentials.user_login_name, name.c_str(), name.size());
    std::strncpy(user.credentials.crypto_setting_str, crypto_settings.c_str(), crypto_settings.size());
    std::strncpy(user.credentials.password_hash_str, pass_hash.c_str(), pass_hash.size());
    std::strncpy(user.home_directory, homedir.c_str(), homedir.size());
    user.compute_csum();
    inf.num_groups++;
    inf.num_users++;
    inf.next_gid++;
    inf.next_uid++;
    __global_info.commit_object();
    user_handle result = __table.add(user).first;
    __sysfs.sync();
    return result;
}