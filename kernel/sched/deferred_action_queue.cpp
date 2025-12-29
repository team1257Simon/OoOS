#include <sched/deferred_action.hpp>
namespace ooos
{
	deferred_action_queue::deferred_action_queue() noexcept		= default;
	deferred_action_queue::~deferred_action_queue() noexcept	= default;
	void deferred_action_queue::tick()
	{
		bool pass		= false;
		for(deferred_action& action : *this)
		{
			action();
			if(!pass && action.is_done()) this->pop();
			else pass	= true;
		}
	}
	time_t deferred_action_queue::compute_ticks(time_t millis) const noexcept
	{
		__float128 ms 		= static_cast<__float128>(millis);
		__float128 result 	= ms * ticks_per_ms;
		return static_cast<time_t>(result);
	}
}