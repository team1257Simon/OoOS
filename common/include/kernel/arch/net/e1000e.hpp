#ifndef __PCIE_GIGABIT_ETHERNET
#define __PCIE_GIGABIT_ETHERNET
#include "arch/pci.hpp"
#include "arch/simple_io_port.hpp"
#include "kernel_mm.hpp"
struct e1000e_device_control_register
{
    bool full_duplex                    : 1;
    bool                                : 1;
    bool gio_master_disable             : 1;
    bool link_reset                     : 1;
    bool                                : 2;
    bool set_link_up                    : 1;
    bool                                : 1;
    uint8_t speed_select                : 2;
    bool                                : 1;
    bool speed_force                    : 1;
    bool duplex_force                   : 1;
    bool                                : 3;
    bool                                : 3;
    bool sdp0_data                      : 1;
    bool dock_undock_status             : 1;
    bool sdp1_data                      : 1;
    bool d2cold_wakeup_capability_adv   : 1;
    bool                                : 1;
    bool sdp0_dir                       : 1;
    bool sdp1_dir                       : 1;
    bool                                : 2;
    bool reset                          : 1;
    bool rx_flow_control_enable         : 1;
    bool tx_flow_control_enable         : 1;
    bool                                : 1;
    bool vlan_mode_enable               : 1;
    bool phy_reset                      : 1;
} attribute(packed, aligned(4));
struct e1000e_device_status_register
{
    bool full_duplex                : 1;
    bool link_up                    : 1;
    uint8_t lan_id                  : 2;
    bool tx_paused                  : 1;
    bool tbi_mode                   : 1;
    uint8_t speed                   : 2;
    uint8_t auto_speed_detected     : 2;
    bool phy_reset_asserted         : 1;
    bool                            : 5;
    bool                            : 3;
    bool gio_master_status          : 1;
    bool                            : 4;
    bool                            : 8;
} attribute(packed, aligned(4));
struct e1000e_eeprom_flash_control_register
{
    bool nvm_clock_bit                 : 1;
    bool nvm_chip_select               : 1;
    bool nvm_data_write_line           : 1;
    bool nvm_data_read_line            : 1;
    uint8_t flash_write_enable_control : 2;
    bool nvm_access_request            : 1;
    bool nvm_access_granted            : 1;
    const bool nvm_present             : 1;
    const bool nvm_auto_read_done      : 1;
    bool                               : 1;
    const uint8_t nvm_size             : 4;
    uint8_t nvm_address_size           : 2;
    bool                               : 3;
    bool auto_flash_update_enable      : 1;
    bool                               : 1;
    bool sector_1_valid                : 1;
    const bool nvm_type                : 1;  // 0 = eeprom, 1 = flash
    uint8_t rsvd;
} attribute(packed, aligned(4));
struct e1000e_eeprom_read_register
{
    bool read_start         : 1;
    bool read_done          : 1;
    uint16_t read_address   : 14;
    uint16_t read_data;
} attribute(packed, aligned(4));
struct e1000e_flash_access_register
{
    bool clock                 : 1;
    bool chip_select           : 1;
    bool data_write            : 1;
    bool data_read             : 1;
    bool access_request        : 1;
    bool access_granted        : 1;
    bool erase_transaction     : 1;
    bool write_transaction     : 1;
    bool write_done            : 1;
    bool                       : 7;
    bool                       : 8;
    bool                       : 6;
    bool flash_busy            : 1;
    bool flash_erase_command   : 1;
} attribute(packed, aligned(4));
struct e1000e_mdi_control_register
{
    uint16_t data;
    uint8_t phy_register_address   : 5;
    uint8_t phy_address            : 5;
    uint8_t operation              : 2; // 0b01 = read, 0b10 = write
    bool ready                     : 1;
    bool interrupt_enable          : 1;
    bool error                     : 1;
    bool                           : 1;
} attribute(packed, aligned(4));
struct e1000e_extended_control_register
{
    bool                            : 2;
    bool sdp2_gpi_enable            : 1;
    bool sdp3_gpi_enable            : 1;
    bool                            : 2;
    bool sdp2_data                  : 1;
    bool sdp3_data                  : 1;
    bool                            : 2;
    bool sdp2_dir                   : 1;
    bool sdp3_dir                   : 1;
    bool auto_speed_detection_check : 1;
    bool eeprom_reset               : 1;
    bool                            : 1;
    bool speed_select_bypass        : 1;
    bool                            : 1;
    bool relaxed_ordering_disable   : 1;
    bool serdes_low_power_enable    : 1;
    bool dma_dynamic_gate_enable    : 1;
    bool phy_power_down_enable      : 1;
    bool                            : 1;
    uint8_t link_mode               : 2;
    bool large_send                 : 1;
    bool large_send_flow_off        : 1;
    bool packet_parity              : 1;
    bool descriptor_parity          : 1;
    bool                            : 1;
    bool interrupt_ack_auto_mask    : 1;
    bool driver_loaded              : 1;
    bool                            : 1;
    bool host_data_fifo_parity      : 1;
    bool                            : 1;
} attribute(packed, aligned(4));
template<typename T> union e1000e_register_dword
{
    uint32_t data_dword{};
    T data_struct;
    constexpr operator uint32_t&() & noexcept { return data_dword; }
    constexpr operator uint32_t() const noexcept { return data_dword; }
    constexpr operator T&() & noexcept { return data_struct; }
    constexpr operator T const&() const& noexcept { return data_struct; }
    constexpr operator T volatile&() volatile& noexcept { return data_struct; }
    constexpr operator T const volatile&() const volatile& noexcept { return data_struct; }
};
struct e1000e_nvm_config_table
{
    word ethernet_address[3];
    struct alignas(uint16_t)
    {
        bool                        : 2;
        bool pci_bridge_present     : 1;
        bool                        : 1;
        bool smbus_connected        : 1;
        bool                        : 3;
        bool is_oem                 : 1;
        bool is_client              : 1;
        bool is_server              : 1;
        bool is_lom                 : 1;
        bool asf_smbus_connected    : 1;
        bool                        : 3;
    } __pack;
    struct alignas(uint16_t)
    {
        uint8_t led_0_control       : 4;
        uint8_t led_1_control       : 4;
        uint8_t led_2_control       : 4;
        uint8_t                     : 4;
    } __pack;
    uint16_t rsv0[3];
    uint16_t printed_board_assembly_number[2];
    uint16_t icw1;
    uint16_t subsystem_id;
    uint16_t subsystem_vendor_id;
    uint16_t device_id;
    uint16_t rsv1;
    uint16_t icw2;
    uint16_t nvm_protected[3];
    uint16_t rsv2[4];
    uint16_t pcie_electrical_delay;
    uint16_t pcie_icw[3];
    struct alignas(uint16_t)
    {
        uint8_t l1_latency              : 2;
        bool electrical_idle            : 1;
        bool                            : 1;
        bool skip_disable               : 1;
        bool l2_disable                 : 1;
        bool                            : 1;
        uint8_t msi_x_num               : 3;
        bool leaky_bucket_disable       : 1;
        bool force_recovery             : 1;
        bool pcie_ltssm                 : 1;
        bool pcie_down_reset_disable    : 1;
        uint8_t l1_latency_hi           : 1;
        bool pcie_receive_valid         : 1;
    } __pack;
    struct alignas(uint16_t)
    {
        uint8_t led1_mode               : 4;
        bool                            : 1;
        bool led1_blink_mode            : 1;
        bool led1_inver                 : 1;
        bool led1_blink                 : 1;
        bool                            : 1;
        bool d0_low_power_link_up       : 1;
        bool low_power_link_up          : 1;
        bool non_d0a_gigabit_disable    : 1;
        bool class_a_mode               : 1;
        bool rsv7                       : 1;
        bool gigabit_disable            : 1;
        bool                            : 1;
    } __pack;
    uint16_t rsv3[2];
    struct alignas(uint16_t)
    {
        uint8_t led0_mode       : 4;
        bool                    : 1;
        bool led0_blink_mode    : 1;
        bool led0_invert        : 1;
        bool led0_blink         : 1;
        uint8_t led2_mode       : 4;
        bool                    : 1;
        bool led2_blink_mode    : 1;
        bool led2_invert        : 1;
        bool led2_blink         : 1;
    } __pack;
    uint16_t flash_param;
    struct alignas(uint16_t)
    {
        bool                                : 7;
        bool lan_expansion_disable          : 1;
        uint8_t lan_flash_address_addend    : 4;
        uint8_t lan_flash_size_shift        : 3;
        bool lan_flash_disable              : 1;
    } __pack;
    uint16_t lan_power_consumption;
    uint16_t flash_sw_detection_word;       // 0xFFFF enables vendor detection; otherwise keep as 0x3FFF
    uint16_t icw3;
    uint16_t apt_hw_words[6];
    uint16_t firmware_id[2];
    uint16_t ncsi_config[2];
    uint16_t vpd_ptr;
    uint16_t pxe_words[14];
    uint16_t checksum;                      // checksum of the above block, such that adding the whole thing equals 0xBABA
} __pack;
union e1000e_receive_descriptor
{
	struct
    {
		uintptr_t buffer_addr;
		uint64_t reserved;
	} read;
	struct
    {
		__le32 multi_queue;
        union
        {
            __le32 rss;	                    /* RSS Hash */
            struct
            {
                __le16 ip_id;               /* IP id */
                __le16 csum;                /* Packet Checksum */
            } __pack;
        } __pack;
		__le32 status_error;                /* ext status/error */
        __le16 length;
        __le16 vlan;	                    /* VLAN tag */
	} __pack wb;                            /* writeback */
};
union e1000e_receive_descriptor_packet_split
 {
	struct { uintptr_t buffer_addr[4]; } read; /* one buffer for protocol header(s), three data buffers */
	struct 
    {
		__le32 multi_queue;
        union 
        {
            __le32 rss;	            /* RSS Hash */
            struct 
            {
                __le16 ip_id;       /* IP id */
                __le16 csum;        /* Packet Checksum */
            } __pack;
        } __pack;
		__le32 status_error;        /* ext status/error */
        __le16 length0;             /* Length of buffer 0 */
        __le16 vlan;	            /* VLAN tag */
		__le16 header_status;
		__le16 length[3];           /* length of buffers 1-3 */
		uint64_t reserved;
	} __pack wb;                    /* writeback */
};
struct e1000e_transmit_descriptor
{
	uintptr_t buffer_addr;          /* Address of the descriptor's data buffer */
	union 
    {
		__le32 data_lower;
		struct 
        {
			__le16 length;          /* Data buffer length */
			byte cso;	            /* Checksum offset */
			byte cmd;	            /* Descriptor control */
		} flags;
	};
	union 
    {
		__le32 data_upper;
		struct 
        {
			byte status;            /* Descriptor status */
			byte css;	            /* Checksum start */
			__le16 special;
		} fields;
	};
};
struct e1000e_offload_context_descriptor 
{
	union 
    {
		__le32 ip_config;
		struct 
        {
			byte ipcss;             /* IP checksum start */
			byte ipcso;             /* IP checksum offset */
			__le16 ipcse;           /* IP checksum end */
		} ip_fields;
	};
	union 
    {
		__le32 tcp_config;
		struct 
        {
			byte tucss;         /* TCP checksum start */
			byte tucso;         /* TCP checksum offset */
			__le16 tucse;       /* TCP checksum end */
		} tcp_fields;
	};
	__le32 cmd_and_length;
	union 
    {
		__le32 seg_config;
		struct 
        {
			byte status;        /* Descriptor status */
			byte hdr_len;       /* Header length */
			__le16 mss;         /* Maximum segment size */
		} seg_fields;
	};
};
struct e1000e_offload_data_descriptor 
{
	uintptr_t buffer_addr;   /* Address of the descriptor's buffer address */
	union 
    {
		__le32 data_lower;
		struct
        {
			__le16 length;    /* Data buffer length */
			byte typ_len_ext;
			byte cmd;
		} flags;
	};
	union 
    {
		__le32 data_upper;
		struct 
        {
			byte status;     /* Descriptor status */
			byte popts;      /* Packet Options */
			__le16 special;
		} fields;
	};
};
constexpr size_t max_single_tx_buffer = 16288UZ;
constexpr size_t max_single_rx_buffer = 16384UZ;
template<typename T>
struct e1000e_ring
{
    T* descriptors;
    size_t descriptor_count;
    addr_t buffer_region_start;
    size_t buffer_region_size;
};
// note: when this is done, it will be a class, not a struct
struct e1000e
{
    pci_config_space* __pcie_e1000e_controller;
    simple_io_port<uint32_t> __io_addr_port;
    simple_io_port<uint32_t> __io_data_port;
    e1000e_ring<e1000e_receive_descriptor> __rx_ring;
    e1000e_ring<e1000e_transmit_descriptor> __tx_ring;
    bool __has_init;
    static e1000e __instance;
    bool __init(size_t descriptor_count_factor = 16UZ);
    e1000e();
public:
    static e1000e& get_instance();
    static bool init_instance();
};
#endif