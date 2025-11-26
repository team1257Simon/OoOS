#ifndef __FD_LOCKS
#define __FD_LOCKS
#include "map"
#include "set"
#include "bits/ios_base.hpp"
struct file_vnode;
struct file_lock_span
{
	size_t start_pos;
	size_t length;
	friend constexpr std::weak_ordering operator<=>(file_lock_span const& __this, file_lock_span const& __that) noexcept
	{
		if(__this.start_pos + __this.length < __that.start_pos) return std::weak_ordering::less;
		else if(__this.start_pos > __that.start_pos + __that.length) return std::weak_ordering::greater;
		else return std::weak_ordering::equivalent;
	}
};
struct file_lock : file_lock_span
{
	enum : short { read = 0, write = 1 } type;
	std::ios_base::seekdir whence_value;
	ptrdiff_t start_value;
	uint64_t locking_pid;
};
class fd_locks_container
{
protected:
	std::set<file_lock> read_locks;
	std::set<file_lock> write_locks;
	file_vnode* locking_file;
	size_t compute_start(ptrdiff_t start, std::ios_base::seekdir whence);
public:
	fd_locks_container(file_vnode* fn);
	~fd_locks_container();
	bool test(file_lock const& l);
	bool test(short type, ptrdiff_t start, std::ios_base::seekdir whence, size_t len);
	file_lock& add(uint64_t pid, short type, std::ios_base::seekdir whence, ptrdiff_t start, size_t len);
	bool remove(ptrdiff_t start, std::ios_base::seekdir whence, size_t len);
};
#endif