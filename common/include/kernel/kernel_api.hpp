#ifndef __KAPI
#define __KAPI
#include "typeindex"
#include "arch/pci.hpp"
struct kernel_api
{
    virtual void on_irq(byte idx, std::function<void()>&& action) = 0;
    virtual void* allocate_dma(size_t size, bool prefetchable) = 0;
    virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) = 0;
    virtual void* acpi_get_table(const char* label) = 0;
};
kernel_api* get_api_instance();
void init_api();
#endif