#ifndef __WORKER_LIST
#define __WORKER_LIST
#include <sched/worker.hpp>
#include <sched/scheduler.hpp>
#include <set>
namespace ooos
{
	class worker_list : std::set<worker>
	{
		using __base = std::set<worker>;
		pid_t __upid() const noexcept;
		constexpr static std::align_val_t __stk_algn = static_cast<std::align_val_t>(PAGESIZE);
		constexpr static std::alignval_allocator<char, __stk_algn> __stk_alloc{};
	public:
		static worker_list instance;
		using __base::iterator;
		using __base::const_iterator;
		using __base::begin;
		using __base::end;
		using __base::find;
		using __base::contains;
		template<__internal::__worker_fn FT>
		inline worker* create_worker(FT&& fn, size_t stack_sz = S04, uint16_t qv = 3US)
		{
			uint16_t rv 						= static_cast<uint16_t>(scheduler::ms_to_ticks(qv));
			std::pair<iterator, bool> result	= this->emplace(std::forward<FT>(fn), __stk_alloc.allocate(stack_sz), stack_sz, this->__upid(), rv);
			return result.first.base();
		}
		bool destroy(worker* w);
		bool destroy_worker(pid_t pid);
	};
}
#define wl ooos::worker_list::instance
#endif