#ifndef __KMOD
#define __KMOD
#include "kernel_api.hpp"
#include <new>
namespace ooos_kernel_module
{
    class abstract_module_base
    {
        kernel_api* __api_hooks;
        kmod_mm* __allocated_mm;
        void (*__fini_fn)();
    public:    
        virtual void initialize() = 0;
        virtual void finalize() = 0;
        inline abstract_module_base* tie_api_mm(kernel_api* api, kmod_mm* mm) { if(!__api_hooks && !__allocated_mm && api && mm) { __api_hooks = api; __allocated_mm = mm; } return this; }
        inline void* allocate_dma(size_t size, bool prefetchable) { return __api_hooks->allocate_dma(size, prefetchable); }
        inline pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) { return __api_hooks->find_pci_device(device_class, subclass); }
        inline void* acpi_get_table(const char* label) { return __api_hooks->acpi_get_table(label); }
        inline void log(const char* msg) { __api_hooks->log(typeid(*this), msg); }
        template<wrappable_actor FT> inline void on_irq(uint8_t irq, FT&& handler) { __api_hooks->on_irq(irq, static_cast<isr_actor&&>(isr_actor(static_cast<FT&&>(handler), __allocated_mm)), this); }
        inline void setup(kernel_api* api, kmod_mm* mm, void (*fini)()) { if(api && mm && fini && !(__api_hooks || __allocated_mm || __fini_fn)) { __api_hooks = api; __allocated_mm = mm; __fini_fn = fini; } }
        friend void module_takedown(abstract_module_base* mod);
    };
    void module_takedown(abstract_module_base* mod);
    template<typename T, typename... Args>
    abstract_module_base* setup_instance(void* addr, kernel_api* api, kframe_tag** frame_tag, kframe_exports* kframe_fns, void (*init)(), void (*fini)(), Args&& ... args)
    {
        if(addr && api && frame_tag && kframe_fns && fini)
        {
            if(kmod_mm* mm = api->create_mm())
            {
                *frame_tag = mm->get_frame();
                api->init_memory_fns(kframe_fns);
                (*init)();
                T* result = new (addr) T(static_cast<Args&&>(args)...);
                result->setup(api, mm, fini);
                return result;
            }
        }
        return nullptr;
    }
}
/**
 * EXPORT_MODULE(T, Args...)
 * All modules must invoke this macro exactly once in order to build properly.
 * When invoked, it defines a function that the kernel will call when the module loads.
 * That function allocates a separate page frame structure which is used to implement most of the underlying "glue" needed by C++ to function fully.
 * It then also sets up some pointers to vtables which tie in kernel functionality modules might need to use.
 */
#define EXPORT_MODULE(module_class, ...) static char __instance[sizeof(module_class)]; extern "C" { ooos_kernel_module::abstract_module_base* module_init(ooos_kernel_module::kernel_api* api, kframe_tag** frame_tag, kframe_exports* kframe_fns, void (*init)(), void (*fini)()) { return ooos_kernel_module::setup_instance<module_class>(&__instance, api, frame_tag, kframe_fns, init, fini, __VA_ARGS__); } }
#endif