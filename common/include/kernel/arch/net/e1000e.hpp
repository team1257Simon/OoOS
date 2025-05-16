#ifndef __PCIE_GIGABIT_ETHERNET
#define __PCIE_GIGABIT_ETHERNET
#include "arch/pci.hpp"
#include "arch/simple_io_port.hpp"
#include "kernel_mm.hpp"
#include "net/netdev.hpp"
#include "arch/net/e1000e_constants.hpp"
struct attribute(packed, aligned(4)) e1000e_device_control_register
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
    bool                                : 5;
    bool sdp0_data                      : 1;
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
};
struct attribute(packed, aligned(4)) e1000e_device_status_register
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
};
struct attribute(packed, aligned(4)) e1000e_eeprom_flash_control_register
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
};
struct attribute(packed, aligned(4)) e1000e_eeprom_read_register
{
    bool read_start         : 1;
    bool read_done          : 1;
    uint16_t read_address   : 14;
    uint16_t read_data;
};
struct attribute(packed, aligned(4)) e1000e_flash_access_register
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
};
struct attribute(packed, aligned(4)) e1000e_mdi_control_register
{
    uint16_t data;
    uint8_t phy_register_address   : 5;
    uint8_t phy_address            : 5; // always 1 for our purposes
    uint8_t operation              : 2; // 0b01 = read, 0b10 = write
    bool ready                     : 1;
    bool interrupt_enable          : 1;
    bool error                     : 1;
    bool                           : 1;
};
struct attribute(packed, aligned(4)) e1000e_extended_control_register
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
    bool packet_parity              : 1;
    bool descriptor_parity          : 1;
    bool                            : 1;
    bool interrupt_ack_auto_mask    : 1;
    bool driver_loaded              : 1;
    bool                            : 1;
    bool host_data_fifo_parity      : 1;
    bool                            : 1;
};
struct attribute(packed, aligned(4)) e1000e_rx_control_register
{
    bool                            : 1;
    bool enable                     : 1;
    bool store_bad_packets          : 1;
    bool unicast_promiscuous        : 1;
    bool multicast_promiscuous      : 1;
    bool long_packet_enable         : 1;
    uint8_t loopback_mode           : 2; // use (0->normal, 1->MAC) loopback
    uint8_t rdtms                   : 2; // sets the RDMT interrupt when the number of free descriptors equals one (0->half, 1 -> quarter, 2->eighth) of total descriptors
    uint8_t desc_type               : 2; // use (0->legacy, 1->packet-split) descriptors
    uint8_t multicast_offset        : 2; // filter multicast packets based on bits (0->47-36, 1->46-35, 2->45-34, 3->43-32) of multicast address
    bool                            : 1;
    bool broadcast_accept           : 1;
    uint8_t buffer_size_shift       : 2; // buffer_size = (extended_buffer_size ? 4096 : 256) << (3 - buffer_size_shift); value 0 is reserved if the extended size bit is set
    bool vlan_filter_enable         : 1;
    bool canonical_form_enable      : 1;
    bool canonical_form_indicator   : 1;
    bool                            : 1;
    bool discard_pause_frames       : 1;
    bool pass_mac_control_frames    : 1;
    bool                            : 1;
    bool extended_buffer_size       : 1;
    bool strip_ethernet_crc         : 1;
    uint8_t flex_buffer_size        : 4; // overrides buffer size calculation above, value in KiB
    bool                            : 1;
};
struct attribute(packed, aligned(4)) e1000e_tx_control_register
{
    bool                            : 1;
    bool enable                     : 1;
    bool                            : 1;
    bool pad_short_packets          : 1;
    uint8_t collision_threshold     : 8;
    uint16_t collision_distance     : 10;
    bool software_xoff              : 1;
    bool                            : 1;
    bool retransmit_late_collision  : 1;
    bool underrun_no_retransmit     : 1;
    uint8_t tdtms                   : 2; // as rdtms but for transmit desccriptors
    bool multi_request_support      : 1;
    bool                            : 3;
};
struct attribute(packed, aligned(4)) e1000e_rx_desc_control_register
{
    uint8_t prefetch_thresh         : 6;
    bool                            : 2;
    uint8_t host_thresh             : 6;
    bool                            : 2;
    uint8_t writeback_thresh        : 6;
    bool                            : 2;
    bool descriptor_granularity     : 1; // 0 means cache line granularity
    bool           					: 7;
};
struct attribute(packed, aligned(4)) e1000e_tx_desc_control_register
{
    uint8_t prefetch_thresh         : 6;
    bool                            : 2;
    uint8_t host_thresh             : 6;
    bool                            : 2;
    uint8_t writeback_thresh        : 6;
    bool rsv_preserved              : 1; // a reserved bit that will be in its correct state on boot (depends on model)
    bool                            : 1;
    bool descriptor_granularity     : 1; // 0 means cache line granularity
    uint8_t low_threshold           : 7;
};
struct attribute(packed, aligned(4)) e1000e_interrupt_cause_register
{
    bool tx_desc_writeback          : 1;
    bool tx_queue_empty             : 1;
    bool link_status_change         : 1;
    bool                            : 1;
    bool rxdt_min_thresh            : 1;
    bool                            : 1;
    bool rx_data_overrun            : 1;
    bool rx_timer                   : 1;
    bool                            : 1;
    bool mdio_access_done           : 1;
    bool                            : 5;
    bool txdt_min_thresh            : 1;
    bool rx_small_receive           : 1;
    bool ack_frame                  : 1;
    bool manageability_event        : 1;
    bool                            : 1;
    bool rxq0                       : 1;
    bool rxq1                       : 1;
    bool txq0                       : 1;
    bool txq1                       : 1;
    bool other                      : 1;
    bool                            : 6;
    bool asserted                   : 1; 
};
template<std::same_size<uint32_t> T> union [[gnu::may_alias]] e1000e_register_dword
{
    typedef uint32_t& dw_ref;
    typedef uint32_t const& dw_cref;
    uint32_t data_dword{};
    T data_struct;
    constexpr operator dw_ref() & noexcept { return data_dword; }
    constexpr operator dw_cref() const& noexcept { return data_dword; }
    constexpr T* operator->() & noexcept { return std::addressof(data_struct); }
    constexpr T const* operator->() const& noexcept { return std::addressof(data_struct); }
};
typedef e1000e_register_dword<e1000e_device_control_register> dev_ctrl;
typedef e1000e_register_dword<e1000e_device_status_register> dev_status;
typedef e1000e_register_dword<e1000e_eeprom_flash_control_register> nvm_ctrl;
typedef e1000e_register_dword<e1000e_eeprom_read_register> ee_read;
typedef e1000e_register_dword<e1000e_flash_access_register> fl_access;
typedef e1000e_register_dword<e1000e_mdi_control_register> mdic;
typedef e1000e_register_dword<e1000e_extended_control_register> ext_ctrl;
typedef e1000e_register_dword<e1000e_rx_control_register> rx_ctrl;
typedef e1000e_register_dword<e1000e_tx_control_register> tx_ctrl;
typedef e1000e_register_dword<e1000e_rx_desc_control_register> rx_desc_ctrl;
typedef e1000e_register_dword<e1000e_tx_desc_control_register> tx_desc_ctrl;
typedef e1000e_register_dword<e1000e_interrupt_cause_register> irq_state;
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
};
union alignas(int128_t) e1000e_receive_descriptor
{
	struct __pack
    {
		uintptr_t buffer_addr;
        uint16_t length;
        uint16_t csum;
        uint8_t status;
		uint8_t errors;
        uint16_t vlan;	                        /* VLAN tag */
	} read;
	struct __pack
    {
		uint32_t multi_queue;
        union __pack
        {
            uint32_t rss;	                    /* RSS Hash */
            struct __pack
            {
                uint16_t ip_id;                 /* IP id */
                uint16_t csum;                  /* Packet Checksum */
            };
        };
		uint32_t status_error_wb;               /* ext status/error */
        uint16_t length_wb;
        uint16_t vlan_wb;	                    /* VLAN tag */
	} wb;                                /* writeback */
};
constexpr auto test = offsetof(e1000e_receive_descriptor, read.status);
union alignas(int128_t) e1000e_receive_descriptor_packet_split
 {
	struct { uintptr_t buffer_addr[4]; } read; /* one buffer for protocol header(s), three data buffers */
	struct 
    {
		uint32_t multi_queue;
        union 
        {
            uint32_t rss;	            /* RSS Hash */
            struct 
            {
                uint16_t ip_id;         /* IP id */
                uint16_t csum;          /* Packet Checksum */
            } __pack;
        } __pack;
		uint32_t status_error;          /* ext status/error */
        uint16_t length0;               /* Length of buffer 0 */
        uint16_t vlan;	                /* VLAN tag */
		uint16_t header_status;
		uint16_t length[3];             /* length of buffers 1-3 */
		uint64_t reserved;
	} __pack wb;                        /* writeback */
};
struct alignas(int128_t) e1000e_transmit_descriptor
{
	uintptr_t buffer_addr;              /* Address of the descriptor's data buffer */
	union 
    {
		uint32_t data_lower;
		struct 
        {
			uint16_t length;            /* Data buffer length */
			uint8_t cso;	            /* Checksum offset */
			uint8_t cmd;	            /* Descriptor control */
		} flags;
	};
	union 
    {
		uint32_t data_upper;
		struct 
        {
			uint8_t status;             /* Descriptor status */
			uint8_t css;	            /* Checksum start */
			uint16_t special;
		} fields;
	};
};
struct e1000e_offload_context_descriptor 
{
	union 
    {
		uint32_t ip_config;
		struct 
        {
			uint8_t ipcss;              /* IP checksum start */
			uint8_t ipcso;              /* IP checksum offset */
			uint16_t ipcse;             /* IP checksum end */
		} ip_fields;
	};
	union 
    {
		uint32_t tcp_config;
		struct 
        {
			uint8_t tucss;              /* TCP checksum start */
			uint8_t tucso;              /* TCP checksum offset */
			uint16_t tucse;             /* TCP checksum end */
		} tcp_fields;
	};
	uint32_t cmd_and_length;
	union 
    {
		uint32_t seg_config;
		struct 
        {
			uint8_t status;             /* Descriptor status */
			uint8_t hdr_len;            /* Header length */
			uint16_t mss;               /* Maximum segment size */
		} seg_fields;
	};
};
struct e1000e_offload_data_descriptor 
{
	uintptr_t buffer_addr;              /* Address of the descriptor's buffer address */
	union 
    {
		uint32_t data_lower;
		struct
        {
			uint16_t length;            /* Data buffer length */
			uint8_t typ_len_ext;
			uint8_t cmd;
		} flags;
	};
	union 
    {
		uint32_t data_upper;
		struct 
        {
			uint8_t status;             /* Descriptor status */
			uint8_t popts;              /* Packet Options */
			uint16_t special;
		} fields;
	};
};
constexpr size_t max_single_tx_buffer = 16288UZ;
constexpr size_t max_single_rx_buffer = 16384UZ;
template<typename T>
struct e1000e_ring
{
    std::allocator<T> alloc;
    T* descriptors;
    T* max_descriptor;
    uint32_t head_descriptor;
    uint32_t tail_descriptor;
    constexpr T& tail() { return descriptors[tail_descriptor]; }
    constexpr T& head() { return descriptors[head_descriptor]; }
    constexpr size_t count() const { return static_cast<size_t>(max_descriptor - descriptors); }
    constexpr e1000e_ring(size_t count_factor) : alloc(), descriptors(alloc.allocate(count_factor * e1000_rxtxdesclen_base)), max_descriptor(descriptors + count_factor * e1000_rxtxdesclen_base) {}
    constexpr ~e1000e_ring() { if(descriptors && max_descriptor) alloc.deallocate(descriptors, count()); }
};
class e1000e : public net_device
{
    bool __has_init;
    addr_t __mmio_region;
    pci_config_space* __pcie_e1000e_controller;
    simple_io_port<uint32_t> __io_addr_port;
    simple_io_port<uint32_t> __io_data_port;
    bool __mdio_await(mdic& mdic_reg);
protected:
    e1000e_ring<e1000e_receive_descriptor> rx_ring;
    e1000e_ring<e1000e_transmit_descriptor> tx_ring;
    virtual bool configure_rx(dev_status& st);
    virtual bool configure_tx(dev_status& st);
    virtual bool configure_mac_phy(dev_status& st);
    bool configure_interrupts(dev_status& st);
    void read_status(dev_status& st);
    bool read_io(int reg_id, uint32_t& r_out);
    bool write_io(int reg_id, uint32_t const& w_in);
    void read_dma(int reg_id, uint32_t& r_out);
    void write_dma(int reg_id, uint32_t const& w_in);
    uint16_t read_phy(int phy_reg);
    void write_phy(int phy_reg, uint16_t data);
    uint16_t read_eeprom(uint16_t eep_addr);
    __isrcall void on_interrupt();
public:
    e1000e(pci_config_space* device, size_t descriptor_count_factor = 32UZ);
    virtual ~e1000e();
    virtual bool initialize() override;
    virtual void enable_transmit() override;
    virtual void enable_receive() override;
    virtual void disable_transmit() override;
    virtual void disable_receive() override;
    virtual int poll_tx(netstack_buffer& buff);
    virtual int poll_rx();
};
#endif