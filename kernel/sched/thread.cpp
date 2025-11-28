#include <sched/thread.hpp>
namespace ooos
{
	void lock_thread_mutex(thread_t& t) { lock(std::addressof(t.ctl_info.thread_lock)); }
	void unlock_thread_mutex(thread_t& t) { release(std::addressof(t.ctl_info.thread_lock)); }
	task_dtv::task_dtv() : __dtv_map(64UZ), __dtv_alloc() {}
	task_dtv::task_dtv(task_dtv const& that) : __dtv_map(64UZ), __dtv_alloc(), base_offsets(that.base_offsets) {}
	void task_dtv::instantiate(thread_t& thread)
	{
		lock_thread_mutex(thread);
		std::tuple<pid_t> id_tuple			= std::tuple<pid_t>(thread.ctl_info.thread_id);
		size_t target_size					= base_offsets.size() + 1UZ;
		dtv_by_thread_id::iterator result	= __dtv_map.emplace(std::piecewise_construct, id_tuple, std::forward_as_tuple(target_size, nullptr, __dtv_alloc)).first;
		dynamic_thread_vector& vec			= result->second;
		for(size_t i = 0UZ; i < base_offsets.size(); i++)
			vec[i + 1]						= addr_t(thread.self).minus(base_offsets[i]);
		thread.dtv_ptr 						= std::addressof(vec);
		thread.dtv_len						= vec.size();
		unlock_thread_mutex(thread);
	}
	bool task_dtv::takedown(thread_t& thread)
	{
		lock_thread_mutex(thread);
		bool result	= __dtv_map.erase(thread.ctl_info.thread_id);
		unlock_thread_mutex(thread);
		return result;
	}
	void update_thread_state(thread_t& thread, task_t& task_struct)
	{
		lock_thread_mutex(thread);
		thread.saved_regs	= task_struct.saved_regs;
		thread.fxsv			= task_struct.fxsv;
		unlock_thread_mutex(thread);
	}
}