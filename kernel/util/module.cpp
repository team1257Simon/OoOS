#include "kernel/module.hpp"
#include "stdexcept"
namespace ooos_kernel_module
{
    void abstract_module_base::create_mm() { if(__api_hooks) { __allocated_mm = __api_hooks->create_mm(); if(!__allocated_mm) throw std::bad_alloc(); } }
    void abstract_module_base::destroy_mm() { if(__allocated_mm) __api_hooks->destroy_mm(__allocated_mm); }
    void abstract_module_base::unregister_actors() { if(__api_hooks) __api_hooks->remove_actors(this); }
    void abstract_module_base::register_kframe(register_kframe_fn frame_cb) { if(__allocated_mm) __allocated_mm->register_as_frame(frame_cb); }
    void abstract_module_base::link_api(kernel_api* kapi) { __api_hooks = kapi; }
    abstract_module_base::~abstract_module_base() = default;
    void abstract_module_base::on_load(kernel_api* kapi, register_kframe_fn frame_cb)
    {
        link_api(kapi);
        create_mm();
        register_kframe(frame_cb);
        initialize();
    }
    void abstract_module_base::on_unload()
    {
        finalize();
        unregister_actors();
        destroy_mm();
    }
}
