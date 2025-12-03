#include <users.hpp>
extern "C"
{
	static uid_t do_impersonate(user_handle const& current, user_handle const& target, const char* pass, task_ctx* task)
	{
		permission_flag cperms		= current->capabilities.system_permissions;
		if(!(cperms & impersonate_any_uid))
		{
			if(__unlikely(!pass)) return static_cast<uid_t>(-EPERM);
			pass					= translate_user_pointer(pass);
			if(__unlikely(!pass)) return static_cast<uid_t>(-EFAULT);
			std::string pstr(pass);
			if(!target->check_pw(pstr)) return static_cast<uid_t>(-EACCES);
		}
		uid_t uid 							= target->uid;
		gid_t gid							= target->gid;
		task->impersonate					= true;
		task->imp_uid						= uid;
		task->imp_gid						= gid;
		task->task_struct.saved_regs.rdx	= static_cast<register_t>(gid);
		return uid;
	}
	uid_t syscall_login(const char* name, const char* pass)
	{
		task_ctx* task	= active_task_context();
		if(__unlikely(!task || !user_accounts_manager::is_initialized())) return static_cast<uid_t>(-ENOSYS);
		name			= translate_user_pointer(name);
		pass			= translate_user_pointer(pass);
		if(__unlikely(!name || !pass)) return static_cast<uid_t>(-EFAULT);
		try
		{
			std::string ustring(name), pstring(pass);
			user_handle user	= user_accounts_manager::get_instance()->get_user(ustring);
			if(user->check_pw(pstring))
			{
				uid_t uid							= user->uid;
				gid_t gid							= user->gid;
				task->uid(uid);
				task->gid(gid);
				task->task_struct.saved_regs.rdx	= static_cast<register_t>(gid);
				return uid;
			}
			return static_cast<uid_t>(-EPERM);
		}
		catch(std::out_of_range& e) { panic(e.what()); return static_cast<uid_t>(-EINVAL); }
		catch(std::bad_alloc&) { return static_cast<uid_t>(-ENOMEM); }
	}
	uid_t syscall_impersonate(const char* name, const char* pass)
	{
		task_ctx* task					= active_task_context();
		if(__unlikely(!task || !user_accounts_manager::is_initialized())) return static_cast<uid_t>(-ENOSYS);
		try
		{
			user_accounts_manager& inst	= *user_accounts_manager::get_instance();
			user_handle current			= inst.get_user(task->euid());
			if(!name) return do_impersonate(current, inst.get_user(0U), pass, task);
			name						= translate_user_pointer(name);
			if(__unlikely(!name)) return static_cast<uid_t>(-EFAULT);
			std::string ustring(name);
			return do_impersonate(current, inst.get_user(ustring), pass, task);
		}
		catch(std::out_of_range& e) { panic(e.what()); return static_cast<uid_t>(-EINVAL); }
		catch(std::bad_alloc&) { return static_cast<uid_t>(-ENOMEM); }
	}
	int syscall_setuid(uid_t id)
	{
		task_ctx* task					= active_task_context();
		if(__unlikely(!task || !user_accounts_manager::is_initialized())) return static_cast<uid_t>(-ENOSYS);
		try
		{
			user_accounts_manager& inst	= *user_accounts_manager::get_instance();
			user_handle current			= inst.get_user(task->uid());
			user_handle target			= inst.get_user(id);
			permission_flag cperms		= current->capabilities.system_permissions;
			if(__unlikely(!(cperms & impersonate_any_uid))) return -EPERM;
			task->euid(id);
			return 0;
		}
		catch(std::out_of_range& e) { panic(e.what()); return static_cast<uid_t>(-EINVAL); }
		catch(std::bad_alloc&) { return static_cast<uid_t>(-ENOMEM); }
	}
	int syscall_setgid(gid_t id)
	{
		task_ctx* task					= active_task_context();
		if(__unlikely(!task || !user_accounts_manager::is_initialized())) return static_cast<uid_t>(-ENOSYS);
		try
		{
			user_accounts_manager& inst	= *user_accounts_manager::get_instance();
			user_handle current			= inst.get_user(task->uid());
			permission_flag cperms		= current->capabilities.system_permissions;
			if(!(cperms & impersonate_any_gid))
			{
				for(size_t i = 0; i < user_groups_array_len; i++)
					if(current->credentials.groups[i] == id)
						goto success;
				return -EPERM;
			}
		success:
			task->egid(id);
			return 0;
		}
		catch(std::out_of_range& e) { panic(e.what()); return static_cast<uid_t>(-EINVAL); }
		catch(std::bad_alloc&) { return static_cast<uid_t>(-ENOMEM); }
	}
	uid_t syscall_urevert()
	{
		task_ctx* task						= active_task_context();
		if(__unlikely(!task)) return static_cast<uid_t>(-ENOSYS);
		task->impersonate					= false;
		task->imp_uid						= uid_undef;
		task->imp_gid						= gid_undef;
		task->task_struct.saved_regs.rdx	= static_cast<register_t>(task->gid());
		return task->uid();
	}
	int syscall_escalate(const char* pass)
	{
		task_ctx* task					= active_task_context();
		if(__unlikely(!task || !user_accounts_manager::is_initialized())) return -ENOSYS;
		pass							= translate_user_pointer(pass);
		if(__unlikely(!pass)) return -EFAULT;
		try
		{
			std::string pstr(pass);
			user_accounts_manager& inst	= *user_accounts_manager::get_instance();
			user_handle current			= inst.get_user(task->uid());
			permission_flag cperms		= current->capabilities.system_permissions;
			if(__unlikely(!(cperms & escalate_process))) return -EPERM;
			if(current->check_pw(pstr))
			{
				task->impersonate		= true;
				task->imp_uid			= root_uid;
				task->imp_gid			= root_gid;
				return 0;
			}
			return -EACCES;
		}
		catch(std::out_of_range& e) { panic(e.what()); return -EINVAL; }
		catch(std::bad_alloc&) { return -ENOMEM; }
	}
}