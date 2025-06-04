#ifndef __USER_INFO
#define __USER_INFO
#include "sys/types.h"
#include "string"
constexpr size_t username_max_len               = 32UZ;
constexpr size_t homedir_max_len                = 64UZ;
constexpr size_t crypto_setting_len             = 29UZ;
constexpr size_t crypto_hash_len                = 31UZ;
constexpr uint64_t create_group                 = 0x1;
constexpr uint64_t create_user                  = 0x2;
constexpr uint64_t delete_group                 = 0x4;
constexpr uint64_t delete_user                  = 0x8;
constexpr uint64_t change_group_capabilities    = 0x10;
constexpr uint64_t change_user_capabilities     = 0x20;
constexpr uint64_t alter_group_membership       = 0x40;
constexpr uint64_t change_file_ownership        = 0x80;
constexpr uint64_t create_environment_variable  = 0x100;
constexpr uint64_t delete_environment_variable  = 0x200;
constexpr uint64_t change_file_modes            = 0x400;
constexpr uint64_t change_directory_modes       = 0x800;
constexpr uint64_t install_module               = 0x1000;
constexpr uint64_t remove_module                = 0x2000;
constexpr uint64_t start_daemon                 = 0x4000;
constexpr uint64_t stop_daemon                  = 0x8000;
struct __pack user_credentials
{
    char        user_login_name[username_max_len];
    char        crypto_setting_str[crypto_setting_len];
    char        password_hash_str[crypto_hash_len];
    time_t      login_require_interval;
    time_t      forced_logout_interval;
    time_t      last_login_time;
    time_t      last_pw_change_time;
    time_t      pw_change_require_interval;
    time_t      pw_change_warning_interval;
};
struct __pack user_capabilities
{
    size_t      thread_quota;
    size_t      process_quota;
    size_t      disk_quota;
    uint64_t    system_permissions;
};
struct __pack user_info
{
    uid_t                   uid;
    gid_t                   gid;
    user_credentials        credentials;
    user_capabilities       capabilities;
    char                    home_directory[homedir_max_len];
    uint32_t                checksum;
    bool check_pw(std::string const& pw) const;
    void compute_csum();
    bool verify_csum() const;
};
#endif