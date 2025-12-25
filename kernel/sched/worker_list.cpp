#include <sched/worker_list.hpp>
#include <stdlib.h> // rand()
namespace ooos
{
	worker_list worker_list::instance;
	bool worker_list::destroy_worker(pid_t pid) { return contains(pid) ? destroy(std::to_address(find(pid))) : false; }
	pid_t worker_list::__upid() const noexcept
	{
		pid_t pid{};
		do pid = static_cast<pid_t>(rand()); while(!pid || contains(pid));
		return pid;
	}
	bool worker_list::destroy(worker* w)
	{
		if(__unlikely(!w)) return false;
		iterator i = find(*w);
		if(__unlikely(i == end())) return false;
		__stk_alloc.deallocate(i->stack_base, i->stack_size);
		erase(i);
		return true;
	}
}