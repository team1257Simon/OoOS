#ifndef __AMD64_AHCI
#define __AMD64_AHCI
#include <cstdint>
#include <module.hpp>
#pragma region magic_numbers
constexpr uint32_t sig_sata 					= 0x00000101U;
constexpr uint32_t sig_atapi 					= 0xEB140101U;
constexpr uint32_t sig_semb 					= 0xC33C0101U;
constexpr uint32_t sig_pmul 					= 0x96690101U;
constexpr uint32_t hba_reset 					= 0b1U;
constexpr uint16_t hba_cmd_start 				= 0x0001US;
constexpr uint16_t hba_command_spin_up_disk 	= 0x0002US;
constexpr uint8_t  hba_sud_bit 					= 1UC;
constexpr uint16_t hba_command_fre 				= 0x0010US;
constexpr uint16_t hba_command_fr  				= 0x4000US;
constexpr uint16_t hba_command_cr  				= 0x8000US;
constexpr uint16_t hba_control_det 				= 0x0001US;
constexpr uint32_t hba_enable_interrupts_all 	= 0xFDC000FFU;
constexpr uint32_t hba_error 					= 0x78000000U;
constexpr uint32_t hba_enable_ahci 				= 0x80000002;
constexpr uint32_t prdt_entries_count 			= 8U;
constexpr byte port_active 						= 0x01UC;
constexpr byte port_present 					= 0x03UC;
constexpr uint32_t port_power_on 				= (1U << 28);
constexpr uint8_t busy_bit 						= 7UC;
constexpr uint8_t drq_bit 						= 3UC;
constexpr uint8_t cr_bit 						= 15UC;
constexpr uint8_t fr_bit 						= 14UC;
constexpr uint8_t fre_bit 						= 4UC;
constexpr uint8_t start_bit 					= 0UC;
constexpr uint8_t descriptor_processed_bit 		= 5UC;
constexpr uint8_t soft_reset_bit 				= 0b10;
constexpr uint8_t nien_bit 						= 0b1;
constexpr uint8_t dev_class_storage_controller 	= 1UC;
constexpr uint8_t subclass_sata_controller 		= 6UC;
constexpr uint64_t max_ext_wait 				= 100000000000000UL;
constexpr uint8_t i_state_hi_byte_error 		= 0x78UC;
constexpr uint8_t oos_bit 						= 2;
constexpr uint8_t ooc_bit 						= 4;
constexpr uint8_t bos_bit 						= 1;
constexpr uint8_t bb_bit 						= 5;
enum fis_type : uint8_t
{
    reg_h2d 		= 0x27,
    reg_d2h 		= 0x34,
    dma_activate 	= 0x39,
    dma_setup 		= 0x41,
    data 			= 0x46,
    bist 			= 0x58,
    pio_setup 		= 0x5F,
    device_bits 	= 0xA1
};
enum ata_command : uint8_t
{
	nop 						= 0x00,
	cfa_request_error_code 		= 0x03,
	data_set_management 		= 0x06,
	data_set_management_xl 		= 0x07,
	request_sense_data_ext 		= 0x0B,
	get_physical_status 		= 0x12,
	read_sectors_pio 			= 0x20, // 8-bit
	read_sectors_pio_ext 		= 0x24, // 16-bit
	read_dma_ext 				= 0x25,
	read_atapi 					= 0xA8,
	write_atapi 				= 0xAA,
	read_stream_dma_ext 		= 0x2A,
	read_stream_ext 			= 0x2B,
	read_log_ext 				= 0x2F,
	write_sectors_pio	 		= 0x30, // 8-bit
	write_sectors_pio_ext 		= 0x34, // 16-bit 
	write_dma_ext 				= 0x35,
	cfa_write_headers_no_erase 	= 0x38,
	write_stream_dma_ext 		= 0x3A,
	write_stream_ext 			= 0x3B,
	write_log_ext 				= 0x3F,
	read_verify_sectors 		= 0x40,
	read_verify_sectors_ext 	= 0x42,
	zero_ext 					= 0x44,
	write_uncorrectable_ext 	= 0x45,
	read_log_dma_ext 			= 0x47,
	zac_management_in 			= 0x4A,
	configure_stream 			= 0x51,
	write_log_dma_ext 			= 0x57,
	trusted_non_data 			= 0x5B,
	trusted_receive 			= 0x5C,
	trusted_receive_dma 		= 0x5D,
	trusted_send 				= 0x5E,
	trusted_send_dma 			= 0x5F,
	read_fpdma_q 				= 0x60,
	write_fpdma_q				= 0x61,
	ncq_non_data 				= 0x63,
	send_fdpma_q 				= 0x64,
	receive_fpdma_q 			= 0x65,
	set_datetime_ext 			= 0x77,
	max_addr_config 			= 0x78,
	remove_element_truncate 	= 0x7C,
	restore_elements_rebuild 	= 0x7D,
	remove_element_modify_zones = 0x7E,
	cfa_translate_sector		= 0x87,
	execute_diagnostic 			= 0x90,
	download_microcode 			= 0x92,
	download_microcode_dma 		= 0x93,
	mutate_ext 					= 0x96,
	zac_management_out 			= 0x9F,
	smart 						= 0xB0,
	set_sector_config_ext 		= 0xB2,
	samotoze 					= 0xB4,
	read_dma 					= 0xC8, // 8-bit
	write_dma 					= 0xCA, // 8-bit
	cfa_write_multiple_no_erase = 0xCD,
	standby_immediate 			= 0xE0,
	idle_immediate 				= 0xE1,
	standby 					= 0xE2,
	idle 						= 0xE3,
	read_buffer 				= 0xE4,
	chk_power_mode 				= 0xE5,
	sleep 						= 0xE6,
	flush_cache 				= 0xE7,
	write_buffer 				= 0xE8,
	read_buffer_dma 			= 0xE9,
	flush_cache_ext 			= 0xEA,
	write_buffer_dma 			= 0xEB,
	identify 					= 0xEC,
	set_features 				= 0xEF,
	set_password 				= 0xF1,
	sec_unlock 					= 0xF2,
	sec_erase_prepare 			= 0xF3,
	sec_erase_unit 				= 0xF4,
	sec_freeze_lock 			= 0xF5,
	disable_password 			= 0xF6
};
#pragma endregion
enum ahci_device : uint8_t
{
    none,
    sata,
    semb,
    pmul,
    atapi
};
struct identify_data
{
	uint16_t low_data[60]; // this data isn't that useful at the moment
	uint32_t logical_sectors_28b;
	uint16_t mid_data[26]; // more things we don't need yet
	uint16_t udma_modes;
	uint16_t more_data[11]; // yet more things we don't need yet
	uint64_t logical_sectors_48b;
	uint16_t transfer_time;
	uint16_t high_data[150];
	uint16_t integrity;
} __pack;
struct fis_reg_h2d
{
    fis_type type;
    uint8_t pmport  : 4;
    uint8_t rsv0    : 3;
    uint8_t ctype   : 1; 	// 1 = command, 0 = control
    uint8_t command;
    uint8_t feature_1o;
    // DWORD 1
	uint8_t  lba0;			// LBA low register, 7:0
	uint8_t  lba1;			// LBA mid register, 15:8
	uint8_t  lba2;			// LBA high register, 23:16
	uint8_t  device;		// Device register
	// DWORD 2
	uint8_t  lba3;			// LBA register, 31:24
	uint8_t  lba4;			// LBA register, 39:32
	uint8_t  lba5;			// LBA register, 47:40
	uint8_t  feature_hi;	// Feature register, 15:8
	// DWORD 3
	uint16_t count;
	uint8_t  icc;		    // Isochronous command completion
	uint8_t  control;	    // Control register
	// DWORD 4
	uint8_t  rsv1[4];		// Reserved
} __pack;
struct fis_reg_d2h
{
    fis_type type;
    uint8_t  pmport     : 4;    // Port multiplier
	uint8_t  rsv0       : 2;    // Reserved
	uint8_t  interrupt  : 1;    // Interrupt bit
	uint8_t  rsv1       : 1;    // Reserved
	uint8_t  status;      		// Status register
	uint8_t  error;      		// Error register
	// DWORD 1
	uint8_t  lba0;        // LBA low register, 7:0
	uint8_t  lba1;        // LBA mid register, 15:8
	uint8_t  lba2;        // LBA high register, 23:16
	uint8_t  device;      // Device register
	// DWORD 2
	uint8_t  lba3;        // LBA register, 31:24
	uint8_t  lba4;        // LBA register, 39:32
	uint8_t  lba5;        // LBA register, 47:40
	uint8_t  rsv2;        // Reserved
	// DWORD 3
	uint8_t  countl;      // Count register, 7:0
	uint8_t  counth;      // Count register, 15:8
	uint8_t  rsv3[2];     // Reserved
	// DWORD 4
	uint8_t  rsv4[4];     // Reserved
} __pack;
struct fis_data
{
    fis_type type;
    uint8_t  pmport : 4;	// Port multiplier
	uint8_t  rsv0   : 4;	// Reserved
	uint8_t  rsv1[2];   	// Reserved
    uint32_t data[];
} __pack;
struct fis_pio_setup
{
    fis_type type;
    uint8_t  pmport     : 4;	// Port multiplier
	uint8_t  rsv0       : 1;	// Reserved
	uint8_t  direction  : 1;	// Data transfer direction, 1 - device to host
	uint8_t  interrupt  : 1;	// Interrupt bit
	uint8_t  rsv1       : 1;
	uint8_t  status;	// Status register
	uint8_t  error;		// Error register
	// DWORD 1
	uint8_t  lba0;		// LBA low register, 7:0
	uint8_t  lba1;		// LBA mid register, 15:8
	uint8_t  lba2;		// LBA high register, 23:16
	uint8_t  device;	// Device register
	// DWORD 2
	uint8_t  lba3;		// LBA register, 31:24
	uint8_t  lba4;		// LBA register, 39:32
	uint8_t  lba5;		// LBA register, 47:40
	uint8_t  rsv2;		// Reserved
	// DWORD 3
	uint8_t  countl;	// Count register, 7:0
	uint8_t  counth;	// Count register, 15:8
	uint8_t  rsv3;		// Reserved
	uint8_t  e_status;	// New value of status register
	// DWORD 4
	uint16_t tc;		// Transfer count
	uint8_t  rsv4[2];	// Reserved
} __pack;
struct fis_dma_setup
{
    uint8_t  fis_type;	            // FIS_TYPE_DMA_SETUP
	uint8_t  pmport         : 4;    // Port multiplier
	uint8_t  rsv0           : 1;    // Reserved
	uint8_t  direction      : 1;    // Data transfer direction, 1 - device to host
	uint8_t  interrupt      : 1;    // Interrupt bit
	uint8_t  auto_activate  : 1;    // Auto-activate. Specifies if DMA Activate FIS is needed
    uint8_t  rsv1[2];   			// Reserved
	//DWORD 1&2
    uint64_t dma_buffer_id;   		// DMA Buffer Identifier. Used to Identify DMA buffer in host memory.
    // SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
    //DWORD 3
    uint32_t rsv2;          		// More reserved
    //DWORD 4
    uint32_t dma_buffer_offset;  	// Byte offset into buffer. First 2 bits must be 0
    //DWORD 5
    uint32_t transfer_count; 		// Number of bytes to transfer. Bit 0 must be 0
    //DWORD 6
    uint32_t rsv3;          		// So many reserved bytes smh
} __pack;
struct fis_set_bits
{
    fis_type type;
    uint8_t pmport      : 4;	// Port multiplier
	uint8_t rsv0        : 2;	// Reserved
    uint8_t interrupt   : 1;	// Interrupt bit
    uint8_t notif       : 1;    // Notification bit
    uint8_t status_lo   : 3;
    uint8_t rsv1        : 1;
    uint8_t status_hi   : 3;
    uint8_t rsv2        : 1;
    uint8_t error;
    uint32_t protocol_specific;
} __pack;
typedef volatile struct tfis
{
    fis_dma_setup dma_setup_f;
    uint8_t pad0[4];
    fis_pio_setup pio_setup_f;
    uint8_t pad1[12];
    fis_reg_d2h dev_reg_f;
    uint8_t pad2[4];
    fis_set_bits set_bits_f;
    uint8_t ufis[64];
    uint8_t rsv[96];
} __pack hba_fis;
struct hba_prdt_entry
{
	uint32_t data_base;		// Data base address, little-endian, low
	uint32_t data_base_hi;
	uint32_t rsv0;			// Reserved
	// DW3
	uint32_t byte_count 			: 22;
	uint32_t rsv1 					: 9;
	bool interrupt_on_completion 	: 1;
} __pack;
struct hba_cmd_table
{
    // 0x00
	uint8_t  cmd_fis[64];	// Command FIS
	// 0x40
	uint8_t  atapi_cmd[16];	// ATAPI command, 12 or 16 bytes
	// 0x50
	uint8_t  rsv[48];		// Reserved
    hba_prdt_entry prdt_entries[prdt_entries_count];
} __pack;
struct hba_cmd_header
{
    // DW0
	uint8_t  cmd_fis_len    : 5;		// Command FIS length in DWORDS, 2 ~ 16
	bool     atapi          : 1;		// ATAPI
	bool     w_direction    : 1;		// Write, 1: H2D, 0: D2H
	bool     prefetch       : 1;		// Prefetchable
	bool     reset          : 1;		// Reset
	bool     bist           : 1;		// BIST
	uint8_t  cl_busy        : 1;		// Clear busy upon R_OK
	uint8_t  rsv0           : 1;    	// Reserved
	uint8_t  pm_port        : 4;		// Port multiplier port
	uint16_t prdt_length;		        // Physical region descriptor table length in entries
	// DW1
	volatile uint32_t prd_count;		// Physical region descriptor byte count transferred
	// DW2, 3
	hba_cmd_table* command_table;	
	// DW4 - 7
	uint32_t rsv1[4];					// Reserved
} __pack;
typedef volatile struct tport
{
	hba_cmd_header* command_list;
	hba_fis* 	fis_receive;
	uint32_t   	i_state;	// 0x10, interrupt status
	uint32_t   	i_enable;   // 0x14, interrupt enable
	uint32_t   	cmd;		// 0x18, command and status
	uint32_t   	rsv0;		// 0x1C, Reserved
	uint32_t   	task_file;	// 0x20, task file data
	uint32_t   	sig;		// 0x24, signature
	uint32_t   	s_status;	// 0x28, SATA status (SCR0:SStatus)
	uint32_t   	s_control;	// 0x2C, SATA control (SCR2:SControl)
	uint32_t   	s_err;		// 0x30, SATA error (SCR1:SError)
	uint32_t   	s_active;	// 0x34, SATA active (SCR3:SActive)
	uint32_t   	cmd_issue;	// 0x38, command issue
	uint32_t   	s_notif;	// 0x3C, SATA notification (SCR4:SNotification)
	uint32_t   	fb_switch;  // 0x40, FIS-based switch control
	uint32_t 	rsv1[11];	// 0x44 ~ 0x6F, Reserved
	uint32_t 	vendor[4];	// 0x70 ~ 0x7F, vendor specific
} __pack hba_port;
typedef volatile struct tmem
{
    // 0x00 - 0x2B, Generic Host Control
	struct
	{
		bool ext_addr 		: 1;
		bool native_q 		: 1;
		bool s_notif  		: 1;
		bool mech_pres 		: 1;
		bool sud_supported 	: 1;
		bool agg_link_p		: 1;
		bool act_led		: 1;
		bool cl_overrd		: 1;
		uint8_t speed 		: 4;
		bool rsv0 			: 1;
		bool ahci_only		: 1;
		bool port_mul		: 1;
		bool fis_switch 	: 1;
		bool multi_drq  	: 1;
		bool slumber		: 1;
		bool part_state 	: 1;
		uint8_t num_c_slots : 5;
		bool cccs 			: 1;
		bool ems 			: 1;
		bool ext_sata 		: 1;
		uint8_t ports_max 	: 5;
	};						    // 0x00, Host capability
	uint32_t ghc;		        // 0x04, Global host control
	uint32_t i_status;	        // 0x08, Interrupt status
	uint32_t p_implemented;		// 0x0C, Port implemented
	uint32_t version;   		// 0x10, Version
	uint32_t ccc_control;       // 0x14, Command completion coalescing control
	uint32_t ccc_ports;	        // 0x18, Command completion coalescing ports
	uint32_t em_location;   	// 0x1C, Enclosure management location
	uint32_t em_control;        // 0x20, Enclosure management control
	struct 						// 0x24, Host capabilities extended
	{
		uint32_t rsv1 				: 26;
		bool devsleep_slumber_only 	: 1;
		bool agg_sleep_manage		: 1;
		bool dev_sleep				: 1;
		bool auto_part_slumber		: 1;
		bool nvmhci_present			: 1;
		bool bios_os_handoff_sup	: 1;
	};
	// 0x28, BIOS/OS handoff control and status
	uint32_t bios_os_handoff;
	// 0x2C - 0x9F, Reserved
	uint8_t rsv[116];
	// 0xA0 - 0xFF, Vendor specific registers
	uint8_t vendor[96];
	// 0x100 - 0x10FF, Port control registers
	hba_port ports[32];
} __pack hba_mem;
/**
 * Config parameters:
 *      ahci_region_size    : qword (size_t)    — how much DMA space to allocate
 *      rw_timeout          : qword (time_t)    — how long to wait on reads and writes before assuming a hung port
 *      start_stop_timeout  : qword (time_t)    — how long to wait on starting or stopping a port before assuming a hung port
 *      disable_ioc         : bool              — if set to true, the interrupt-on-completion bit will not be set for commands sent to the HBA
 */
