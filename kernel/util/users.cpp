#include "users.hpp"
#include "ow-crypt.h"
#include "sys/errno.h"
constexpr std::align_val_t align_data = static_cast<std::align_val_t>(alignof(vpwd_entry));
static char account_manager_space[sizeof(user_accounts_manager)]{};
user_accounts_manager* user_accounts_manager::__instance{};
bool user_accounts_manager::__has_init{};
static inline sysfs_vnode& get_global_account_info(sysfs& s);
static addr_t alloc_sys_vpwd(size_t n) { return ::operator new(n, align_data); }
static addr_t alloc_user_vpwd(size_t n, task_ctx const& ctx);
void user_info::compute_csum() { checksum = crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(this), offsetof(user_info, checksum)); }
bool user_info::verify_csum() const { return checksum == crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(this), offsetof(user_info, checksum)); }
vpwd_entry* user_accounts_manager::__get_vpwd_entry(user_info const& user, task_ctx& context) { return __load_vpwd_data(user, std::bind(alloc_user_vpwd, std::placeholders::_1, context)); }
user_accounts_manager* user_accounts_manager::get_instance() { return __instance; }
bool user_accounts_manager::is_initialized() { return __has_init; }
bool user_accounts_manager::user_exists(std::string const& username) { return __table.contains(username.c_str()); }
bool user_accounts_manager::can_create_user(std::string const& name) { return name.size() < username_max_len && !user_exists(name); }
global_info_handle user_accounts_manager::global_configs() { return global_info_handle(__global_info); }
user_handle user_accounts_manager::get_user(std::string const& name) { return __table.get(name.c_str()); }
bool user_accounts_manager::persist() { return __sysfs.sync(); }
user_accounts_manager::user_accounts_manager(sysfs& config_src) :
	__sysfs			{ config_src },
	__global_info	{ get_global_account_info(config_src) },
	__table			{ config_src, "user_accounts", sysfs_object_type::USER_INFO },
	__uid_index_map {},
	__vpwd			{ config_src.open(__global_info->vpwd_ino) }
					{}
user_accounts_manager::user_accounts_manager(sysfs& config_src, uint32_t table_ino) :
	__sysfs			{ config_src },
	__global_info	{ get_global_account_info(config_src) },
	__table			{ config_src.open(table_ino) },
	__uid_index_map {},
	__vpwd			{ config_src.open(__global_info->vpwd_ino) }
					{
						size_t total_users		= __table.size();
						user_info* table_data	= __table.data();
						for(size_t i = 0; i < total_users; i++)
							__uid_index_map.insert(std::make_pair(static_cast<uid_t>(table_data[i].uid), static_cast<size_t>(i + 1)));
					}
