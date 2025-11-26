/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2018 Intel Corporation. */
/* NOTE: converted to C++ constant expressions but otherwise equivalent code */
#ifndef __E1000E_DEFS
#define __E1000E_DEFS
#include "kernel_defs.h"
constexpr int e1000_max_phy_addr	= 8;
/* IGP01E1000 Specific Registers */
constexpr int igp01e1000_phy_port_config	= 0x10;	/* Port Config */
constexpr int igp01e1000_phy_port_status	= 0x11;	/* Status */
constexpr int igp01e1000_phy_port_ctrl		= 0x12;	/* Control */
constexpr int igp01e1000_phy_link_health	= 0x13;	/* PHY Link Health */
constexpr int igp02e1000_phy_power_mgmt		= 0x19;	/* Power Management */
constexpr int igp01e1000_phy_page_select	= 0x1F;	/* Page Select */
constexpr int bm_phy_page_select			= 22;			/* Page Select for BM */
constexpr int igp_page_shift				= 5;
constexpr int phy_page_shift				= 5;
constexpr int phy_reg_mask					= 0x1F;
constexpr int max_phy_reg_address			= 0x1F;	/* 5 bit address bus (0-0x1F) */
/* BM/HV Specific Registers */
constexpr int bm_port_ctrl_page				= 769;
constexpr int bm_wuc_page					= 800;
constexpr int bm_wuc_address_opcode			= 0x11;
constexpr int bm_wuc_data_opcode			= 0x12;
constexpr int bm_wuc_enable_page			= bm_port_ctrl_page;
constexpr int bm_wuc_enable_reg				= 17;
constexpr int bm_wuc_enable_bit				= (1 << 2);
constexpr int bm_wuc_host_wu_bit			= (1 << 4);
constexpr int bm_wuc_me_wu_bit				= (1 << 5);
constexpr int phy_upper_shift				= 21;
constexpr int bm_phy_reg(int page, int reg) { return (reg & max_phy_reg_address) | ((page & 0xFFFF) << phy_page_shift) | ((reg & ~max_phy_reg_address) << (phy_upper_shift - phy_page_shift)); }
constexpr uint16_t bm_phy_reg_page(int offset) { return static_cast<uint16_t>((offset >> phy_page_shift) & 0xFFFF); }
constexpr uint16_t bm_phy_reg_num(int offset) { return static_cast<uint16_t>((offset & max_phy_reg_address) | ((offset >> (phy_upper_shift - phy_page_shift)) & ~max_phy_reg_address)); }
constexpr int hv_intc_fc_page_start			= 768;
constexpr int i82578_addr_reg				= 29;
constexpr int i82577_addr_reg				= 16;
constexpr int i82577_cfg_reg				= 22;
constexpr int i82577_cfg_assert_crs_on_tx	= (1 << 15);
constexpr int i82577_cfg_enable_downshift	= (3U	<< 10); /* auto downshift */
constexpr int i82577_ctrl_reg				= 23;
/* 82577 specific PHY registers */
constexpr int i82577_phy_ctrl_2				= 18;
constexpr int i82577_phy_lbk_ctrl			= 19;
constexpr int i82577_phy_status_2			= 26;
constexpr int i82577_phy_diag_status		= 31;
/* I82577 PHY Status 2 */
constexpr int i82577_phy_status2_rev_polarity	= 0x0400;
constexpr int i82577_phy_status2_mdix			= 0x0800;
constexpr int i82577_phy_status2_speed_mask		= 0x0300;
constexpr int i82577_phy_status2_speed_1000mbps	= 0x0200;
/* I82577 PHY Control 2 */
constexpr int i82577_phy_ctrl2_manual_mdix		= 0x0200;
constexpr int i82577_phy_ctrl2_auto_mdi_mdix	= 0x0400;
constexpr int i82577_phy_ctrl2_mdix_cfg_mask	= 0x0600;
/* I82577 PHY Diagnostics Status */
constexpr int i82577_dstatus_cable_length		= 0x03FC;
constexpr int i82577_dstatus_cable_length_shift	= 2;
/* BM PHY Copper Specific Control 1 */
constexpr int bm_cs_ctrl1						= 16;
/* BM PHY Copper Specific Status */
constexpr int bm_cs_status						= 17;
constexpr int bm_cs_status_link_up				= 0x0400;
constexpr int bm_cs_status_resolved				= 0x0800;
constexpr int bm_cs_status_speed_mask			= 0xC000;
constexpr int bm_cs_status_speed_1000			= 0x8000;
/* 82577 Mobile Phy Status Register */
constexpr int hv_m_status						= 26;
constexpr int hv_m_status_autoneg_complete		= 0x1000;
constexpr int hv_m_status_speed_mask			= 0x0300;
constexpr int hv_m_status_speed_1000			= 0x0200;
constexpr int hv_m_status_speed_100				= 0x0100;
constexpr int hv_m_status_link_up				= 0x0040;
constexpr int igp01e1000_phy_pcs_init_reg		= 0x00B4;
constexpr int igp01e1000_phy_polarity_mask		= 0x0078;
constexpr int igp01e1000_pscr_auto_mdix			= 0x1000;
constexpr int igp01e1000_pscr_force_mdi_mdix	= 0x2000;	/* 0=MDI, 1=MDIX */
constexpr int igp01e1000_pscfr_smart_speed		= 0x0080;
constexpr int igp02e1000_pm_spd					= 0x0001;	/* Smart Power Down */
constexpr int igp02e1000_pm_d0_lplu				= 0x0002;	/* For D0a states */
constexpr int igp02e1000_pm_d3_lplu				= 0x0004;	/* For all other states */
constexpr int igp01e1000_plhr_ss_downgrade		= 0x8000;
constexpr int igp01e1000_pssr_polarity_reversed	= 0x0002;
constexpr int igp01e1000_pssr_mdix				= 0x0800;
constexpr int igp01e1000_pssr_speed_mask		= 0xC000;
constexpr int igp01e1000_pssr_speed_1000mbps	= 0xC000;
constexpr int igp02e1000_phy_channel_num		= 4;
constexpr int igp02e1000_phy_agc_a				= 0x11B1;
constexpr int igp02e1000_phy_agc_b				= 0x12B1;
constexpr int igp02e1000_phy_agc_c				= 0x14B1;
constexpr int igp02e1000_phy_agc_d				= 0x18B1;
constexpr int igp02e1000_agc_length_shift		= 9;		/* Coarse=15:13, Fine=12:9 */
constexpr int igp02e1000_agc_length_mask		= 0x7F;
constexpr int igp02e1000_agc_range				= 15;
constexpr int e1000_cable_length_undefined		= 0xFF;
constexpr int e1000_kmrnctrlsta_offset			= 0x001F0000;
constexpr int e1000_kmrnctrlsta_offset_shift	= 16;
constexpr int e1000_kmrnctrlsta_ren				= 0x00200000;
constexpr int e1000_kmrnctrlsta_ctrl_offset		= 0x1;		/* Kumeran Control */
constexpr int e1000_kmrnctrlsta_diag_offset		= 0x3;		/* Kumeran Diagnostic */
constexpr int e1000_kmrnctrlsta_timeouts		= 0x4;		/* Kumeran Timeouts */
constexpr int e1000_kmrnctrlsta_inband_param	= 0x9;		/* Kumeran InBand Parameters */
constexpr int e1000_kmrnctrlsta_ibist_disable	= 0x0200;	/* Kumeran IBIST Disable */
constexpr int e1000_kmrnctrlsta_diag_nelpbk		= 0x1000;	/* Nearend Loopback mode */
constexpr int e1000_kmrnctrlsta_k1_config		= 0x7;
constexpr int e1000_kmrnctrlsta_k1_enable		= 0x0002;	/* enable K1 */
constexpr int e1000_kmrnctrlsta_hd_ctrl			= 0x10;		/* Kumeran HD Control */
constexpr int ife_phy_extended_status_control	= 0x10;
constexpr int ife_phy_special_control			= 0x11;		/* 100BaseTx PHY Special Ctrl */
constexpr int ife_phy_special_control_led		= 0x1B;		/* PHY Special and LED Ctrl */
constexpr int ife_phy_mdix_control				= 0x1C;		/* MDI/MDI-X Control */
/* IFE PHY Extended Status Control */
constexpr int ife_pesc_polarity_reversed		= 0x0100;
/* IFE PHY Special Control */
constexpr int ife_psc_auto_polarity_disable		= 0x0010;
constexpr int ife_psc_force_polarity			= 0x0020;
/* IFE PHY Special Control and LED Control */
constexpr int ife_pscl_probe_mode		= 0x0020;
constexpr int ife_pscl_probe_leds_off	= 0x0006;	/* Force LEDs 0 and 2 off */
constexpr int ife_pscl_probe_leds_on	= 0x0007;	/* Force LEDs 0 and 2 on */
/* IFE PHY MDIX Control */
constexpr int ife_pmc_mdix_status		= 0x0020;	/* 1=MDI-X, 0=MDI */
constexpr int ife_pmc_force_mdix		= 0x0040;	/* 1=force MDI-X, 0=force MDI */
constexpr int ife_pmc_auto_mdix			= 0x0080;	/* 1=enable auto, 0=disable */
constexpr int e1000_ctrl				= 0x00000;	/* Device Control - RW */
constexpr int e1000_status				= 0x00008;	/* Device Status - RO */
constexpr int e1000_eecd				= 0x00010;	/* EEPROM/Flash Control - RW */
constexpr int e1000_eerd				= 0x00014;	/* EEPROM Read - RW */
constexpr int e1000_ctrl_ext			= 0x00018;	/* Extended Device Control - RW */
constexpr int e1000_fla					= 0x0001C;	/* Flash Access - RW */
constexpr int e1000_mdic				= 0x00020;	/* MDI Control - RW */
constexpr int e1000_sctl				= 0x00024;	/* SerDes Control - RW */
constexpr int e1000_fcal				= 0x00028;	/* Flow Control Address Low - RW */
constexpr int e1000_fcah				= 0x0002C;	/* Flow Control Address High -RW */
constexpr int e1000_fext				= 0x0002C;	/* Future Extended - RW */
constexpr int e1000_fextnvm				= 0x00028;	/* Future Extended NVM - RW */
constexpr int e1000_fextnvm3			= 0x0003C;	/* Future Extended NVM 3 - RW */
constexpr int e1000_fextnvm4			= 0x00024;	/* Future Extended NVM 4 - RW */
constexpr int e1000_fextnvm5			= 0x00014;	/* Future Extended NVM 5 - RW */
constexpr int e1000_fextnvm6			= 0x00010;	/* Future Extended NVM 6 - RW */
constexpr int e1000_fextnvm7			= 0x000E4;	/* Future Extended NVM 7 - RW */
constexpr int e1000_fextnvm8			= 0x5BB0;	/* Future Extended NVM 8 - RW */
constexpr int e1000_fextnvm9			= 0x5BB4;	/* Future Extended NVM 9 - RW */
constexpr int e1000_fextnvm11			= 0x5BBC;	/* Future Extended NVM 11 - RW */
constexpr int e1000_fextnvm12			= 0x5BC0;	/* Future Extended NVM 12 - RW */
constexpr int e1000_pcieanacfg			= 0x00F18;	/* PCIE Analog Config */
constexpr int e1000_dpgfr				= 0x00FAC;	/* Dynamic Power Gate Force Control Register */
constexpr int e1000_fct					= 0x00030;	/* Flow Control Type - RW */
constexpr int e1000_vet					= 0x00038;	/* VLAN Ether Type - RW */
constexpr int e1000_icr					= 0x000C0;	/* Interrupt Cause Read - R/clr */
constexpr int e1000_itr					= 0x000C4;	/* Interrupt Throttling Rate - RW */
constexpr int e1000_ics					= 0x000C8;	/* Interrupt Cause Set - WO */
constexpr int e1000_ims					= 0x000D0;	/* Interrupt Mask Set - RW */
constexpr int e1000_imc					= 0x000D8;	/* Interrupt Mask Clear - WO */
constexpr int e1000_iam					= 0x000E0;	/* Interrupt Acknowledge Auto Mask */
constexpr int e1000_ivar				= 0x000E4;	/* Interrupt Vector Allocation Register - RW */
constexpr int e1000_svcr				= 0x000F0;
constexpr int e1000_svt					= 0x000F4;
constexpr int e1000_lpic				= 0x000FC;	/* Low Power IDLE control */
constexpr int e1000_rctl				= 0x00100;	/* Rx Control - RW */
constexpr int e1000_fcttv				= 0x00170;	/* Flow Control Transmit Timer Value - RW */
constexpr int e1000_txcw				= 0x00178;	/* Tx Configuration Word - RW */
constexpr int e1000_rxcw				= 0x00180;	/* Rx Configuration Word - RO */
constexpr int e1000_pba_ecc				= 0x01100;	/* PBA ECC Register */
constexpr int e1000_tctl				= 0x00400;	/* Tx Control - RW */
constexpr int e1000_tctl_ext			= 0x00404;	/* Extended Tx Control - RW */
constexpr int e1000_tipg				= 0x00410;	/* Tx Inter-packet gap -RW */
constexpr int e1000_ait					= 0x00458;	/* Adaptive Interframe Spacing Throttle - RW */
constexpr int e1000_ledctl				= 0x00E00;	/* LED Control - RW */
constexpr int e1000_extcnf_ctrl			= 0x00F00;	/* Extended Configuration Control */
constexpr int e1000_extcnf_size			= 0x00F08;	/* Extended Configuration Size */
constexpr int e1000_phy_ctrl			= 0x00F10;	/* PHY Control Register in CSR */
constexpr int e1000_poemb				= 0x00F10;	/* PHY OEM Bits */
constexpr int e1000_pba					= 0x01000;	/* Packet Buffer Allocation - RW */
constexpr int e1000_pbs					= 0x01008;	/* Packet Buffer Size */
constexpr int e1000_pbeccsts			= 0x0100C;	/* Packet Buffer ECC Status - RW */
constexpr int e1000_iosfpc				= 0x00F28;	/* TX corrupted data	*/
constexpr int e1000_eemngctl			= 0x01010;	/* MNG EEprom Control */
constexpr int e1000_eewr				= 0x0102C;	/* EEPROM Write Register - RW */
constexpr int e1000_flop				= 0x0103C;	/* FLASH Opcode Register */
constexpr int e1000_ert					= 0x02008;	/* Early Rx Threshold - RW */
constexpr int e1000_fcrtl				= 0x02160;	/* Flow Control Receive Threshold Low - RW */
constexpr int e1000_fcrth				= 0x02168;	/* Flow Control Receive Threshold High - RW */
constexpr int e1000_psrctl				= 0x02170;	/* Packet Split Receive Control - RW */
constexpr int e1000_rdfh				= 0x02410;	/* Rx Data FIFO Head - RW */
constexpr int e1000_rdft				= 0x02418;	/* Rx Data FIFO Tail - RW */
constexpr int e1000_rdfhs				= 0x02420;	/* Rx Data FIFO Head Saved - RW */
constexpr int e1000_rdfts				= 0x02428;	/* Rx Data FIFO Tail Saved - RW */
constexpr int e1000_rdfpc				= 0x02430;	/* Rx Data FIFO Packet Count - RW */
/* Split and Replication Rx Control - RW */
constexpr int e1000_rdtr				= 0x02820;	/* Rx Delay Timer - RW */
constexpr int e1000_radv				= 0x0282C;	/* Rx Interrupt Absolute Delay Timer - RW */
/*
 * Note: "n" is the queue number of the register to be written to.
 *
 * Example usage:
 * e1000_rdbal(current_rx_queue)
 */
