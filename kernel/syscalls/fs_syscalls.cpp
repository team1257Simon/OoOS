#include "fs/fs.hpp"
#include "sched/task_ctx.hpp"
#include "stdexcept"
#include "errno.h"
#include "kdebug.hpp"
static inline timespec timestamp_to_timespec(time_t ts) { return { ts / 1000U, static_cast<long>(ts % 1000U) * 1000000L }; }
static inline void __stat_init(vnode* n, filesystem* fsptr, stat* st) 
{
	size_t bs = fsptr->block_size();
	new(st) stat
	{ 
		.st_dev     = fsptr->get_dev_id(), 
		.st_ino     = n->cid(), 
		.st_mode    = n->mode, 
		.st_nlink   = n->num_refs(),
		.st_uid     = 0,    // WIP
		.st_gid     = 0,    // WIP
		.st_rdev    = n->is_device() ? dynamic_cast<device_vnode*>(n)->get_device_id() : 0U, 
		.st_size    = static_cast<long>(n->size()),
		.st_atim    = timestamp_to_timespec(n->create_time),
		.st_mtim    = timestamp_to_timespec(n->modif_time), 
		.st_ctim    = timestamp_to_timespec(n->modif_time), 
		.st_blksize = bs,
		.st_blocks  = div_round_up(n->size(), bs) 
	};  
}
extern "C"
{
	int syscall_open(char* name, int flags, ...)
	{
		filesystem* fsptr   = get_task_vfs();
		name                = translate_user_pointer(name);
		if(__unlikely(!fsptr)) return -ENOSYS;
		if(__unlikely(!name)) return -EFAULT;
		std::ios_base::openmode mode;
		mode.app    = (flags & O_APPEND) != 0;
		mode.in     = (flags & O_RDONLY) || (flags & O_RDWR);
		mode.out    = (flags & O_WRONLY) || (flags & O_RDWR);
		mode.trunc  = (flags & O_TRUNC);
		try 
		{
			if(vnode* existing = fsptr->find_node(name, false, mode))
			{
				if(file_vnode* fn = dynamic_cast<file_vnode*>(existing))
					fn->current_mode = mode;
				if(flags & O_TRUNC) 
					if(__unlikely(!existing->truncate())) return -EIO;
				return existing->vid();
			}
			else if(__unlikely(!(flags & O_CREAT)))
				return -ENOENT;
			else if(file_vnode* n = fsptr->open_file(name, mode)) 
				return n->vid();
		}
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; }
		catch(std::out_of_range& e)     { panic(e.what()); return -ENOENT; }
		catch(std::domain_error& e)     { panic(e.what()); return -ENOLCK; }
		catch(std::runtime_error& e)    { panic(e.what()); return -ENOSPC; }
		catch(std::exception& e)        { panic(e.what()); return -ENOMEM; }
		return -EINVAL;
	}
	int syscall_close(int fd)
	{
		filesystem* fsptr = get_task_vfs();
		if(!fsptr) return -ENOSYS;
		try { if(file_vnode* n = get_by_fd(fsptr, active_task_context(), fd)) { fsptr->close_file(n); return 0; } else return EBADF; } catch(std::exception& e) { panic(e.what()); }
		return ENOMEM;
	}
	int syscall_write(int fd, char* ptr, int len)
	{
		filesystem* fsptr   = get_task_vfs();
		ptr                 = translate_user_pointer(ptr);
		if(!fsptr) return -ENOSYS;
		if(!ptr) return -EFAULT;
		try
		{ 
			if(file_vnode* n = get_by_fd(fsptr, active_task_context(), fd)) 
			{
				if(__unlikely(!n->current_mode.out)) return -EACCES;
				n->write(ptr, len);
				n->fsync();
				return 0; 
			} 
			else return -EBADF;
		} 
		catch(std::logic_error&)
		{
			task_ctx* task = active_task_context();
			force_signal(task, 13); // SIGPIPE
			return -EPIPE;
		}
		catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	int syscall_read(int fd, char* ptr, int len)
	{
		filesystem* fsptr   = get_task_vfs();
		ptr                 = translate_user_pointer(ptr);
		if(__unlikely(!fsptr)) return -ENOSYS;
		if(__unlikely(!ptr)) return -EFAULT;
		try 
		{ 
			if(file_vnode* n = get_by_fd(fsptr, active_task_context(), fd))
			{
				if(!n->current_mode.in) return -EACCES;
				n->read(ptr, len); 
				return 0;
			} 
			else return -EBADF; 
		} 
		catch(std::logic_error&)
		{
			task_ctx* task = active_task_context();
			force_signal(task, 13); // SIGPIPE
			return -EPIPE;
		}
		catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	int syscall_link(char* restrict old, char* restrict __new)
	{
		filesystem* fsptr   = get_task_vfs();
		old                 = translate_user_pointer(old);
		__new               = translate_user_pointer(__new);
		if(__unlikely(!fsptr)) return -ENOSYS;
		if(__unlikely(!old || !__new)) return -EFAULT;
		try { return fsptr->link(old, __new) != nullptr ? 0 : -ENOSPC; }
		catch(std::overflow_error& e) { panic(e.what()); return -EMLINK; }
		catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	int syscall_lseek(int fd, long offs, int way) 
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		try 
		{ 
			if(file_vnode* n = get_by_fd(fsptr, active_task_context(), fd)) 
			{
				std::ios_base::seekdir xway;
				if(way == 0) xway       = std::ios_base::beg;
				else if(way == 1) xway  = std::ios_base::cur;
				else if(way == 2) xway  = std::ios_base::end;
				else return -EINVAL;
				off_t result = n->seek(offs, xway);
				if(result < 0) return -EOVERFLOW;
				else return static_cast<int>(result);
			}
			return -EBADF; 
		} 
		catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	int syscall_unlink(char* name)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		name = translate_user_pointer(name);
		if(__unlikely(!name)) return -EFAULT;
		try { return fsptr->unlink(name) ? 0 : -ENOENT; }
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::exception& e)        { panic(e.what()); }
		return -ENOMEM;
	}
	int syscall_isatty(int fd)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		try { if(file_vnode* n = get_by_fd(fsptr, active_task_context(), fd)) return n->is_device() ? 1 : 0; else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	int syscall_fstat(int fd, stat* st)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		st = translate_user_pointer(st);
		if(__unlikely(!st)) return -EFAULT;
		try { if(file_vnode* n = get_by_fd(fsptr, active_task_context(), fd)) { __stat_init(n, fsptr, st); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	int syscall_stat(const char* restrict name, stat* restrict st)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		name    = translate_user_pointer(name);
		st      = translate_user_pointer(st);
		if(__unlikely(!st || !name)) return -EFAULT;
		try
		{
			vnode* fn = fsptr->find_node(name);
			if(fn) { __stat_init(fn, fsptr, st); return 0; } 
			return -ENOENT;
		}
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; } 
		return -ENOMEM;
	}
	int syscall_fchmod(int fd, mode_t m)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		try 
		{ 
			if(file_vnode* n = get_by_fd(fsptr, active_task_context(), fd)) 
			{
				n->mode = m;
				n->fsync();
				return 0;
			}
			else return -EBADF; 
		} 
		catch(std::exception& e) { panic(e.what()); }
		return -ENOMEM;
	}
	int syscall_chmod(const char* name, mode_t m)
	{
		filesystem* fsptr = get_task_vfs();
		name = translate_user_pointer(name);
		if(__unlikely(!fsptr)) return -ENOSYS;
		if(__unlikely(!name)) return -EFAULT;
		try
		{ 
			if(file_vnode* n = fsptr->get_file(name))
			{
				n->mode = m;
				n->fsync();
				return 0;
			} 
			else return -EISDIR;
		} 
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::exception& e)        { panic(e.what()); }
		return -ENOENT;
	}
	int syscall_mkdir(const char* path, mode_t mode)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		path = translate_user_pointer(path);
		if(__unlikely(!path) )return -EFAULT;
		if(__unlikely(std::strnlen(path, 255) != std::strnlen(path, 256))) return -ENAMETOOLONG;
		if(__unlikely(fsptr->get_directory_or_null(path, false) != nullptr)) return -EEXIST;
		mode_t full_mode = mode | 0040000;
		try { fsptr->create_node(nullptr, path, full_mode); return 0; }
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; }
		catch(std::out_of_range& e)     { panic(e.what()); return -ENOENT; }
		catch(std::runtime_error& e)    { panic(e.what()); return -ENOSPC; }
		catch(std::exception& e)        { panic(e.what()); }
		return -ENOMEM;
	}
	addr_t syscall_fdopendir(int fd)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		task_ctx* task = active_task_context();
		try
		{
			vnode* node       = fsptr->get_fd_node(fd);
			directory_vnode* dir = dynamic_cast<directory_vnode*>(node);
			if(!node)   return addr_t(static_cast<uintptr_t>(-EBADF));
			if(!dir)    return addr_t(static_cast<uintptr_t>(-ENOTDIR));
			return task->opened_directories.emplace(std::piecewise_construct, std::forward_as_tuple(fd), std::forward_as_tuple(dir, task->task_struct.frame_ptr)).first->second.get_dir_struct_vaddr();
		}
		catch(std::exception& e) { panic(e.what()); }
		return addr_t(static_cast<uintptr_t>(-ENOMEM));
	}
	addr_t syscall_opendir(const char* name)
	{
		filesystem* fsptr   = get_task_vfs();
		name                = translate_user_pointer(name);
		task_ctx* task      = active_task_context();
		if(__unlikely(!fsptr)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		if(__unlikely(!name)) return addr_t(static_cast<uintptr_t>(-EFAULT));
		try
		{
			directory_vnode* dir = fsptr->get_directory_or_null(name, false);
			if(__unlikely(!dir)) return addr_t(static_cast<uintptr_t>(-ENOENT));
			int fd = dir->vid();
			return task->opened_directories.emplace(std::piecewise_construct, std::forward_as_tuple(fd), std::forward_as_tuple(dir, task->task_struct.frame_ptr)).first->second.get_dir_struct_vaddr();
		}
		catch(std::exception& e) { panic(e.what()); }
		return addr_t(static_cast<uintptr_t>(-ENOMEM));
	}
	int syscall_closedir(addr_t dirp)
	{
		filesystem* fsptr   = get_task_vfs();
		dirp                = translate_user_pointer(dirp);
		if(__unlikely(!fsptr)) return -ENOSYS;
		if(__unlikely(!dirp)) return -EFAULT;
		task_ctx* task = active_task_context();
		task->opened_directories.erase(dirp.deref<DIR>().fd);
		return 0;
	}
	int syscall_lstat(const char* restrict name, stat* restrict st)
	{
		filesystem* fsptr   = get_task_vfs();
		name                = translate_user_pointer(name);
		st                  = translate_user_pointer(st);
		if(__unlikely(!fsptr)) return -ENOSYS;
		if(__unlikely(!st || !name)) return -EFAULT;
		try
		{
			vnode* fn = fsptr->find_node(name, true);
			if(fn) { __stat_init(fn, fsptr, st); return 0; } 
			return -ENOENT;
		}
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; } 
		return -ENOMEM;
	}
	int syscall_mknod(const char* name, mode_t mode, dev_t dev)
	{
		filesystem* fsptr   = get_task_vfs();
		name                = translate_user_pointer(name);
		if(__unlikely(!fsptr)) return -ENOSYS;
		if(__unlikely(!name)) return -EFAULT;
		try { fsptr->create_node(nullptr, name, mode, dev); }
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; }
		catch(std::domain_error& e)     { panic(e.what()); return -EEXIST; }
		catch(std::runtime_error& e)    { panic(e.what()); return -ENOSPC; }
		catch(std::out_of_range& e)     { panic(e.what()); return -ENOENT; }
		catch(std::exception& e)        { panic(e.what()); return -ENOMEM; }
		return 0;
	}
	int syscall_mknodat(int fd, const char* name, mode_t mode, dev_t dev)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		name = translate_user_pointer(name);
		if(__unlikely(!name)) return -EFAULT;
		vnode* node = fsptr->get_fd_node(fd);
		if(__unlikely(!node)) return -EBADF;
		directory_vnode* dirnode = dynamic_cast<directory_vnode*>(node);
		if(__unlikely(!node)) return -ENOTDIR;
		try { fsptr->create_node(dirnode, name, mode, dev); }
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; } 
		catch(std::domain_error& e)     { panic(e.what()); return -EEXIST; }
		catch(std::runtime_error& e)    { panic(e.what()); return -ENOSPC; }
		catch(std::out_of_range& e)     { panic(e.what()); return -ENOENT; }
		catch(std::exception& e)        { panic(e.what()); return -ENOMEM; }
		return 0;
	}
	int syscall_pipe(int* out)
	{
		filesystem* fsptr = get_task_vfs();
		if(__unlikely(!fsptr)) return -ENOSYS;
		out = translate_user_pointer(out);
		if(__unlikely(!out)) return -EFAULT;
		int fds[2]{ 0, 0 };
		try { fsptr->create_pipe(fds); }
		catch(std::overflow_error& e)   { panic(e.what()); return -EMLINK; }
		catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; } 
		catch(std::domain_error& e)     { panic(e.what()); return -EEXIST; }
		catch(std::runtime_error& e)    { panic(e.what()); return -ENOSPC; }
		catch(std::out_of_range& e)     { panic(e.what()); return -ENOENT; }
		catch(std::exception& e)        { panic(e.what()); return -ENOMEM; }
		return 0;
	}
}