constexpr auto ahci_config()
{
    return ooos::create_config
    (
        ooos::parameter("ahci_region_size",     ooos::parameter_type<size_t>,   524288U),
        ooos::parameter("rw_timeout",           ooos::parameter_type<time_t>,   10000000L),
        ooos::parameter("start_stop_timeout",   ooos::parameter_type<time_t>,   100000000000000L),
        ooos::parameter("disable_ioc",          ooos::parameter_type<bool>,     false)
    );
}
class amd64_ahci;
class ahci_port : public ooos::abstract_block_device
{
    amd64_ahci& __module;
    hba_port& __port;
	ahci_device __device_type;
	uint32_t __idx;
    stticket __find_cmd_slot();
    void __build_h2d_io_fis(uintptr_t dest_start, size_t sector_count, addr_t buffer, ata_command action, hba_cmd_header& cmd);
    void __cmd_issue(utticket slot);
	void __log_error(ooos::module_eh_ctx& ctx);
public:
    typedef ooos::abstract_block_device::provider io_provider;
    ahci_port(amd64_ahci& m, hba_port& p, uint32_t i, ahci_device dt);
    virtual stticket read(void* dest, uintptr_t src_start, size_t sector_count) override;
    virtual stticket write(uintptr_t dest_start, const void* src, size_t sector_count) override;
    virtual size_t block_size() const noexcept override;
    virtual bool io_busy() const override;
    virtual bool io_complete(utticket task_ticket) const override;
    virtual size_t io_count(utticket task_ticket) const override;
    virtual ooos::blockdev_type device_type() const noexcept override;
    void start_port();
    void stop_port();
    bool init_port(addr_t region_base);
    void soft_reset();
    void hard_reset();
};
class amd64_ahci : public ooos::abstract_module_base, public ahci_port::io_provider
{
    pci_config_space* __ahci_pci_cfg;
    hba_mem* __abar;
    ahci_port* __ports[32];
	uidx_t __device_count;
    ooos::empty_blockdev_slot __empty_placeholder;
    char __port_struct_data[32][sizeof(ahci_port)];
    addr_t __dma_block;
	size_t __dma_size;
    typedef decltype(ahci_config()) __config_type;
    static __config_type __cfg;
	friend class ahci_port;
	void __handle_irq(dword s);
	void __irq_init();
public:
    amd64_ahci() noexcept;
    virtual uidx_t count() const noexcept override;
    virtual sidx_t index_of(ooos::blockdev_type dev_type) const noexcept override;
    virtual ooos::abstract_block_device& operator[](uidx_t idx) override;
    virtual bool initialize() override;
    virtual void finalize() override;
    virtual ooos::generic_config_table& get_config() override;
};
#endif