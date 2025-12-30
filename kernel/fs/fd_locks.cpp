#include <fs/fd_locks.hpp>
#include <fs/fs.hpp>
fd_locks_container::fd_locks_container(file_vnode* fn) : read_locks(), write_locks(), locking_file(fn) {}
fd_locks_container::~fd_locks_container() = default;
size_t fd_locks_container::compute_start(ptrdiff_t start, std::ios_base::seekdir whence)
{
	ptrdiff_t base	= (whence > 0 ? static_cast<ptrdiff_t>(locking_file->size()) : whence == std::ios_base::cur ? static_cast<ptrdiff_t>(locking_file->tell()) : 0L);
	base			= std::max(base + start, 0Z);
	return static_cast<size_t>(base);
}
bool fd_locks_container::test(file_lock const& l)
{
	if(l.type == file_lock::read)
		return !read_locks.contains(l);
	return !write_locks.contains(l);
}
bool fd_locks_container::test(short type, ptrdiff_t start, std::ios_base::seekdir whence, size_t len)
{
	size_t actual_start	= compute_start(start, whence);
	file_lock test_file_lock(file_lock_span(actual_start, len));
	if(type == file_lock::read) return !read_locks.contains(test_file_lock);
	else return !write_locks.contains(test_file_lock);
}
file_lock& fd_locks_container::add(uint64_t pid, short type, std::ios_base::seekdir whence, ptrdiff_t start, size_t len)
{
	if(type == file_lock::read)
		return *(read_locks.insert(file_lock(file_lock_span(compute_start(start, whence), len), file_lock::read, whence, start, pid)).first);
	else
		return *(write_locks.insert(file_lock(file_lock_span(compute_start(start, whence), len), file_lock::write, whence, start, pid)).first);
}
bool fd_locks_container::remove(ptrdiff_t start, std::ios_base::seekdir whence, size_t len)
{
	size_t actual_start	= compute_start(start, whence);
	file_lock test_file_lock(file_lock_span(actual_start, len));
	return read_locks.erase(test_file_lock) || write_locks.erase(test_file_lock);
}