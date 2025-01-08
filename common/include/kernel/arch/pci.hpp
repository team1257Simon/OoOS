#ifndef __PERIPHERAL_CONTROL
#define __PERIPHERAL_CONTROL
#include "vector"
#include "libk_decls.h"
enum ht
{
    st = 0x0,   // standard
    br = 0x1,   // pci-to-pci bridge
    cb = 0x2    // cardbus bridge
};
struct pci_config_space
{
    uint16_t vendor_id;
    uint16_t device_id;
    struct
    {
        bool io_space : 1;
        bool memory_space : 1;
        bool bus_master : 1;
        bool : 3;
        bool parity_error_response : 1;
        bool : 1;
        bool serrno_enable : 1;
        bool : 1;
        bool interrupt_disable : 1;
        bool : 5;
    } command;
    struct
    {
        bool : 3;
        bool interrupt_status : 1;
        bool capabilities_list : 1; // hardwired to 1
        bool : 3;
        bool master_data_parity_error : 1;
        bool : 2;
        bool signaled_target_abort : 1;
        bool received_target_abort : 1;
        bool received_master_abort : 1;
        bool signaled_system_error : 1;
        bool detected_parity_error : 1;
    } status;
    uint8_t revision_id;
    uint8_t prog_if;
    uint8_t subclass;
    uint8_t class_code;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    byte header_type;
    byte bist;
    union
    {
        struct
        {
            uint32_t bar[6];                // base address register; not to be confused with foo
            uint32_t cardbus_cis_pointer;
            uint16_t subsystem_vendor_id;
            uint16_t subsystem_id;
            uint32_t expansion_rom_base_addr;
            uint8_t capabilities_pointer;   // bottom two bits are reserved
            uint8_t reserved0[7];
            uint8_t interrupt_line;
            uint8_t interrupt_pin;
            uint8_t min_grant;
            uint8_t max_latency;
            uint32_t reserved1[21];
        } header_0x0;
        struct
        {
            uint32_t bar[2];
            uint8_t primary_bus;
            uint8_t secondary_bus;
            uint8_t subordinate_bus;
            uint8_t secondary_latency_timer;
            uint8_t io_base_lo;
            uint8_t io_limit_lo;
            uint16_t secondary_status;
            uint16_t memory_base;
            uint16_t memory_limit;
            uint16_t prefetch_base_lo;
            uint16_t prefetch_limit_lo;
            uint32_t prefetch_base_hi;
            uint32_t prefetch_limit_hi;
            uint16_t io_base_hi;
            uint16_t io_limit_hi;
            uint8_t capabilities_pointer;
            uint8_t reserved0[3];
            uint32_t expansion_rom_base_addr;
            uint8_t interrupt_line;
            uint8_t interrupt_pin;
            uint16_t bridge_control;
            uint32_t reserved1[21];
        } header_0x1;
        struct 
        {
            uint32_t cardbus_base;
            uint8_t capabilities_list;
            uint8_t reserved0;
            uint16_t secondary_status;
            uint8_t pci_bus_number;
            uint8_t cardbus_bus_number;
            uint8_t subordinate_bus_number;
            uint8_t cardbus_latency_timer;
            uint32_t memory_base_0;
            uint32_t memory_limit_0;
            uint32_t memory_base_1;
            uint32_t memory_limit_1;
            uint32_t io_base_0;
            uint32_t io_limit_0;
            uint32_t io_base_1;
            uint32_t io_limit_1;
            uint8_t interrupt_line;
            uint8_t interrupt_pin;
            uint16_t bridge_control;
            uint16_t subsystem_device_id;
            uint16_t subsystem_vendor_id;
            uint32_t legacy_mode_base_addr;
            uint32_t reserved1[14];
            uint32_t system_control;
            uint8_t multimedia_control;
            uint8_t general_status;
            uint16_t reserved2;
            uint8_t gpio_control[4];
            uint32_t irq_multiplexer_routing;
            uint8_t retry_status;
            uint8_t card_control;
            uint8_t device_control;
            uint8_t device_diagnostic;
        } header_0x2;
    } __pack;
    uint8_t device_specific[3948];
} __align(4) __pack;
struct pci_config_ptr
{
    pci_config_space* config_start;
    struct
    {
        uint16_t segment_group;
        uint8_t start_bus;
        uint8_t end_bus;
    };
    uint32_t reserved;
} __pack;
struct pci_config_table
{
    struct acpi_header hdr;
    uint8_t reserved[8];
    pci_config_ptr addr_allocations[];
} __pack;
pci_config_table* find_pci_config(xsdt_t* xsdt);
pci_config_space* get_device(pci_config_table* tb, uint8_t bus, uint8_t slot, uint8_t func);
vaddr_t compute_base_address(uint32_t bar_registers[], uint8_t i);
vaddr_t compute_base(uint32_t bar);
class pci_device_list : public std::vector<pci_config_space*>
{
    constexpr pci_config_space* __add(pci_config_space* s) { if(s) this->push_back(s); return s; }
    pci_device_list() = default;
    void add_all(pci_config_table* tb);
    static pci_device_list __instance;
public:
    pci_device_list(pci_device_list&&) = delete;
    pci_device_list(pci_device_list const&) = delete;
    static bool init_instance(xsdt_t* xsdt);
    static pci_device_list* get_instance();
    pci_config_space* find(uint8_t device_class, uint8_t subclass);
};

#endif