#include <arch/pci_device_list.hpp>
#include <string>
pci_device_list pci_device_list::__instance{};
constexpr static pci_config_space* check_valid(pci_config_space* ptr) { return ptr->vendor_id != 0xFFFFUS ? ptr : nullptr; }
pci_config_table* find_pci_config() { return static_cast<pci_config_table*>(find_system_table("MCFG")); }
pci_config_space* get_device(pci_config_table* tb, uint8_t bus, uint8_t slot, uint8_t func) { for(uint8_t i = 0; i < static_cast<uint8_t>((tb->hdr.length - sizeof(tb->hdr) - 8UC) / 16UC); i++) { if(tb->addr_allocations[i].start_bus <= bus && tb->addr_allocations[i].end_bus >= bus) return check_valid(tb->addr_allocations[i].config_start + ((bus - tb->addr_allocations[i].start_bus) * 256 + slot * 8 + func)); } return nullptr; }
void* compute_base_address(uint32_t bar_registers[], uint8_t i) { return addr_t(static_cast<uintptr_t>((bar_registers[i] & 0x00000001UL) ? static_cast<uint64_t>(bar_registers[i] & 0xFFFFFFFCUL) : static_cast<uint64_t>((bar_registers[i] & 0xFFFFFFF0UL) | (bar_registers[i] & 0x00000004UL ? (static_cast<uint64_t>(bar_registers[i + 1]) << 32) : 0UL)))); }
void* compute_base(uint32_t bar) { return addr_t((bar & 0x00000001U) ? static_cast<uintptr_t>(bar & 0xFFFFFFFCU) : static_cast<uintptr_t>(bar & 0xFFFFFFF0U)); }
void pci_device_list::add_all(pci_config_table* tb) { for(int i = 0; i < 256; i++) { for(int j = 0; j < 32; j++) { pci_config_space* p = __add(get_device(tb, i, j, 0)); if(p && p->header_type & BIT(7)){ for(int k = 1; k < 8; k++) { __add(get_device(tb, i, j, k)); } } } } }
pci_config_space* pci_device_list::find(uint8_t device_class, uint8_t subclass) { for(pci_config_space* s : *this) if(s && (s->class_code == device_class) && (s->subclass == subclass)) return s; return nullptr; }
bool pci_device_list::init_instance() { if(pci_config_table* tb = find_pci_config()) { __instance.add_all(tb); return true; } return false; }
pci_device_list* pci_device_list::get_instance() { return std::addressof(__instance); }
pci_capabilities_register* get_first_capability_register(pci_config_space* device)
{
    if(device->status.capabilities_list)
    {
        uint8_t caps_ptr;
        char* dev_space			= reinterpret_cast<char*>(device);
        switch(device->header_type & ~0x80UC)
        {
            case st: caps_ptr	= device->header_0x0.capabilities_pointer; break;
            case br: caps_ptr	= device->header_0x1.capabilities_pointer; break;
            case cb: caps_ptr	= device->header_0x2.capabilities_pointer; break;
            default: panic("[PCI] invalid device header type"); return nullptr;
        }
        return reinterpret_cast<pci_capabilities_register*>(dev_space + caps_ptr);
    }
    panic("[PCI] capabilities list not supported");
    return nullptr;
}
pci_capabilities_register* get_next_capability_register(pci_config_space* device, pci_capabilities_register* r)
{
    char* dev_space	= reinterpret_cast<char*>(device);
    uint8_t next	= r->next;
    return reinterpret_cast<pci_capabilities_register*>(dev_space + next);
}