#include "kernel/kernel_mm.hpp"
#include "kdebug.hpp"
#include "sched/task_ctx.hpp"
#include "errno.h"
extern "C"
{
    addr_t syscall_sbrk(ptrdiff_t incr)
    {
        uframe_tag* ctask_frame = current_active_task()->frame_ptr;
        if(ctask_frame->magic != uframe_magic) return addr_t(static_cast<uintptr_t>(-EINVAL));
        kernel_memory_mgr::get().enter_frame(ctask_frame);
        addr_t result = ctask_frame->extent;
        bool success = ctask_frame->shift_extent(incr);
        kernel_memory_mgr::get().exit_frame();
        if(success) { return result; }
        else return addr_t(static_cast<uintptr_t>(-ENOMEM));
    }
    addr_t syscall_mmap(addr_t addr, size_t len, int prot, int flags, int fd, ptrdiff_t offset)
    {
        uframe_tag* ctask_frame = current_active_task()->frame_ptr;
        if(ctask_frame->magic != uframe_magic || !len || size_t(offset) > len || offset % page_size) return addr_t(static_cast<uintptr_t>(-EINVAL));
        if(!prot) return nullptr;
        addr_t min(std::max(mmap_min_addr, ctask_frame->mapped_max.full));
        if(min != min.page_aligned()) min = min.plus(page_size).page_aligned();
        if(!(flags & MAP_FIXED)) addr = std::max(min, addr).page_aligned();
        else if(addr && (addr < min || addr != addr.page_aligned())) return addr_t(static_cast<uintptr_t>(-EINVAL));
        kernel_memory_mgr::get().enter_frame(ctask_frame);
        addr_t result = ctask_frame->mmap_add(addr, len, prot & PROT_WRITE, prot & PROT_READ);
        kernel_memory_mgr::get().exit_frame();
        if(!(flags & MAP_ANONYMOUS))
        {
            filesystem* fsptr = get_fs_instance();
            if(!fsptr) return addr_t(static_cast<uintptr_t>(-ENOSYS));
            else try
            {
                file_node* n = get_by_fd(fsptr,current_active_task()->self, fd);
                if(n)
                {
                    size_t data_len = std::min(size_t(len - offset), n->size());
                    file_node::pos_type pos = n->tell();
                    n->seek(offset, std::ios_base::beg);
                    n->read(result, data_len);
                    n->seek(pos);
                    return result;
                }
            }
            catch(std::exception& e) { panic(e.what()); }
            return addr_t(static_cast<uintptr_t>(-EBADF));
        }
        return result;
    }
    int syscall_munmap(addr_t addr, size_t len)
    {
        uframe_tag* ctask_frame = current_active_task()->frame_ptr;
        if(ctask_frame->magic != uframe_magic) return -ENOSYS;
        if(!ctask_frame->mmap_remove(addr, len)) return -EINVAL;
        return 0;
    }
}