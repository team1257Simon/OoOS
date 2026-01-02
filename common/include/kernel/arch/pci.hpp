#ifndef __PERIPHERAL_CONTROL
#define __PERIPHERAL_CONTROL
#include <libk_decls.h>
enum ht
{
	st	= 0x0,	// standard
	br	= 0x1,	// pci-to-pci bridge
	cb	= 0x2	// cardbus bridge
};
enum class pci_capability_id : uint8_t
{
	POWER_MANAGEMENT	= 0x01UC,
	MSI					= 0x05UC,
	PCIE				= 0x10UC,
	MSIX				= 0x11UC,
	SATA				= 0x12UC,
	// ...
};
struct attribute(packed, aligned(4)) pci_config_space
{
	uint16_t vendor_id;
	uint16_t device_id;
	struct
	{
		bool io_space				: 1;
		bool memory_space			: 1;
		bool bus_master				: 1;
		bool						: 3;
		bool parity_error_response	: 1;
		bool						: 1;
		bool serrno_enable			: 1;
		bool						: 1;
		bool interrupt_disable		: 1;
		bool						: 5;
	} command;
	struct
	{
		bool						: 3;
		bool interrupt_status		: 1;
		bool capabilities_list		: 1;	// hardwired to 1 for pcie
		bool						: 3;
		bool master_parity_error	: 1;
		bool						: 2;
		bool signaled_target_abort	: 1;
		bool received_target_abort	: 1;
		bool received_master_abort	: 1;
		bool signaled_system_error	: 1;
		bool detected_parity_error	: 1;
	} status;
	uint8_t revision_id;
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class_code;
	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;
	union
	{
		struct
		{
			uint32_t bar[6];				// base address register; not to be confused with foo
			uint32_t cardbus_cis_pointer;
			uint16_t subsystem_vendor_id;
			uint16_t subsystem_id;
			uint32_t expansion_rom_base_addr;
			uint8_t capabilities_pointer;	// bottom two bits are reserved
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
			uint8_t capabilities_pointer;
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
};
struct __pack pci_msix_bir
{
	uint8_t bar_idx			: 3;	// which BAR is used for the MSI-X base address
	uint32_t table_offset	: 29;
	constexpr operator size_t() const noexcept { if consteval { return static_cast<size_t>(table_offset) << 3; } else { return static_cast<size_t>(std::bit_cast<uint32_t>(*this) & ~0b111U); } }
};
struct attribute(packed, aligned(4)) pci_capabilities_register
{
	pci_capability_id id;
	uint8_t next;			// offset from the start of the config space to the next capabilities register
	volatile union
	{
		struct
		{
			struct
			{
				uint8_t version		: 3;
				bool				: 2;
				bool dsi			: 1;
				uint8_t aux_current	: 3;
				bool d1_suppport	: 1;
				bool d2_support		: 1;
				uint8_t pme_support : 5;
			} __pack capabilities;
			struct
			{
				uint8_t power_state	: 2;
				bool				: 6;
				bool pme_enable		: 1;
				uint8_t data_select	: 4;
				uint8_t data_scale	: 2;
				bool pme_status		: 1;
			} __pack ctl_status;
			uint8_t bridge_support_extensions;
			uint8_t data;
		} __pack power_management;
		struct
		{
			struct
			{
				bool msi_enable					: 1;
				uint8_t multi_message_capable	: 3;
				uint8_t multi_message_enable	: 3;
				bool x64_capable				: 1;
				bool							: 8;
			} __pack message_control;
			uint32_t message_address;
			uint16_t message_data;
			short								: 16;
			uint32_t mask_bits;
			uint32_t pending_bits;
		} __pack msi_32;
		struct
		{
			struct
			{
				bool msi_enable					: 1;
				uint8_t multi_message_capable	: 3;
				uint8_t multi_message_enable	: 3;
				bool x64_capable				: 1;
				bool							: 8;
			} __pack message_control;
			uint64_t message_address;
			uint16_t message_data;
			short								: 16;
			uint32_t mask_bits;
			uint32_t pending_bits;
		} __pack msi_64;
		struct
		{
			struct
			{
				uint16_t table_size	: 11;
				bool				: 3;
				bool function_mask	: 1;
				bool msix_enable	: 1;
			} __pack message_control;
			pci_msix_bir table_offset;
			pci_msix_bir pending_bit_array_offset;
		} __pack msix;
		struct
		{
			struct
			{
				uint8_t version						: 4;
				uint8_t dev_port_type				: 4;
				bool slot_implemented				: 1;
				uint8_t interrupt_message_number	: 5;
				bool								: 2;
			} __pack;
			struct
			{
				bool								: 4;
				uint8_t slot_limit_scale			: 2;
				uint8_t slot_power_limit			: 8;
				bool								: 4;
				bool power_indicator_present		: 1;
				bool attention_indicator_present	: 1;
				bool attention_button_present		: 1;
				uint8_t l1_latency_allowance		: 3;
				uint8_t l0s_latency_allowance		: 3;
				bool extended_tag_field				: 1;
				uint8_t phantom_function			: 2;
				uint8_t max_tlp_payload				: 3;
			} __pack;
			struct
			{
				uint32_t capability;
				uint16_t control;
				uint16_t status;
			} __pack device_specific_capabilities;
		} __pack pcie_device_capability;
	} __pack;
};
struct __pack pci_config_ptr
{
	pci_config_space* config_start;
	struct
	{
		uint16_t segment_group;
		uint8_t start_bus;
		uint8_t end_bus;
	};
	uint32_t reserved;
};
struct __pack pci_config_table
{
	acpi_header hdr;
	uint8_t reserved[8];
	pci_config_ptr addr_allocations[];
};
struct bar_desc
{
	bool is_mmap_bar;
	bool is_prefetchable;
	addr_t base_value;
	size_t base_size;
};
inline bar_desc compute_bar_info(pci_config_space volatile& dev, int i)
{
	if(__unlikely(dev.header_type == 0x2 || (i > (dev.header_type == 0x1 ? 2 : 6)))) return bar_desc();
	uint16_t volatile* cmd_reg		= reinterpret_cast<uint16_t volatile*>(std::addressof(dev.command));
	uint16_t cmd_saved				= *cmd_reg;
	barrier();
	dev.command.io_space			= false;
	dev.command.memory_space		= false;
	barrier();
	uint32_t volatile* bar_reg		= std::addressof((dev.header_type == 0x0 ? dev.header_0x0.bar : dev.header_0x1.bar)[i]);
	uint32_t bar_value				= bar_reg[0];
	bool is_io_bar					= (bar_value & BIT(0)) != 0U;
	bool is_long_bar				= is_io_bar ? false : (bar_value & BIT(2)) != 0U;
	bool is_prefetchable			= is_io_bar ? false : (bar_value & BIT(3)) != 0U;
	uint32_t value_mask				= is_io_bar ? 0xFFFFFFFCU : 0xFFFFFFF0U;
	uint32_t masked_value			= bar_value & value_mask;
	uint32_t hi_value				= is_long_bar ? bar_reg[1] : 0U;
	barrier();
	bar_reg[0]						= 0xFFFFFFFFU;
	if(is_long_bar) bar_reg[1]		= 0xFFFFFFFFU;
	barrier();
	uint32_t size_indicator			= bar_reg[0];
	uint32_t size_hi				= is_long_bar ? bar_reg[1] : 0U;
	barrier();
	bar_reg[0]						= bar_value;
	if(is_long_bar) bar_reg[1]		= hi_value;
	barrier();
	*cmd_reg						= cmd_saved;
	barrier();
	return bar_desc
	{
		.is_mmap_bar				= !is_io_bar,
		.is_prefetchable			= is_prefetchable,
		.base_value					= addr_t(qword(masked_value, hi_value)),
		.base_size					= qword(~(size_indicator & value_mask) + 1U, size_hi),
	};
}
inline pci_capabilities_register* get_capability_register(pci_config_space const volatile& device)
{
    if(__unlikely(!device.status.capabilities_list)) return nullptr;
	switch(device.header_type & ~0x80UC)
	{
		case st: return addr_t(std::addressof(device)).plus(device.header_0x0.capabilities_pointer & 0xFCUC);
		case br: return addr_t(std::addressof(device)).plus(device.header_0x1.capabilities_pointer & 0xFCUC);
		case cb: return addr_t(std::addressof(device)).plus(device.header_0x2.capabilities_pointer & 0xFCUC);
		default: return nullptr;
	}
}
inline pci_capabilities_register* find_capability_register(pci_config_space const volatile& device, pci_capability_id id)
{
	for(pci_capabilities_register* reg = get_capability_register(device); reg != nullptr; reg = reg->next ? addr_t(std::addressof(device)).plus(reg->next & 0xFCUC) : nullptr)
		if(reg->id == id)
			return reg;
	return nullptr;
}
#if defined(__KERNEL__) || defined(__LIBK__)
pci_config_table* find_pci_config();
pci_config_space* get_device(pci_config_table* tb, uint8_t bus, uint8_t slot, uint8_t func);
#endif
#endif