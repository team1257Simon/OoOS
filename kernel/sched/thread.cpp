#include <sched/thread.hpp>
namespace ooos
{
	void task_dtv::__lthread(thread_t& t) { lock(std::addressof(t.ctl_info.thread_lock)); }
	void task_dtv::__ulthread(thread_t& t) { release(std::addressof(t.ctl_info.thread_lock)); }
	task_dtv::task_dtv() : __dtv_map(64UZ), __dtv_alloc() {}
	void task_dtv::instantiate(thread_t& thread)
	{
		__lthread(thread);
		std::tuple<uint32_t> id_tuple		= std::tuple<uint32_t>(thread.ctl_info.thread_id);
		size_t target_size					= base_offsets.size() + 1UZ;
		dtv_by_thread_id::iterator result	= __dtv_map.emplace(std::piecewise_construct, id_tuple, std::forward_as_tuple(target_size, nullptr, __dtv_alloc)).first;
		dynamic_thread_vector& vec			= result->second;
		for(size_t i = 0UZ; i < base_offsets.size(); i++)
			vec[i + 1]						= addr_t(thread.self).minus(base_offsets[i]);
		thread.dtv_ptr 						= std::addressof(vec);
		thread.dtv_len						= vec.size();
		__ulthread(thread);
	}
	bool task_dtv::takedown(thread_t& thread)
	{
		__lthread(thread);
		bool result							= __dtv_map.erase(thread.ctl_info.thread_id);
		__ulthread(thread);
		return result;
	}
}