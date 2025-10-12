#ifndef __PCI_DEV_LIST
#define __PCI_DEV_LIST
#include "arch/pci.hpp"
#include "vector"
class pci_device_list : public std::vector<pci_config_space*>
{
    constexpr pci_config_space* __add(pci_config_space* s) { if(s) this->push_back(s); return s; }
    pci_device_list() = default;
    void add_all(pci_config_table* tb);
    static pci_device_list __instance;
public:
    pci_device_list(pci_device_list&&) = delete;
    pci_device_list(pci_device_list const&) = delete;
    static bool init_instance();
    static pci_device_list* get_instance();
    pci_config_space* find(uint8_t device_class, uint8_t subclass);
};
#endif