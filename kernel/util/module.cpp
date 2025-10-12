#include "kernel/module.hpp"
#include "stdexcept"
#include "kdebug.hpp"
namespace ooos_kernel_module
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
