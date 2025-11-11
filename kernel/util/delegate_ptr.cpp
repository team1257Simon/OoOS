#define INST_DELEGATE
#include "ext/delegate_ptr.hpp"
namespace std
{
	namespace ext
	{
		namespace __impl
		{
			unordered_map<const type_info*, __generic_ptr_container> __delegate_ptr_mgr_inst{ 256 };
			size_t __generic_ptr_container::__target_idx()
			{
				if(__last_idx)
				{
					size_t result	= __last_idx;
					__last_idx		= 0UL;
					return result;
				}
				size_t result		= size();
				push_back(nullptr);
				return result;
			}
			void __generic_ptr_container::__erase_at_index(size_t idx)
			{
				(*__dealloc_node)(at(idx));
				(*this)[idx]	= nullptr;
				__last_idx		= idx;
			}
			__generic_ptr_container::__generic_ptr_container(__node_alloc_ftor alloc, __node_dealloc_ftor dealloc) :
				__acquire_fn	{},
				__release_fn	{},
				__alloc_node	{ alloc },
				__dealloc_node	{ dealloc },
				__destroy_node	{ bind(&__generic_ptr_container::__erase_at_index, this, placeholders::_1) }
								{} 
			__generic_ptr_container::__generic_ptr_container(delegate_callback acq, delegate_callback rel, __node_alloc_ftor alloc, __node_dealloc_ftor dealloc) :
				__acquire_fn	{ acq },
				__release_fn	{ rel },
				__alloc_node	{ alloc },
				__dealloc_node	{ dealloc },
				__destroy_node	{ bind(&__generic_ptr_container::__erase_at_index, this, placeholders::_1) }
								{}
		}
	}
}