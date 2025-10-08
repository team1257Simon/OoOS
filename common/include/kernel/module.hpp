#ifndef __KMOD
#define __KMOD
#include "kernel_api.hpp"
namespace ooos_kmod
{
    using namespace std::ext;
    class abstract_module_base
    {
        kernel_api* __api_hooks;
        kmod_mm* __allocated_mm;
    protected:
        virtual void init() = 0;
        virtual void exit() = 0;
        inline void* allocate_dma(size_t size, bool prefetchable) { return __api_hooks->allocate_dma(size, prefetchable); }
        inline pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) { return __api_hooks->find_pci_device(device_class, subclass); }
        inline void* acpi_get_table(const char* label) { return __api_hooks->acpi_get_table(label); }
    public:
        void on_load();
        void on_unload();
    };
}
#endif