static addr_t alloc_user_vpwd(size_t n, task_ctx const& ctx)
{
	uframe_tag* frame	= ctx.task_struct.frame_ptr;
	addr_t raw			= frame->sysres_add(n);
	kmm.enter_frame(frame);
	addr_t result(kmm.frame_translate(raw));
	kmm.exit_frame();
	return result;
}
user_handle user_accounts_manager::get_user(uid_t uid)
{
	std::map<uid_t, size_t>::iterator result = __uid_index_map.find(uid);
	if(result != __uid_index_map.end()) return user_handle(__table, result->second);
	throw std::out_of_range("[UAM] no user with uid " + std::to_string(uid));
}
bool user_info::check_pw(std::string const& pw) const
{
	std::string setting_str(credentials.crypto_setting_str, crypto_setting_len);
	std::string check_str = create_crypto_string(pw, setting_str);
	return !std::strncmp(check_str.c_str(), credentials.password_hash_str, crypto_hash_len);
}
static inline sysfs_vnode& get_global_account_info(sysfs& s)
{
	uint32_t n			= s.find_node("accounts_meta");
	if(n) return s.open(n);
	n					= s.mknod("accounts_meta", sysfs_object_type::GENERAL_CONFIG);
	if(!n) throw std::runtime_error("[UAM] failed to create accounts metadata object");
	sysfs_vnode& result = s.open(n);
	global_info_handle inf(result);
	n					= s.mknod("vpwd", sysfs_object_type::STRING_TABLE);
	if(!n) throw std::runtime_error("[UAM] failed to create string table object");
	inf->vpwd_ino		= n;
	return result;
}
bool user_accounts_manager::init_instance(sysfs& config_src)
{
	if(__unlikely(__has_init)) return true;
	try
	{
		uint32_t n			= config_src.find_node("user_accounts");
		if(n) __instance	= new(account_manager_space) user_accounts_manager(config_src, n);
		else
		{
			__instance		= new(account_manager_space) user_accounts_manager(config_src);
			user_info root_info
			{
				.uid			{ 0U },
				.gid			{ 0U },
				.credentials	{ .user_login_name		{ "root" } },
				.capabilities	{ .system_permissions	{ static_cast<permission_flag>(-1) } }
			};
			user_handle root_handle		= __instance->__table.add(root_info).first;
			root_handle->compute_csum();
			global_accounts_info& inf	= *__instance->__global_info;
			inf.next_gid				= 1000U;
			inf.next_uid				= 1000U;
			inf.next_service_uid		= 1U;
			inf.next_service_gid		= 1U;
			inf.num_users++;
			inf.num_groups++;
			inf.last_updated			= sys_time(nullptr);
			__instance->__global_info.commit_object();
			config_src.sync();
		}
	}
	catch(std::exception& e) { panic(e.what()); return false; }
	return (__has_init = true);
}
int user_accounts_manager::__create_credentials(user_info& out, std::string const& name, std::string const& pw)
{
	if(__unlikely(name.size() > username_max_len)) return -ENAMETOOLONG;
	if(__unlikely(user_exists(name))) return -EEXIST;
	std::strncpy(out.credentials.user_login_name, name.c_str(), std::min(name.size(), username_max_len));
	set_pw(out, pw);
	return 0;
}
void user_accounts_manager::set_pw(user_info& out, std::string const& pw)
{
	std::string setting = create_hash_setting_string();
	std::string pw_hash = create_crypto_string(pw, setting);
	std::strncpy(out.credentials.crypto_setting_str, setting.c_str(), crypto_setting_len);
	std::strncpy(out.credentials.password_hash_str, pw_hash.c_str(), crypto_hash_len);
	out.last_pw_change_time = sys_time(nullptr);
	out.compute_csum();
}
int user_accounts_manager::__create_vpwd_records(user_info& user, const char* home, const char* gecos, const char* shell)
{
	std::string uname_str(user.credentials.user_login_name, std::strnlen(user.credentials.user_login_name, username_max_len));
	off_t gecos_ptr			= __vpwd.write(gecos ? std::string(gecos, std::strnlen(gecos, 1024UZ)) : uname_str);
	off_t home_ptr			= __vpwd.write(home ? std::string(home, std::strnlen(home, 1024UZ)) : ("/home/" + uname_str));
	off_t shell_ptr			= __vpwd.write(shell ? std::string(shell, std::strnlen(shell, 1024UZ)) : std::string("/bin/sh"));
	if(__unlikely(gecos_ptr < 0 || home_ptr < 0 || shell_ptr < 0)) return -ENOSPC;
	user.vpwd_gecos			= gecos_ptr;
	user.vpwd_home			= home_ptr;
	user.vpwd_shell			= shell_ptr;
	user.compute_csum();
	return 0;
}
int user_accounts_manager::create_user(std::string const& name, std::string const& pw, const char* shell, const char* gecos, const char* home, user_capabilities* caps)
{
	global_accounts_info& inf 	= *__global_info;
	user_info account
	{
		.uid			{ inf.next_uid },
		.gid			{ inf.next_gid },
		.capabilities	{ nonnull_or_else(caps, inf.default_permissions) }
	};
	int err 					= __create_credentials(account, name, pw);
	if(__unlikely(err != 0)) return err;
	user_handle result			= __table.add(account).first;
	err				 			= __create_vpwd_records(*result, home, gecos, shell);
	if(__unlikely(err != 0)) return err;
	inf.next_uid++;
	inf.next_gid++;
	inf.num_users++;
	inf.num_groups++;
	inf.last_updated 			= sys_time(nullptr);
	__global_info.commit_object();
	return !__sysfs.sync() ? -EIO : 0;
}
int user_accounts_manager::create_service_account(std::string const& name, std::string const& pw, const char* shell, const char* gecos, const char* home, user_capabilities* caps)
{
	global_accounts_info& inf 	= *__global_info;
	user_info account
	{
		.uid			{ inf.next_service_uid },
		.gid			{ inf.next_service_gid },
		.capabilities	{ nonnull_or_else(caps, inf.default_permissions) }
	};
	int err = __create_credentials(account, name, pw);
	if(__unlikely(err != 0)) return err;
	user_handle result			= __table.add(account).first;
	err				 			= __create_vpwd_records(*result, home, gecos, shell ? shell : "/sbin/nologin");
	if(__unlikely(err != 0)) return err;
	inf.next_service_uid++;
	inf.next_service_gid++;
	inf.num_users++;
	inf.num_groups++;
	inf.last_updated 			= sys_time(nullptr);
	__global_info.commit_object();
	return !__sysfs.sync() ? -EIO : 0;
}
vpwd_entry* user_accounts_manager::__load_vpwd_data(user_info const& user, std::function<addr_t(size_t)> const& alloc_fn)
{
	std::string gecos			= __vpwd.read(user.vpwd_gecos);
	std::string home			= __vpwd.read(user.vpwd_home);
	std::string sh				= __vpwd.read(user.vpwd_shell);
	size_t str_size				= gecos.size() + home.size() + sh.size() + 3UZ;
	size_t total_size			= sizeof(vpwd_entry) + str_size;
	void* out					= alloc_fn(total_size);
	if(__unlikely(!out)) return nullptr;
	vpwd_entry* result			= new(out) vpwd_entry
	{
		.entry_size				{ total_size },
		.gecos_size				{ gecos.size() },
		.home_size				{ home.size() },
		.shell_size				{ sh.size() },
		.uid					{ user.uid },
		.gid					{ user.gid },
		.credentials			{ user.credentials },
		.capabilities			{ user.capabilities },
		.last_login_time		{ user.last_login_time },
		.last_pw_change_time	{ user.last_pw_change_time }
	};
	char* copy_target			= result->gecos_home_shell;
	std::strncpy(copy_target, gecos.c_str(), gecos.size());
	copy_target					+= gecos.size();
	*copy_target				= '\0';
	copy_target++;
	std::strncpy(copy_target, home.c_str(), home.size());
	copy_target					+= home.size();
	*copy_target				= '\0';
	copy_target++;
	std::strncpy(copy_target, sh.c_str(), sh.size());
	*copy_target				= '\0';
	return result;
}
int user_accounts_manager::__get_vpwd_entry(user_info const& user, std::function<int(vpwd_entry&)> const& callback)
{
	vpwd_entry* data	= __load_vpwd_data(user, alloc_sys_vpwd);
	if(__unlikely(!data)) return -ENOMEM;
	int result			= callback(*data);
	::operator delete(data, data->entry_size, align_data);
	return result;
}
vpwd_entry* user_accounts_manager::get_vpwd_entry(uid_t uid, task_ctx& context)
{
	user_handle handle	= get_user(uid);
	user_info& info		= *handle;
	handle.release();
	return __get_vpwd_entry(info, context);
}
vpwd_entry* user_accounts_manager::get_vpwd_entry(std::string const& name, task_ctx& context)
{
	user_handle handle	= get_user(name);
	user_info& info		= *handle;
	handle.release();
	return __get_vpwd_entry(info, context);
}
int user_accounts_manager::get_vpwd_entry(uid_t uid, std::function<int(vpwd_entry&)> const& callback)
{
	user_handle handle	= get_user(uid);
	user_info& info		= *handle;
	handle.release();
	return __get_vpwd_entry(info, callback);
}
int user_accounts_manager::get_vpwd_entry(std::string const& name, std::function<int(vpwd_entry&)> const& callback)
{
	user_handle handle	= get_user(name);
	user_info& info		= *handle;
	handle.release();
	return __get_vpwd_entry(info, callback);
}
vpwd_entry* user_accounts_manager::first_vpwd_entry(task_ctx& context)
{
	if(__unlikely(!__table.size())) return nullptr;
	user_handle handle(__table, 1UZ);
	user_info& info		= *handle;
	handle.release();
	vpwd_entry* result	= __get_vpwd_entry(info, context);
	if(!result) throw std::bad_alloc();
	return result;
}
vpwd_entry* user_accounts_manager::next_vpwd_entry(uid_t prev, task_ctx& context)
{
	user_handle handle		= get_user(prev);
	if(handle.value_index < __table.size())
	{
		user_info& info		= *(++handle);
		handle.release();
		vpwd_entry* result	= __get_vpwd_entry(info, context);
		if(!result) throw std::bad_alloc();
		return result;
	}
	handle.release();
	return nullptr;
}