constexpr int e1000_rdbal(int n) { return (n < 4 ? (0x02800 + (n * 0x100)) : (0x0C000 + (n * 0x40))); }
constexpr int e1000_rdbah(int n) { return (n < 4 ? (0x02804 + (n * 0x100)) : (0x0C004 + (n * 0x40))); }
constexpr int e1000_rdlen(int n) { return (n < 4 ? (0x02808 + (n * 0x100)) : (0x0C008 + (n * 0x40))); }
constexpr int e1000_rdh(int n) { return (n < 4 ? (0x02810 + (n * 0x100)) : (0x0C010 + (n * 0x40))); }
constexpr int e1000_rdt(int n) { return (n < 4 ? (0x02818 + (n * 0x100)) : (0x0C018 + (n * 0x40))); }
constexpr int e1000_rxdctl(int n) { return (n < 4 ? (0x02828 + (n * 0x100)) : (0x0C028 + (n * 0x40))); }
constexpr int e1000_tdbal(int n) { return (n < 4 ? (0x03800 + (n * 0x100)) : (0x0E000 + (n * 0x40))); }
constexpr int e1000_tdbah(int n) { return (n < 4 ? (0x03804 + (n * 0x100)) : (0x0E004 + (n * 0x40))); }
constexpr int e1000_tdlen(int n) { return (n < 4 ? (0x03808 + (n * 0x100)) : (0x0E008 + (n * 0x40))); }
constexpr int e1000_tdh(int n) { return (n < 4 ? (0x03810 + (n * 0x100)) : (0x0E010 + (n * 0x40))); }
constexpr int e1000_tdt(int n) { return (n < 4 ? (0x03818 + (n * 0x100)) : (0x0E018 + (n * 0x40))); }
constexpr int e1000_txdctl(int n) { return (n < 4 ? (0x03828 + (n * 0x100)) : (0x0E028 + (n * 0x40))); }
constexpr int e1000_tarc(int n) { return (0x03840 + (n * 0x100)); }
constexpr int e1000_ral(int i) { return ((i <= 15) ? (0x05400 + (i * 8)) : (0x054E0 + ((i - 16) * 8))); }
constexpr int e1000_rah(int i) { return ((i <= 15) ? (0x05404 + (i * 8)) : (0x054E4 + ((i - 16) * 8))); }
constexpr int e1000_shral(int i) { return 0x05438 + (i * 8); }
constexpr int e1000_shrah(int i) { return 0x0543C + (i * 8); }
constexpr int e1000_kabgtxd		= 0x03004;		/* AFE Band Gap Transmit Ref Data */
constexpr int e1000_tdfh		= 0x03410;		/* Tx Data FIFO Head - RW */
constexpr int e1000_tdft		= 0x03418;		/* Tx Data FIFO Tail - RW */
constexpr int e1000_tdfhs		= 0x03420;		/* Tx Data FIFO Head Saved - RW */
constexpr int e1000_tdfts		= 0x03428;		/* Tx Data FIFO Tail Saved - RW */
constexpr int e1000_tdfpc		= 0x03430;		/* Tx Data FIFO Packet Count - RW */
constexpr int e1000_tidv		= 0x03820;		/* Tx Interrupt Delay Value - RW */
constexpr int e1000_tadv		= 0x0382C;		/* Tx Interrupt Absolute Delay Val - RW */
constexpr int e1000_crcerrs		= 0x04000;		/* CRC Error Count - R/clr */
constexpr int e1000_algnerrc	= 0x04004;		/* Alignment Error Count - R/clr */
constexpr int e1000_symerrs		= 0x04008;		/* Symbol Error Count - R/clr */
constexpr int e1000_rxerrc		= 0x0400C;		/* Receive Error Count - R/clr */
constexpr int e1000_mpc		= 0x04010;		/* Missed Packet Count - R/clr */
constexpr int e1000_scc		= 0x04014;		/* Single Collision Count - R/clr */
constexpr int e1000_ecol	= 0x04018;		/* Excessive Collision Count - R/clr */
constexpr int e1000_mcc		= 0x0401C;		/* Multiple Collision Count - R/clr */
constexpr int e1000_latecol	= 0x04020;		/* Late Collision Count - R/clr */
constexpr int e1000_colc	= 0x04028;		/* Collision Count - R/clr */
constexpr int e1000_dc		= 0x04030;		/* Defer Count - R/clr */
constexpr int e1000_tncrs	= 0x04034;		/* Tx-No CRS - R/clr */
constexpr int e1000_sec		= 0x04038;		/* Sequence Error Count - R/clr */
constexpr int e1000_cexterr	= 0x0403C;		/* Carrier Extension Error Count - R/clr */
constexpr int e1000_rlec	= 0x04040;		/* Receive Length Error Count - R/clr */
constexpr int e1000_xonrxc	= 0x04048;		/* XON Rx Count - R/clr */
constexpr int e1000_xontxc	= 0x0404C;		/* XON Tx Count - R/clr */
constexpr int e1000_xoffrxc	= 0x04050;		/* XOFF Rx Count - R/clr */
constexpr int e1000_xofftxc	= 0x04054;		/* XOFF Tx Count - R/clr */
constexpr int e1000_fcruc	= 0x04058;		/* Flow Control Rx Unsupported Count- R/clr */
constexpr int e1000_prc64	= 0x0405C;		/* Packets Rx (64 bytes) - R/clr */
constexpr int e1000_prc127	= 0x04060;		/* Packets Rx (65-127 bytes) - R/clr */
constexpr int e1000_prc255	= 0x04064;		/* Packets Rx (128-255 bytes) - R/clr */
constexpr int e1000_prc511	= 0x04068;		/* Packets Rx (255-511 bytes) - R/clr */
constexpr int e1000_prc1023	= 0x0406C;		/* Packets Rx (512-1023 bytes) - R/clr */
constexpr int e1000_prc1522	= 0x04070;		/* Packets Rx (1024-1522 bytes) - R/clr */
constexpr int e1000_gprc	= 0x04074;		/* Good Packets Rx Count - R/clr */
constexpr int e1000_bprc	= 0x04078;		/* Broadcast Packets Rx Count - R/clr */
constexpr int e1000_mprc	= 0x0407C;		/* Multicast Packets Rx Count - R/clr */
constexpr int e1000_gptc	= 0x04080;		/* Good Packets Tx Count - R/clr */
constexpr int e1000_gorcl	= 0x04088;		/* Good Octets Rx Count Low - R/clr */
constexpr int e1000_gorch	= 0x0408C;		/* Good Octets Rx Count High - R/clr */
constexpr int e1000_gotcl	= 0x04090;		/* Good Octets Tx Count Low - R/clr */
constexpr int e1000_gotch	= 0x04094;		/* Good Octets Tx Count High - R/clr */
constexpr int e1000_rnbc	= 0x040A0;		/* Rx No Buffers Count - R/clr */
constexpr int e1000_ruc		= 0x040A4;		/* Rx Undersize Count - R/clr */
constexpr int e1000_rfc		= 0x040A8;		/* Rx Fragment Count - R/clr */
constexpr int e1000_roc		= 0x040AC;		/* Rx Oversize Count - R/clr */
constexpr int e1000_rjc		= 0x040B0;		/* Rx Jabber Count - R/clr */
constexpr int e1000_mgtprc	= 0x040B4;		/* Management Packets Rx Count - R/clr */
constexpr int e1000_mgtpdc	= 0x040B8;		/* Management Packets Dropped Count - R/clr */
constexpr int e1000_mgtptc	= 0x040BC;		/* Management Packets Tx Count - R/clr */
constexpr int e1000_torl	= 0x040C0;		/* Total Octets Rx Low - R/clr */
constexpr int e1000_torh	= 0x040C4;		/* Total Octets Rx High - R/clr */
constexpr int e1000_totl	= 0x040C8;		/* Total Octets Tx Low - R/clr */
constexpr int e1000_toth	= 0x040CC;		/* Total Octets Tx High - R/clr */
constexpr int e1000_tpr		= 0x040D0;		/* Total Packets Rx - R/clr */
constexpr int e1000_tpt		= 0x040D4;		/* Total Packets Tx - R/clr */
constexpr int e1000_ptc64	= 0x040D8;		/* Packets Tx (64 bytes) - R/clr */
constexpr int e1000_ptc127	= 0x040DC;		/* Packets Tx (65-127 bytes) - R/clr */
constexpr int e1000_ptc255	= 0x040E0;		/* Packets Tx (128-255 bytes) - R/clr */
constexpr int e1000_ptc511	= 0x040E4;		/* Packets Tx (256-511 bytes) - R/clr */
constexpr int e1000_ptc1023	= 0x040E8;		/* Packets Tx (512-1023 bytes) - R/clr */
constexpr int e1000_ptc1522	= 0x040EC;		/* Packets Tx (1024-1522 Bytes) - R/clr */
constexpr int e1000_mptc	= 0x040F0;		/* Multicast Packets Tx Count - R/clr */
constexpr int e1000_bptc	= 0x040F4;		/* Broadcast Packets Tx Count - R/clr */
constexpr int e1000_tsctc	= 0x040F8;		/* TCP Segmentation Context Tx - R/clr */
constexpr int e1000_tsctfc	= 0x040FC;		/* TCP Segmentation Context Tx Fail - R/clr */
constexpr int e1000_iac		= 0x04100;		/* Interrupt Assertion Count */
constexpr int e1000_icrxptc	= 0x04104;		/* Interrupt Cause Rx Pkt Timer Expire Count */
constexpr int e1000_icrxatc	= 0x04108;		/* Interrupt Cause Rx Abs Timer Expire Count */
constexpr int e1000_ictxptc	= 0x0410C;		/* Interrupt Cause Tx Pkt Timer Expire Count */
constexpr int e1000_ictxatc	= 0x04110;		/* Interrupt Cause Tx Abs Timer Expire Count */
constexpr int e1000_ictxqec	= 0x04118;		/* Interrupt Cause Tx Queue Empty Count */
constexpr int e1000_ictxqmtc	= 0x0411C;	/* Interrupt Cause Tx Queue Min Thresh Count */
constexpr int e1000_icrxdmtc	= 0x04120;	/* Interrupt Cause Rx Desc Min Thresh Count */
constexpr int e1000_icrxoc		= 0x04124;	/* Interrupt Cause Receiver Overrun Count */
constexpr int e1000_crc_offset	= 0x05F50;	/* CRC Offset register */
constexpr int e1000_pcs_lctl	= 0x04208;	/* PCS Link Control - RW */
constexpr int e1000_pcs_lstat	= 0x0420C;	/* PCS Link Status - RO */
constexpr int e1000_pcs_anadv	= 0x04218;	/* AN advertisement - RW */
constexpr int e1000_pcs_lpab	= 0x0421C;	/* Link Partner Ability - RW */
constexpr int e1000_rxcsum		= 0x05000;	/* Rx Checksum Control - RW */
constexpr int e1000_rfctl		= 0x05008;	/* Receive Filter Control */
constexpr int e1000_mta			= 0x05200;	/* Multicast Table Array - RW Array */
constexpr int e1000_ra			= 0x05400;	/* Receive Address - RW Array */
constexpr int e1000_vfta		= 0x05600;	/* VLAN Filter Table Array - RW Array */
constexpr int e1000_wuc			= 0x05800;	/* Wakeup Control - RW */
constexpr int e1000_wufc		= 0x05808;	/* Wakeup Filter Control - RW */
constexpr int e1000_wus			= 0x05810;	/* Wakeup Status - RO */
constexpr int e1000_manc		= 0x05820;	/* Management Control - RW */
constexpr int e1000_fflt		= 0x05F00;	/* Flexible Filter Length Table - RW Array */
constexpr int e1000_host_if		= 0x08800;	/* Host Interface */
constexpr int e1000_kmrnctrlsta	= 0x00034;	/* MAC-PHY interface - RW */
constexpr int e1000_manc2h		= 0x05860;	/* Management Control To Host - RW */
/* Management Decision Filters */
constexpr int e1000_mdef(int n) { return 0x05890 + (4 * n); }
constexpr int e1000_sw_fw_sync	= 0x05B5C;	/* SW-FW Synchronization - RW */
constexpr int e1000_gcr			= 0x05B00;	/* PCI-Ex Control */
constexpr int e1000_gcr2		= 0x05B64;	/* PCI-Ex Control #2 */
constexpr int e1000_factps		= 0x05B30;	/* Function Active and Power State to MNG */
constexpr int e1000_swsm		= 0x05B50;	/* SW Semaphore */
constexpr int e1000_fwsm		= 0x05B54;	/* FW Semaphore */
constexpr int e1000_exfwsm		= 0x05B58;	/* Extended FW Semaphore */
/* Driver-only SW semaphore (not used by BOOT agents) */
constexpr int e1000_swsm2		= 0x05B58;
constexpr int e1000_fflt_dbg	= 0x05F04;	/* Debug Register */
constexpr int e1000_hicr		= 0x08F00;	/* Host Interface Control */
/* RSS registers */
constexpr int e1000_mrqc		= 0x05818;						/* Multiple Receive Control - RW */
constexpr int e1000_reta(int i) { return 0x05C00 + (i * 4); }	/* Redirection Table - RW */
constexpr int e1000_rssrk(int i) { return 0x05C80 + (i * 4); }	/* RSS Random Key - RW */
constexpr int e1000_tsyncrxctl	= 0x0B620;					/* Rx Time Sync Control register - RW */
constexpr int e1000_tsynctxctl	= 0x0B614;					/* Tx Time Sync Control register - RW */
constexpr int e1000_rxstmpl		= 0x0B624;					/* Rx timestamp Low - RO */
constexpr int e1000_rxstmph		= 0x0B628;					/* Rx timestamp High - RO */
constexpr int e1000_txstmpl		= 0x0B618;					/* Tx timestamp value Low - RO */
constexpr int e1000_txstmph		= 0x0B61C;					/* Tx timestamp value High - RO */
constexpr int e1000_systiml		= 0x0B600;					/* System time register Low - RO */
constexpr int e1000_systimh		= 0x0B604;					/* System time register High - RO */
constexpr int e1000_timinca		= 0x0B608;					/* Increment attributes register - RW */
constexpr int e1000_sysstmpl	= 0x0B648;					/* HH Timesync system stamp low register */
constexpr int e1000_sysstmph	= 0x0B64C;					/* HH Timesync system stamp hi register */
constexpr int e1000_pltstmpl	= 0x0B640;					/* HH Timesync platform stamp low register */
constexpr int e1000_pltstmph	= 0x0B644;					/* HH Timesync platform stamp hi register */
constexpr int e1000_rxmtrl		= 0x0B634;					/* Time sync Rx EtherType and Msg Type - RW */
constexpr int e1000_rxudp		= 0x0B638;					/* Time Sync Rx UDP Port - RW */
constexpr int phy_reg(int page, int reg) { return (page << phy_page_shift) | (reg & max_phy_reg_address); }
/* PHY registers */
constexpr int i82579_dft_ctrl	= phy_reg(769, 20);
/* Rx / Tx descriptor rings must be sized in multiples of 8 */
constexpr size_t e1000_rxtxdesclen_base				= 8;
constexpr int e1000_stats_min						= e1000_crcerrs;
constexpr int e1000_stats_max						= e1000_icrxoc + 4;
constexpr int e1000_dev_id_82571eb_copper			= 0x105E;
constexpr int e1000_dev_id_82571eb_fiber			= 0x105F;
constexpr int e1000_dev_id_82571eb_serdes			= 0x1060;
constexpr int e1000_dev_id_82571eb_quad_copper		= 0x10A4;
constexpr int e1000_dev_id_82571pt_quad_copper		= 0x10D5;
constexpr int e1000_dev_id_82571eb_quad_fiber		= 0x10A5;
constexpr int e1000_dev_id_82571eb_quad_copper_lp	= 0x10BC;
constexpr int e1000_dev_id_82571eb_serdes_dual		= 0x10D9;
constexpr int e1000_dev_id_82571eb_serdes_quad		= 0x10DA;
constexpr int e1000_dev_id_82572ei_copper			= 0x107D;
constexpr int e1000_dev_id_82572ei_fiber			= 0x107E;
constexpr int e1000_dev_id_82572ei_serdes			= 0x107F;
constexpr int e1000_dev_id_82572ei					= 0x10B9;
constexpr int e1000_dev_id_82573e					= 0x108B;
constexpr int e1000_dev_id_82573e_iamt				= 0x108C;
constexpr int e1000_dev_id_82573l					= 0x109A;
constexpr int e1000_dev_id_82574l					= 0x10D3;
constexpr int e1000_dev_id_82574la					= 0x10F6;
constexpr int e1000_dev_id_82583v					= 0x150C;
constexpr int e1000_dev_id_80003es2lan_copper_dpt	= 0x1096;
constexpr int e1000_dev_id_80003es2lan_serdes_dpt	= 0x1098;
constexpr int e1000_dev_id_80003es2lan_copper_spt	= 0x10BA;
constexpr int e1000_dev_id_80003es2lan_serdes_spt	= 0x10BB;
constexpr int e1000_dev_id_ich8_82567v_3			= 0x1501;
constexpr int e1000_dev_id_ich8_igp_m_amt			= 0x1049;
constexpr int e1000_dev_id_ich8_igp_amt				= 0x104A;
constexpr int e1000_dev_id_ich8_igp_c				= 0x104B;
constexpr int e1000_dev_id_ich8_ife					= 0x104C;
constexpr int e1000_dev_id_ich8_ife_gt				= 0x10C4;
constexpr int e1000_dev_id_ich8_ife_g				= 0x10C5;
constexpr int e1000_dev_id_ich8_igp_m				= 0x104D;
constexpr int e1000_dev_id_ich9_igp_amt				= 0x10BD;
constexpr int e1000_dev_id_ich9_bm					= 0x10E5;
constexpr int e1000_dev_id_ich9_igp_m_amt			= 0x10F5;
constexpr int e1000_dev_id_ich9_igp_m				= 0x10BF;
constexpr int e1000_dev_id_ich9_igp_m_v				= 0x10CB;
constexpr int e1000_dev_id_ich9_igp_c				= 0x294C;
constexpr int e1000_dev_id_ich9_ife					= 0x10C0;
constexpr int e1000_dev_id_ich9_ife_gt				= 0x10C3;
constexpr int e1000_dev_id_ich9_ife_g				= 0x10C2;
constexpr int e1000_dev_id_ich10_r_bm_lm			= 0x10CC;
constexpr int e1000_dev_id_ich10_r_bm_lf			= 0x10CD;
constexpr int e1000_dev_id_ich10_r_bm_v				= 0x10CE;
constexpr int e1000_dev_id_ich10_d_bm_lm			= 0x10DE;
constexpr int e1000_dev_id_ich10_d_bm_lf			= 0x10DF;
constexpr int e1000_dev_id_ich10_d_bm_v				= 0x1525;
constexpr int e1000_dev_id_pch_m_hv_lm				= 0x10EA;
constexpr int e1000_dev_id_pch_m_hv_lc				= 0x10EB;
constexpr int e1000_dev_id_pch_d_hv_dm				= 0x10EF;
constexpr int e1000_dev_id_pch_d_hv_dc				= 0x10F0;
constexpr int e1000_dev_id_pch2_lv_lm				= 0x1502;
constexpr int e1000_dev_id_pch2_lv_v				= 0x1503;
constexpr int e1000_dev_id_pch_lpt_i217_lm			= 0x153A;
constexpr int e1000_dev_id_pch_lpt_i217_v			= 0x153B;
constexpr int e1000_dev_id_pch_lptlp_i218_lm		= 0x155A;
constexpr int e1000_dev_id_pch_lptlp_i218_v			= 0x1559;
constexpr int e1000_dev_id_pch_i218_lm2				= 0x15A0;
constexpr int e1000_dev_id_pch_i218_v2				= 0x15A1;
constexpr int e1000_dev_id_pch_i218_lm3				= 0x15A2; 	/* Wildcat Point PCH */
constexpr int e1000_dev_id_pch_i218_v3				= 0x15A3;	/* Wildcat Point PCH */
constexpr int e1000_dev_id_pch_spt_i219_lm			= 0x156F;	/* SPT PCH */
constexpr int e1000_dev_id_pch_spt_i219_v			= 0x1570;	/* SPT PCH */
constexpr int e1000_dev_id_pch_spt_i219_lm2			= 0x15B7;	/* SPT-H PCH */
constexpr int e1000_dev_id_pch_spt_i219_v2			= 0x15B8;	/* SPT-H PCH */
constexpr int e1000_dev_id_pch_lbg_i219_lm3			= 0x15B9;	/* LBG PCH */
constexpr int e1000_dev_id_pch_spt_i219_lm4			= 0x15D7;
constexpr int e1000_dev_id_pch_spt_i219_v4			= 0x15D8;
constexpr int e1000_dev_id_pch_spt_i219_lm5			= 0x15E3;
constexpr int e1000_dev_id_pch_spt_i219_v5			= 0x15D6;
constexpr int e1000_dev_id_pch_cnp_i219_lm6			= 0x15BD;
constexpr int e1000_dev_id_pch_cnp_i219_v6			= 0x15BE;
constexpr int e1000_dev_id_pch_cnp_i219_lm7			= 0x15BB;
constexpr int e1000_dev_id_pch_cnp_i219_v7			= 0x15BC;
constexpr int e1000_dev_id_pch_icp_i219_lm8			= 0x15DF;
constexpr int e1000_dev_id_pch_icp_i219_v8			= 0x15E0;
constexpr int e1000_dev_id_pch_icp_i219_lm9			= 0x15E1;
constexpr int e1000_dev_id_pch_icp_i219_v9			= 0x15E2;
constexpr int e1000_dev_id_pch_cmp_i219_lm10		= 0x0D4E;
constexpr int e1000_dev_id_pch_cmp_i219_v10			= 0x0D4F;
constexpr int e1000_dev_id_pch_cmp_i219_lm11		= 0x0D4C;
constexpr int e1000_dev_id_pch_cmp_i219_v11			= 0x0D4D;
constexpr int e1000_dev_id_pch_cmp_i219_lm12		= 0x0D53;
constexpr int e1000_dev_id_pch_cmp_i219_v12			= 0x0D55;
constexpr int e1000_dev_id_pch_tgp_i219_lm13		= 0x15FB;
constexpr int e1000_dev_id_pch_tgp_i219_v13			= 0x15FC;
constexpr int e1000_dev_id_pch_tgp_i219_lm14		= 0x15F9;
constexpr int e1000_dev_id_pch_tgp_i219_v14			= 0x15FA;
constexpr int e1000_dev_id_pch_tgp_i219_lm15		= 0x15F4;
constexpr int e1000_dev_id_pch_tgp_i219_v15			= 0x15F5;
constexpr int e1000_dev_id_pch_rpl_i219_lm23		= 0x0DC5;
constexpr int e1000_dev_id_pch_rpl_i219_v23			= 0x0DC6;
constexpr int e1000_dev_id_pch_adp_i219_lm16		= 0x1A1E;
constexpr int e1000_dev_id_pch_adp_i219_v16			= 0x1A1F;
constexpr int e1000_dev_id_pch_adp_i219_lm17		= 0x1A1C;
constexpr int e1000_dev_id_pch_adp_i219_v17			= 0x1A1D;
constexpr int e1000_dev_id_pch_rpl_i219_lm22		= 0x0DC7;
constexpr int e1000_dev_id_pch_rpl_i219_v22			= 0x0DC8;
constexpr int e1000_dev_id_pch_mtp_i219_lm18		= 0x550A;
constexpr int e1000_dev_id_pch_mtp_i219_v18			= 0x550B;
constexpr int e1000_dev_id_pch_adp_i219_lm19		= 0x550C;
constexpr int e1000_dev_id_pch_adp_i219_v19			= 0x550D;
constexpr int e1000_dev_id_pch_lnp_i219_lm20		= 0x550E;
constexpr int e1000_dev_id_pch_lnp_i219_v20			= 0x550F;
constexpr int e1000_dev_id_pch_lnp_i219_lm21		= 0x5510;
constexpr int e1000_dev_id_pch_lnp_i219_v21			= 0x5511;
constexpr int e1000_dev_id_pch_arl_i219_lm24		= 0x57A0;
constexpr int e1000_dev_id_pch_arl_i219_v24			= 0x57A1;
constexpr int e1000_dev_id_pch_ptp_i219_lm25		= 0x57B3;
constexpr int e1000_dev_id_pch_ptp_i219_v25			= 0x57B4;
constexpr int e1000_dev_id_pch_ptp_i219_lm26		= 0x57B5;
constexpr int e1000_dev_id_pch_ptp_i219_v26			= 0x57B6;
constexpr int e1000_dev_id_pch_ptp_i219_lm27		= 0x57B7;
constexpr int e1000_dev_id_pch_ptp_i219_v27			= 0x57B8;
constexpr int e1000_dev_id_pch_nvl_i219_lm29		= 0x57B9;
constexpr int e1000_dev_id_pch_nvl_i219_v29			= 0x57BA;
constexpr int e1000_revision_4						= 4;
constexpr int e1000_func_1							= 1;
constexpr int e1000_alt_mac_address_offset_lan0		= 0;
constexpr int e1000_alt_mac_address_offset_lan1		= 3;
#endif