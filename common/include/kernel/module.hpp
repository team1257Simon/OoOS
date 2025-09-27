#ifndef __KMOD
#define __KMOD
#include "typeindex"
struct pci_config_space;
struct kernel_api;
namespace ooos_kmod
{
    using namespace std::ext;
    class abstract_module_base
    {
        type_erasure __impl_type;
        kernel_api* __api_hooks;
    protected:
        virtual void init() = 0;
        virtual void exit() = 0;
        void on_irq(uint8_t idx, std::function<void()>&& action);
        void* allocate_dma(size_t size, bool prefetchable);
        pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass);
        void* acpi_get_table(const char* label);
    public:
        virtual ~abstract_module_base() = 0;
        void on_load();
        void on_unload();
        type_erasure const& get_type() const;
    };
}
#endif