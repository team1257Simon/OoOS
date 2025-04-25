#include "fs/fs.hpp"
#include "sched/task_ctx.hpp"
#include "stdexcept"
#include "errno.h"
#include "kdebug.hpp"
static inline timespec timestamp_to_timespec(time_t ts) { return { ts / 1000U, static_cast<long>(ts % 1000U) * 1000000L }; }
static inline void __stat_init(fs_node* n, filesystem* fsptr, stat* st) 
{
    new(st) stat
    { 
        .st_dev = fsptr->get_dev_id(), 
        .st_ino = n->cid(), 
        .st_mode = n->mode, 
        .st_nlink = n->num_refs(), 
        .st_uid = 0,    // WIP
        .st_gid = 0,    // WIP
        .st_rdev = n->is_device() ? 1U : 0, 
        .st_size = static_cast<long>(n->size()),
        .st_atim = timestamp_to_timespec(n->create_time),
        .st_mtim = timestamp_to_timespec(n->modif_time), 
        .st_ctim = timestamp_to_timespec(n->modif_time), 
        .st_blksize = physical_block_size,
        .st_blocks = div_round_up(n->size(), physical_block_size) 
    };  
}
extern "C"
{
    int syscall_open(char* name, int flags, ...)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        uint8_t smallflags{ static_cast<uint8_t>(flags) };
        name = translate_user_pointer(name);
        if(!name) return -EINVAL;
        try 
        {
            if(fs_node* existing = fsptr->find_node(name))
                return existing->vid();
            else if(file_node* n = fsptr->open_file(name, std::ios_base::openmode(smallflags))) 
                return n->vid(); 
        }
        catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; }
        catch(std::out_of_range& e) { panic(e.what()); return -ENOENT; }
        catch(std::domain_error& e) { panic(e.what()); return -ENOLCK; }
        catch(std::runtime_error& e) { panic(e.what()); return -ENOSPC; }
        catch(std::exception& e) { panic(e.what()); return -ENOMEM; }
        return -EINVAL;
    }
    int syscall_close(int fd)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, active_task_context(), fd)) { fsptr->close_file(n); return 0; } else return EBADF; } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_write(int fd, char* ptr, int len)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        ptr = translate_user_pointer(ptr);
        if(!ptr) return -EINVAL;
        try { if(file_node* n = get_by_fd(fsptr, active_task_context(), fd)) { n->write(ptr, len); n->fsync(); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -ENOMEM;
    }
    int syscall_read(int fd, char* ptr, int len)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        ptr = translate_user_pointer(ptr);
        if(!ptr) return -EINVAL;
        try { if(file_node* n = get_by_fd(fsptr, active_task_context(), fd)) { n->read(ptr, len); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -ENOMEM;
    }
    int syscall_link(char* old, char* __new)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        old = translate_user_pointer(old);
        __new = translate_user_pointer(__new);
        if(!old || !__new) return -EINVAL;
        try { return fsptr->link(old, __new) != nullptr ? 0 : -ENOSPC; } catch(std::exception& e) { panic(e.what()); }
        return -ENOMEM;
    }
    int syscall_lseek(int fd, long offs, int way) 
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, active_task_context(), fd)) { return n->seek(offs, way == 0 ? std::ios_base::beg : way == 1 ? std::ios_base::cur : std::ios_base::end) >= 0 ? 0 : -EIO; } return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_unlink(char* name)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        name = translate_user_pointer(name);
        if(!name) return -EINVAL;
        try { return fsptr->unlink(name) ? 0 : -ENOENT; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_isatty(int fd)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, active_task_context(), fd)) return n->is_device() ? 1 : 0; else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_fstat(int fd, stat* st)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        st = translate_user_pointer(st);
        if(!st) return -EINVAL;
        try { if(file_node* n = get_by_fd(fsptr, active_task_context(), fd)) { __stat_init(n, fsptr, st); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -ENOMEM;
    }
    int syscall_stat(const char* restrict name, stat* restrict st)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        name = translate_user_pointer(name);
        st = translate_user_pointer(st);
        if(!st || !name) return -EINVAL;
        try
        {
            file_node* fn = fsptr->get_file(name);
            if(fn) { __stat_init(fn, fsptr, st); return 0; } 
            else if(directory_node* dn = fsptr->open_directory(name, false)) { __stat_init(dn, fsptr, st); return 0; } 
        }
        catch(std::out_of_range& e) { panic(e.what()); return -ENOENT; }
        catch(std::logic_error& e) { panic(e.what()); return -ENOTDIR; } 
        return -ENOMEM;
    }
    int syscall_fchmod(int fd, mode_t m)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, active_task_context(), fd)) { n->mode = m; return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -ENOMEM;
    }
    int syscall_chmod(const char* name, mode_t m)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        name = translate_user_pointer(name);
        if(!name) return -EINVAL;
        try { if(file_node* n = fsptr->get_file(name)) { n->mode = m; return 0; } else return -EISDIR; } catch(std::exception& e) { panic(e.what()); }
        return -ENOENT;
    }
    int syscall_mkdir(const char* path, mode_t mode)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        path = translate_user_pointer(path);
        if(!path) return -EINVAL;
        if(std::strnlen(path, 255) != std::strnlen(path, 256)) return -ENAMETOOLONG;
        if(fsptr->get_directory_or_null(path, false)) return -EEXIST;
        try { if(directory_node* n = fsptr->open_directory(path)) { n->mode = mode; return 0; } }
        catch(std::invalid_argument& e) { panic(e.what()); return -ENOTDIR; }
        catch(std::logic_error& e) { panic(e.what()); return -ENOENT; }
        catch(std::runtime_error& e) { panic(e.what()); return -ENOSPC; }
        catch(std::exception& e) { panic(e.what()); }
        return -ENOMEM;
    }
    addr_t syscall_fdopendir(int fd)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return addr_t(static_cast<uintptr_t>(-ENOSYS));
        task_ctx* task = active_task_context();
        try
        {
            fs_node* node = fsptr->get_fd_node(fd);
            if(!node) return addr_t(static_cast<uintptr_t>(-EBADF));
            directory_node* dir = dynamic_cast<directory_node*>(node);
            if(!dir) return addr_t(static_cast<uintptr_t>(-ENOTDIR));
            return task->opened_directories.emplace(std::piecewise_construct, std::forward_as_tuple(fd), std::forward_as_tuple(dir, task->task_struct.frame_ptr)).first->second.get_dir_struct_vaddr();
        }
        catch(std::exception& e) { panic(e.what()); }
        return addr_t(static_cast<uintptr_t>(-ENOMEM));
    }
    addr_t syscall_opendir(const char* name)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return addr_t(static_cast<uintptr_t>(-ENOSYS));
        name = translate_user_pointer(name);
        if(!name) return addr_t(static_cast<uintptr_t>(-EINVAL));
        task_ctx* task = active_task_context();
        try
        {
            directory_node* dir = fsptr->get_directory_or_null(name, false);
            if(!dir) return addr_t(static_cast<uintptr_t>(-ENOENT));
            int fd = dir->vid();
            return task->opened_directories.emplace(std::piecewise_construct, std::forward_as_tuple(fd), std::forward_as_tuple(dir, task->task_struct.frame_ptr)).first->second.get_dir_struct_vaddr();
        }
        catch(std::exception& e) { panic(e.what()); }
        return addr_t(static_cast<uintptr_t>(-ENOMEM));
    }
    int syscall_closedir(addr_t dirp)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        dirp = translate_user_pointer(dirp);
        if(!dirp) return -EINVAL;
        task_ctx* task = active_task_context();
        task->opened_directories.erase(dirp.ref<DIR>().fd);
        return 0;
    }
}