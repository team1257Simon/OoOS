#include "module.hpp"
#include "kernel_api.hpp"
namespace ooos_kmod
{
    void abstract_module_base::on_load() { new(std::addressof(__impl_type)) type_erasure(typeid(*this)); __api_hooks = get_api_instance(); init(); }
    type_erasure const& abstract_module_base::get_type() const { return __impl_type; }
    void abstract_module_base::on_unload() { exit(); }
    void abstract_module_base::on_irq(uint8_t idx, std::function<void()>&& action) { __api_hooks->on_irq(idx, std::move(action)); }
    void* abstract_module_base::allocate_dma(size_t size, bool prefetchable) { return __api_hooks->allocate_dma(size, prefetchable); }
    pci_config_space* abstract_module_base::find_pci_device(uint8_t device_class, uint8_t subclass) { return __api_hooks->find_pci_device(device_class, subclass); }
    void* abstract_module_base::acpi_get_table(const char* label) { return __api_hooks->acpi_get_table(label); }

}
