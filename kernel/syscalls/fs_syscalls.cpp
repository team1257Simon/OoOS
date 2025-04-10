#include "fs/fs.hpp"
#include "sched/task_ctx.hpp"
#include "stdexcept"
#include "errno.h"
#include "kdebug.hpp"
static inline timespec timestamp_to_timespec(time_t ts) { return { ts / 1000U, static_cast<long>(ts % 1000U) * 1000000L }; }
static inline void __stat_init(fs_node* n, filesystem* fsptr, stat* st) 
{
    new (translate_user_pointer(st)) stat
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
        try { if(file_node* n = fsptr->open_file(static_cast<const char*>(translate_user_pointer(name)), std::ios_base::openmode(smallflags))) return n->vid(); } catch(std::exception& e) { panic(e.what()); return -ENOENT; }
        return -EINVAL;
    }
    int syscall_close(int fd)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { fsptr->close_file(n); return 0; } else return EBADF; } catch(std::exception& e) { panic(e.what()); }
        return EINVAL;
    }
    int syscall_write(int fd, char* ptr, int len)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { n->write(translate_user_pointer(ptr), len); n->fsync(); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_read(int fd, char* ptr, int len)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { n->read(translate_user_pointer(ptr), len); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_link(char* old, char* __new)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { return fsptr->link(old, __new) != nullptr ? 0 : -ENOENT; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_lseek(int fd, long offs, int way) 
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { return n->seek(offs, way == 0 ? std::ios_base::beg : way == 1 ? std::ios_base::cur : std::ios_base::end) >= 0 ? 0 : -EIO; } return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_unlink(char* name)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { return fsptr->unlink(static_cast<const char*>(translate_user_pointer(name))) ? 0 : -ENOENT; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_isatty(int fd)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) return n->is_device() ? 1 : 0; else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_fstat(int fd, stat* st)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { __stat_init(n, fsptr, st); return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_stat(const char* restrict name, stat* restrict st)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        name = translate_user_pointer(name);
        try { if(file_node* n = fsptr->get_file(name)) { __stat_init(n, fsptr, st); return 0; } else if(directory_node* n{ fsptr->get_dir(name, false) }) { __stat_init(n, fsptr, st); return 0; } } catch(std::logic_error& e) { panic(e.what()); return -ENOTDIR; } catch(std::runtime_error& e) { panic(e.what()); return -ENOENT; }
        return -EINVAL;
    }
    int syscall_fchmod(int fd, mode_t m)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        try { if(file_node* n = get_by_fd(fsptr, current_active_task()->self, fd)) { n->mode = m; return 0; } else return -EBADF; } catch(std::exception& e) { panic(e.what()); }
        return -EINVAL;
    }
    int syscall_chmod(const char* name, mode_t m)
    {
        filesystem* fsptr = get_fs_instance();
        if(!fsptr) return -ENOSYS;
        name = translate_user_pointer(name);
        try { if(file_node* n = fsptr->get_file(name)) { n->mode = m; return 0; } else return -EISDIR; } catch(std::exception& e) { panic(e.what()); return -ENOENT; }
        return -EINVAL;
    }
}