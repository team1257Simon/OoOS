#include "kernel/kernel_mm.hpp"
#include "kdebug.hpp"
#include "sched/task_ctx.hpp"
#include "errno.h"
extern "C"
{
	addr_t syscall_sbrk(ptrdiff_t incr)
	{
		uframe_tag* ctask_frame = active_frame();
		if(__unlikely(ctask_frame->magic != uframe_magic)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		addr_t result	= ctask_frame->extent;
		bool success	= ctask_frame->shift_extent(incr);
		if(success) { return result; }
		else return addr_t(static_cast<uintptr_t>(-ENOMEM));
	}
	addr_t syscall_mmap(addr_t addr, size_t len, int prot, int flags, int fd, ptrdiff_t offset)
	{
		uframe_tag* ctask_frame = active_frame();
		if(__unlikely(ctask_frame->magic != uframe_magic)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
		if(__unlikely(!len || size_t(offset) > len || offset % page_size)) return addr_t(static_cast<uintptr_t>(-EINVAL));
		if(__unlikely(!prot)) return nullptr;
		addr_t min(std::max(mmap_min_addr, ctask_frame->mapped_max.full));
		if(min != min.page_aligned()) min = min.plus(page_size).page_aligned();
		if(addr && !(flags & MAP_FIXED)) addr = std::max(min, addr).page_aligned();
		else if(__unlikely(addr && (addr < min || addr != addr.page_aligned()))) return addr_t(static_cast<uintptr_t>(-EINVAL));
		addr_t result = ctask_frame->mmap_add(addr, len, prot & PROT_WRITE, prot & PROT_READ);
		if(!(flags & MAP_ANONYMOUS))
		{
			filesystem* fsptr	= get_task_vfs();
			if(__unlikely(!fsptr)) return addr_t(static_cast<uintptr_t>(-ENOSYS));
			else try
			{
				file_node* n = get_by_fd(fsptr, active_task_context(), fd);
				if(n)
				{
					file_node::pos_type pos = n->tell();
					n->seek(offset, std::ios_base::beg);
					n->read(translate_user_pointer(result), std::min(size_t(len - offset), n->size()));
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
		uframe_tag* ctask_frame = active_frame();
		if(__unlikely(ctask_frame->magic != uframe_magic)) return -ENOSYS;
		if(__unlikely(!ctask_frame->mmap_remove(addr, len))) return -EINVAL;
		return 0;
	}
}