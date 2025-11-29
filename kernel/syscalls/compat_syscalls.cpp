#include "users.hpp"
#include "sys/errno.h"
static void init_pwd(unix_pwd* out, vpwd_entry* data)
{
	new(out) unix_pwd
	{
		.pw_name	{ data->credentials.user_login_name },
		.pw_passwd	{ data->dummy_passwd },
		.pw_uid		{ data->uid },
		.pw_gid		{ data->gid },
		.pw_comment	{ data->gecos_home_shell },
		.pw_gecos	{ data->gecos_home_shell },
		.pw_dir		{ std::addressof(data->gecos_home_shell[data->gecos_size + 1]) },
		.pw_shell	{ std::addressof(data->gecos_home_shell[data->gecos_size + data->home_size + 2]) }
	};
}
extern "C"
{
	int syscall_getvpwuid(uid_t uid, unix_pwd* out)
	{
		task_ctx* task	= active_task_context();
		out				= translate_user_pointer(out);
		if(__unlikely(!task || !out)) return -EFAULT;
		if(__unlikely(!user_accounts_manager::is_initialized())) return -ENOSYS;
		user_accounts_manager* uam	= user_accounts_manager::get_instance();
		try
		{
			vpwd_entry* data		= uam->get_vpwd_entry(uid, *task);
			if(__unlikely(!data)) return -ENOMEM;
			init_pwd(out, data);
		}
		catch(std::invalid_argument&)	{ return -EINVAL; }
		catch(std::out_of_range&)		{ return -ENOENT; }
		return 0;
	}
	int syscall_getvpwnam(const char* name, unix_pwd* out)
	{
		task_ctx* task	= active_task_context();
		name			= translate_user_pointer(name);
		out				= translate_user_pointer(out);
		if(__unlikely(!task || !name || !out)) return -EFAULT;
		if(__unlikely(!user_accounts_manager::is_initialized())) return -ENOSYS;
		user_accounts_manager* uam	= user_accounts_manager::get_instance();
		try
		{
			vpwd_entry* data		= uam->get_vpwd_entry(name, *task);
			if(__unlikely(!data)) return -ENOMEM;
			init_pwd(out, data);
		}
		catch(std::invalid_argument&)	{ return -EINVAL; }
		catch(std::out_of_range&)		{ return -ENOENT; }
		return 0;
	}
	int syscall_getvpwent(unix_pwd* ent)
	{
		task_ctx* task	= active_task_context();
		ent			 	= translate_user_pointer(ent);
		if(__unlikely(!task || !ent)) return -EFAULT;
		if(__unlikely(!user_accounts_manager::is_initialized())) return -ENOSYS;
		user_accounts_manager* uam	= user_accounts_manager::get_instance();
		try
		{
			if(ent->pw_name)
			{
				vpwd_entry* data	= uam->next_vpwd_entry(ent->pw_uid, *task);
				if(data) init_pwd(ent, data);
				else return 1;
			}
			else
			{
				vpwd_entry* data	= uam->first_vpwd_entry(*task);
				if(__unlikely(!data)) return 1;
				init_pwd(ent, data);
			}
		}
		catch(std::invalid_argument&)	{ return -EINVAL; }
		catch(std::bad_alloc&)			{ return -ENOMEM; }
		return 0;
	}
}