#ifndef __KMOD
#define __KMOD
#include "kernel_api.hpp"
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
        inline void* allocate_dma(size_t size, bool prefetchable) { return __api_hooks->allocate_dma(size, prefetchable); }
        inline pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) { return __api_hooks->find_pci_device(device_class, subclass); }
        inline void* acpi_get_table(const char* label) { return __api_hooks->acpi_get_table(label); }
        inline void log(const char* msg) { __api_hooks->log(this, msg); }
        template<wrappable_actor FT> inline void on_irq(uint8_t irq, FT&& handler) { __api_hooks->on_irq(irq, static_cast<isr_actor&&>(isr_actor(static_cast<FT&&>(handler), __allocated_mm)), this); }
#ifdef __KERNEL__
        friend bool module_pre_setup(abstract_module_base* mod, kframe_tag** frame_ptr, kframe_exports* ptrs, void (*fini_fn)());
        friend void module_init(abstract_module_base* mod);
        friend void module_takedown(abstract_module_base* mod);
#endif
    };
}
#define EXPORT_MODULE(module_class, ...) static module_class __instance{ __VA_ARGS__ }; extern "C" { ooos_kernel_module::abstract_module_base* module_instance() { return &__instance; } }
#endif