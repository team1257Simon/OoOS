#include "arch/pci.hpp"
#include "string"
pci_device_list pci_device_list::__instance{};
constexpr static pci_config_space* check_valid(pci_config_space* ptr) { return ptr->vendor_id != 0xFFFF ? ptr : nullptr; }
pci_config_table* find_pci_config(xsdt_t* xsdt) { if(xsdt) return vaddr_t{ find_system_table(xsdt, "MCFG") }; return nullptr; }
pci_config_space* get_device(pci_config_table* tb, uint8_t bus, uint8_t slot, uint8_t func) { for(uint8_t i = 0; i < static_cast<uint8_t>((tb->hdr.length - sizeof(tb->hdr) - 8) / 16); i++) { if(tb->addr_allocations[i].start_bus <= bus && tb->addr_allocations[i].end_bus >= bus) return check_valid(tb->addr_allocations[i].config_start + ((bus - tb->addr_allocations[i].start_bus) * 256 + slot * 8 + func)); } return nullptr; }
vaddr_t compute_base_address(uint32_t bar_registers[], uint8_t i) { return vaddr_t { uintptr_t((bar_registers[i] & 0x00000001) ? uintptr_t(bar_registers[i] & 0xFFFFFFFC) : uint64_t((bar_registers[i] & 0xFFFFFFF0uL) | (bar_registers[i] & 0x00000004 ? (uint64_t(bar_registers[i+1]) << 32) : 0))) }; }
vaddr_t compute_base(uint32_t bar) { return vaddr_t{ ((bar & 0x00000001) ? uintptr_t(bar & 0xFFFFFFFC) : uintptr_t(bar & 0xFFFFFFF0)) }; }
void pci_device_list::add_all(pci_config_table *tb) { for(int i = 0; i < 256; i++) { for(int j = 0; j < 32; j++) { pci_config_space* p = __add(get_device(tb, i, j, 0)); if(p && p->header_type[7]){ for(int k = 1; k < 8; k++) { __add(get_device(tb, i, j, k)); } } } } }
pci_config_space *pci_device_list::find(uint8_t device_class, uint8_t subclass) { for(pci_config_space* s : *this) if(s && (s->class_code == device_class) && (s->subclass == subclass)) return s; return nullptr; }
bool pci_device_list::init_instance(xsdt_t *xsdt) { if(pci_config_table* tb = find_pci_config(xsdt)) { __instance.add_all(tb); return true; } return false; }
pci_device_list *pci_device_list::get_instance() { return &__instance; }