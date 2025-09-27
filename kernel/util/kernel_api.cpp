#include "kernel_api.hpp"
#include "isr_table.hpp"
#include "kernel_mm.hpp"
static struct : kernel_api
{
    kernel_memory_mgr* mm;
    pci_device_list* pci;
    virtual void on_irq(byte idx, std::function<void()>&& action) override { interrupt_table::add_irq_handler(idx, std::move(action)); }
    virtual void* allocate_dma(size_t size, bool prefetchable) override { return mm->allocate_dma(size, prefetchable); }
    virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) override {  return pci->find(device_class, subclass); }
    virtual void* acpi_get_table(const char* label) override { return find_system_table(label); }
} __api_impl{};
void init_api() { __api_impl.mm = kernel_memory_mgr::__instance; __api_impl.pci = pci_device_list::get_instance(); }
kernel_api* get_api_instance() { if(__unlikely(!__api_impl.pci || !__api_impl.mm)) return nullptr; else return std::addressof(__api_impl); }