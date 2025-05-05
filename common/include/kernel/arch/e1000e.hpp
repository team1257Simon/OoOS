#ifndef __PCIE_GIGABIT_ETHERNET
#define __PCIE_GIGABIT_ETHERNET
#include "arch/pci.hpp"
struct e1000e_eeprom_flash_control_register
{
    bool nvm_clock_bit                          : 1;
    bool nvm_chip_select                        : 1;
    bool nvm_data_write_line                    : 1;
    volatile bool nvm_data_read_line            : 1;
    uint8_t flash_write_enable_control          : 2;
    bool nvm_access_request                     : 1;
    volatile bool nvm_access_granted            : 1;
    const volatile bool nvm_present             : 1;
    const volatile bool nvm_auto_read_done      : 1;
    bool                                        : 1;
    const uint8_t nvm_size                      : 4;
    uint8_t nvm_address_size                    : 2;
    bool                                        : 3;
    bool autonomous_flash_update_enable         : 1;
    bool                                        : 1;
    bool sector_1_valid                         : 1;
    const volatile bool nvm_type                : 1;  // 0 = eeprom, 1 = flash
    uint8_t                                     : 8;
} attribute(packed, aligned(4));
struct e1000e_eeprom_read_register
{
    volatile bool read_start    : 1;
    volatile bool read_done     : 1;
    uint16_t read_address       : 14;
    volatile uint16_t read_data : 16;
} attribute(packed, aligned(4));
struct e1000e_flash_access_register
{
    volatile bool clock                 : 1;
    volatile bool chip_select           : 1;
    volatile bool data_write            : 1;
    volatile bool data_read             : 1;
    volatile bool access_request        : 1;
    volatile bool access_granted        : 1;
    volatile bool erase_transaction     : 1;
    volatile bool write_transaction     : 1;
    volatile bool write_done            : 1;
    volatile int                        : 21;
    volatile bool flash_busy            : 1;
    volatile bool flash_erase_command   : 1;
} attribute(packed, aligned(4));
struct e1000e_mdi_control_register
{
    volatile uint16_t data                  : 16;
    volatile uint8_t phy_register_address   : 5;
    volatile uint8_t phy_address            : 5;
    volatile uint8_t operation              : 2; // 0b01 = read, 0b10 = write
    volatile bool ready                     : 1;
    volatile bool interrupt_enable          : 1;
    volatile bool error                     : 1;
    volatile bool                           : 1;
} attribute(packed, aligned(4));
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
            } __pack csum_ip;
        } __pack hi_dword;
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
            } __pack csum_ip;
        } __pack hi_dword;
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
		__le32 data;
		struct 
        {
			__le16 length;          /* Data buffer length */
			byte cso;	            /* Checksum offset */
			byte cmd;	            /* Descriptor control */
		} flags;
	} lower;
	union 
    {
		__le32 data;
		struct 
        {
			byte status;            /* Descriptor status */
			byte css;	            /* Checksum start */
			__le16 special;
		} fields;
	} upper;
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
	} lower_setup;
	union 
    {
		__le32 tcp_config;
		struct 
        {
			byte tucss;         /* TCP checksum start */
			byte tucso;         /* TCP checksum offset */
			__le16 tucse;       /* TCP checksum end */
		} tcp_fields;
	} upper_setup;
	__le32 cmd_and_length;
	union 
    {
		__le32 data;
		struct 
        {
			byte status;        /* Descriptor status */
			byte hdr_len;       /* Header length */
			__le16 mss;         /* Maximum segment size */
		} fields;
	} tcp_seg_setup;
};
struct e1000e_offload_data_descriptor 
{
	uintptr_t buffer_addr;   /* Address of the descriptor's buffer address */
	union 
    {
		__le32 data;
		struct 
        {
			__le16 length;    /* Data buffer length */
			byte typ_len_ext;
			byte cmd;
		} flags;
	} lower;
	union 
    {
		__le32 data;
		struct 
        {
			byte status;     /* Descriptor status */
			byte popts;      /* Packet Options */
			__le16 special;
		} fields;
	} upper;
};
#endif