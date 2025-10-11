#include "kernel/module.hpp"
#include "stdexcept"
namespace ooos_kernel_module
{
    void module_init(abstract_module_base* mod) { if(mod) mod->initialize(); }
    bool module_pre_setup(abstract_module_base* mod, kframe_tag** frame_ptr, kframe_exports* ptrs, void (*fini_fn)())
    {
        if(mod && frame_ptr && ptrs)
        {
            mod->__api_hooks = get_api_instance();
            mod->__allocated_mm = mod->__api_hooks->create_mm();
            if(__unlikely(!mod->__allocated_mm)) return false;
            *frame_ptr = mod->__allocated_mm->get_frame();
            init_memory_fns(ptrs);
            mod->__fini_fn = fini_fn;
            return true;
        }
        return false;
    }
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
