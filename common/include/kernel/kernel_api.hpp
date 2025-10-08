#ifndef __KAPI
#define __KAPI
#ifdef __KERNEL__
#include "typeindex"
#include "arch/pci.hpp"
#else
struct pci_config_space;
#endif
namespace ooos_kmod { class abstract_module_base; }
struct kmod_mm
{
    virtual void* mem_allocate(size_t size, size_t align) = 0;
    virtual void mem_release(void* block, size_t align) = 0;
    virtual ~kmod_mm() = default;
};
struct kernel_api
{
    virtual void* allocate_dma(size_t size, bool prefetchable) = 0;
    virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) = 0;
    virtual void* acpi_get_table(const char* label) = 0;
protected:
    friend class ooos_kmod::abstract_module_base;
    virtual kmod_mm* create_mm() = 0;
    virtual void destroy_mm(kmod_mm* mm) = 0;
};
kernel_api* get_api_instance();
void init_api();
#endif