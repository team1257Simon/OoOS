#include "kernel/module.hpp"
namespace ooos
{
	void module_takedown(abstract_module_base* mod)
	{
		if(mod && mod->__api_hooks)
		{
			mod->finalize();
			if(mod->__fini_fn) (*mod->__fini_fn)();
			mod->__api_hooks->remove_actors(mod);
			if(mod->__allocated_mm) mod->__api_hooks->destroy_mm(mod->__allocated_mm);
		}
	}
}