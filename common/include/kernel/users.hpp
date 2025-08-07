#ifndef __USER_INFO
#define __USER_INFO
#include "sys/types.h"
#include "fs/sysfs.hpp"
#include "sched/task_ctx.hpp"
#include "map"
typedef uint32_t permission_flag;
constexpr size_t username_max_len                      = 32UZ;
constexpr size_t crypto_setting_len                    = 29UZ;
constexpr size_t crypto_hash_len                       = 31UZ;
constexpr size_t user_groups_array_len                 = 23UZ;
constexpr gid_t end_of_list                            = static_cast<gid_t>(-1);
constexpr permission_flag create_delete_group          = 0x1;
constexpr permission_flag create_delete_user           = 0x2;
constexpr permission_flag change_group_capabilities    = 0x4;
constexpr permission_flag change_user_capabilities     = 0x8;
constexpr permission_flag alter_group_membership       = 0x10;
constexpr permission_flag change_file_ownership        = 0x20;
constexpr permission_flag create_delete_env            = 0x40;
constexpr permission_flag change_file_modes            = 0x80;
constexpr permission_flag change_directory_modes       = 0x100;
constexpr permission_flag install_remove_module        = 0x200;
constexpr permission_flag start_stop_daemon            = 0x400;
constexpr permission_flag change_setting_defaults      = 0x800;
constexpr permission_flag is_service_account           = 0x1000;
struct __pack user_credentials
{
    char        user_login_name[username_max_len];
    char        crypto_setting_str[crypto_setting_len];
    char        password_hash_str[crypto_hash_len];
    gid_t       groups[user_groups_array_len];
};
struct __pack user_capabilities
{
    int                 login_require_interval;
    int                 forced_logout_interval;
    int                 pw_change_require_interval;
    int                 pw_change_warning_interval;
    permission_flag     system_permissions;
};
struct __pack global_accounts_info
{
    time_t last_updated;
    size_t num_users;
    size_t num_groups;
    uint32_t vpwd_ino;  // the vpwd, or virtual passwd file, is a string table that contains unix-specified user info fields of variable string length
    uid_t next_uid;
    gid_t next_gid;
    uid_t next_service_uid;
    gid_t next_service_gid;
    user_capabilities default_permissions;
};
struct __pack user_info
{
    uid_t                   uid;
    gid_t                   gid;
    off_t                   vpwd_gecos;
    off_t                   vpwd_home;
    off_t                   vpwd_shell;
    user_credentials        credentials;
    user_capabilities       capabilities;
    time_t                  last_login_time;
    time_t                  last_pw_change_time;
    uint32_t                checksum;
    bool check_pw(std::string const& pw) const;
    void compute_csum();
    bool verify_csum() const;
};
struct vpwd_entry
{
    size_t                  entry_size;
    size_t                  gecos_size;
    size_t                  home_size;
    size_t                  shell_size;
    uid_t                   uid;
    gid_t                   gid;
    user_credentials        credentials;
    user_capabilities       capabilities;
    char                    dummy_passwd[2] = "x";
    time_t                  last_login_time;
    time_t                  last_pw_change_time;
    char                    gecos_home_shell[];
};
struct unix_pwd
{
	char*       pw_name;		            /* user name */
	char*       pw_passwd;		            /* encrypted password */
	uid_t	    pw_uid;			            /* user uid */
	gid_t	    pw_gid;			            /* user gid */
	char*       pw_comment;	                /* comment */
	char*       pw_gecos;		            /* Honeywell login info */
	char*       pw_dir;		                /* home directory */
	char*       pw_shell;		            /* default shell */
};
struct username_extract { constexpr const char* operator()(user_info const& inf) const noexcept { return inf.credentials.user_login_name; } };
struct username_hash { constexpr size_t operator()(const char data[username_max_len]) const noexcept { uint32_t h = 5381U; for(size_t i = 0; i < username_max_len && data[i]; i++) h += static_cast<uint8_t>(data[i]) + (h << 5); return h; } };
struct username_equals { constexpr bool operator()(const char n1[username_max_len], const char n2[username_max_len]) const noexcept { return std::strncmp(n1, n2, username_max_len) == 0; } };
typedef sysfs_hash_table<const char*, user_info, username_extract, username_hash, username_equals> user_accounts_table;
typedef sysfs_object_handle<global_accounts_info> global_info_handle;
typedef user_accounts_table::value_handle user_handle;
class user_accounts_manager
{
    static user_accounts_manager* __instance;
    static bool __has_init;
    sysfs& __sysfs;
    global_info_handle __global_info;
    user_accounts_table __table;
    std::map<uid_t, size_t> __uid_index_map;
    sysfs_string_table __vpwd;
    user_accounts_manager(sysfs& config_src);
    user_accounts_manager(sysfs& config_src, uint32_t table_ino);
    vpwd_entry* __load_vpwd_data(user_info const& user, std::function<addr_t(size_t)> const& alloc_fn);
    int __create_credentials(user_info& out, std::string const& name, std::string const& pw);
    int __create_vpwd_records(user_info& user, const char* home, const char* gecos, const char* shell);
    vpwd_entry* __get_vpwd_entry(user_info const& user, task_ctx& context);
    int __get_vpwd_entry(user_info const& user, std::function<int(vpwd_entry&)> const& callback);
public:
    static bool init_instance(sysfs& config_src);
    static bool is_initialized();
    static user_accounts_manager* get_instance();
    bool user_exists(std::string const& username);
    bool can_create_user(std::string const& name);
    global_info_handle global_configs();
    user_handle get_user(std::string const& name);
    user_handle get_user(uid_t uid);
    void set_pw(user_info& out, std::string const& pw);
    int create_user(std::string const& name, std::string const& pw, const char* shell = nullptr, const char* gecos = nullptr, const char* home = nullptr, user_capabilities* caps = nullptr);
    int create_service_account(std::string const& name, std::string const& pw, const char* shell = nullptr, const char* gecos = nullptr, const char* home = nullptr, user_capabilities* caps = nullptr);
    vpwd_entry* get_vpwd_entry(uid_t uid, task_ctx& context);
    vpwd_entry* get_vpwd_entry(std::string const& name, task_ctx& context);
    vpwd_entry* first_vpwd_entry(task_ctx& context);
    vpwd_entry* next_vpwd_entry(uid_t prev, task_ctx& context);
    int get_vpwd_entry(uid_t uid, std::function<int(vpwd_entry&)> const& callback);
    int get_vpwd_entry(std::string const& name, std::function<int(vpwd_entry&)> const& callback);
    bool persist();
};
extern "C"
{
    int syscall_getvpwuid(uid_t uid, unix_pwd* out);
    int syscall_getvpwnam(const char* name, unix_pwd* out);
    int syscall_getvpwent(unix_pwd* ent);
}
#endif