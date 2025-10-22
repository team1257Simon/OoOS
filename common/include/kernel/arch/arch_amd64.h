#ifndef __ARCH_AMD64
#define __ARCH_AMD64
#include <libk_decls.h>
#ifdef __cplusplus
#include <concepts>
constexpr byte sig_pic_eoi       =   0x20UC;	    /* End-of-interrupt command code */
constexpr word command_pic1      =   0x20UC;	    /* IO base address for master PIC*/
constexpr word command_pic2      =   0xA0UC;		/* IO base address for slave PIC */
constexpr word data_pic1         =   0x21UC;        /* IO data address for master PIC*/
constexpr word data_pic2         =   0xA1UC;        /* IO data address for slave PIC */
constexpr byte icw1_icw4         =   0x01UC;		/* Indicates that ICW4 will be present */
constexpr byte icw1_single       =   0x02UC;		/* Single (cascade) mode */
constexpr byte icw1_interval_4   =   0x04UC;	    /* Call address interval 4 (8) */
constexpr byte icw1_edge         =   0x08UC;		/* Level triggered (edge) mode */
constexpr byte icw1_init         =   0x10UC;		/* Initialization - required! */
constexpr byte icw4_8086_mode    =   0x01UC;		/* 8086/88 (MCS-80/85) mode */
constexpr byte icw4_auto_eoi     =   0x02UC;		/* Auto (normal) EOI */
constexpr byte icw4_mode_slv     =   0x08UC;	    /* Buffered mode/slave */
constexpr byte icw4_mode_mst     =   0x0CUC;	    /* Buffered mode/master */
constexpr byte icw4_sfnm         =   0x10UC;		/* Special fully nested (not) */
constexpr byte sig_read_irr      =   0x0AUC;        /* OCW3 irq ready next CMD read */
constexpr byte sig_read_isr      =   0x0BUC;        /* OCW3 irq service next CMD read */
constexpr word command_keybd     =   0x64UC;        /* IO base address for keyboard controller */
constexpr word data_keybd        =   0x60UC;        /* IO data address for keyboard controller */
constexpr word command_rtc       =   0x70UC;        /* IO base address for the CMOS realtime clock */
constexpr word data_rtc          =   0x71UC;        /* IO data address for the CMOS realtime clock */
constexpr byte sig_keybd_ping    =   0xEEUC;        /* The keyboard should respond with an equal byte */
constexpr byte sig_keybd_rst     =   0xFFUC;        /* Use this to reset the keyboard, for instance at startup to ensure no stale data */
constexpr byte sig_keybd_enable  =   0xF4UC;        /* Send to enable the keyboard. This must be used after any reset is performed */
extern "C" 
{
#endif
typedef struct attribute(packed, aligned(4)) __tss_bits
{
    uint32_t : 32;
    addr_t rsp[3];
    uint32_t : 32;
    uint32_t : 32;
    addr_t ist[7];
    uint32_t : 32;
    uint32_t : 32;
    union
    {
        struct 
        {
            uint16_t : 16;
            uint16_t iopb;
        } __pack;
        uint32_t : 32;
    } __pack;
} tss;
typedef struct __pack __tss_descriptor
{
    uint16_t limit_lo;
    uint32_t base_lo    : 24;
    uint8_t access;
    uint8_t limit_hi    : 4;
    uint8_t flags       : 4;
    uint8_t base_mid    : 8;
    uint64_t base_hi;
} tss_descriptor;
typedef struct __cpuid_leaf
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_leaf;
inline void cli() noexcept { asm volatile("cli" ::: "memory"); }
inline void sti() noexcept { asm volatile("sti" ::: "memory"); }
inline cpuid_leaf cpuid(uint32_t a, uint32_t c) { cpuid_leaf l; asm volatile("cpuid" : "=a"(l.eax), "=b"(l.ebx), "=c"(l.ecx), "=d"(l.edx) : "0"(a), "2"(c) : "memory"); return l; }
#ifdef __cplusplus
}
template<integral_structure I = byte> constexpr I in(uint16_t from) { I result; asm volatile("in %1, %0" : "=a"(result) : "Nd"(from) : "memory"); return result; }
template<integral_structure I = byte> constexpr void out(uint16_t to, I value) { asm volatile("out %0, %1" :: "a"(value), "Nd"(to) : "memory"); }
constexpr void outb(uint16_t to, byte value) { out(to, value); }
constexpr byte inb(uint16_t from) { return in(from); }
constexpr void io_wait() { outb(0x80US, 0); }
constexpr void outbw(uint16_t to, byte value) { outb(to, value); io_wait(); }
constexpr byte inbw(uint16_t from) { byte result = inb(from); io_wait(); return result; }
constexpr void kb_wait() { for(uint8_t result = inb(0x64US); result & 0x02UC; result = inb(0x64US)); }
constexpr void kb_put(byte b) { kb_wait(); outb(data_keybd, b); kb_wait(); }
constexpr byte kb_get() { return inb(data_keybd); }
constexpr void nmi_enable() { outb(command_rtc, inb(command_rtc) & 0x7FUC); inb(data_rtc); }
constexpr void nmi_disable() { outb(command_rtc, inb(command_rtc) | 0x80UC); inb(data_rtc); }
template<uint8_t O1, uint8_t O2> constexpr void pic_remap() { outbw(command_pic1, icw1_init | icw1_icw4); outbw(command_pic2, icw1_init | icw1_icw4); outbw(data_pic1, O1); outbw(data_pic2, O2); outbw(data_pic1, 4); outbw(data_pic2, 2); outbw(data_pic1, icw4_8086_mode); outbw(data_pic2, icw4_8086_mode); outb(data_pic1, 0UC); outb(data_pic2, 0UC); }
template<uint8_t R> constexpr void rtc_select() { byte reg = R; byte prev = inbw(command_rtc); outbw(command_rtc, (prev & 0x80) | R); }
template<uint8_t R> constexpr byte read_rtc_register() { rtc_select<R>(); return inb(data_rtc); }
template<uint8_t R> constexpr void write_rtc_register(byte val) { rtc_select<R>(); outb(data_rtc, val); }
constexpr bool is_cmos_update_in_progress() { return (read_rtc_register<0x0AUC>() & 0x80) != 0; }
constexpr byte kb_ping() { kb_put(sig_keybd_ping); return kb_get(); }
constexpr byte kb_reset() { do { kb_put(sig_keybd_rst); kb_get(); } while (kb_ping() != sig_keybd_ping); kb_put(sig_keybd_enable); return kb_get(); }
template<uint32_t R> constexpr qword read_msr() { dword lo, hi; asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(R) : "memory"); return qword(lo, hi); }
template<uint32_t R> constexpr void write_msr(qword value) { asm volatile("wrmsr" :: "a"(value.lo), "d"(value.hi), "c"(R) : "memory"); }
constexpr dword kernel_gs_base = 0xC0000102;
constexpr dword ia32_efer      = 0xC0000080;
constexpr dword ia32_star      = 0xC0000081;
constexpr dword lstar          = 0xC0000082;
constexpr dword cstar          = 0xC0000083;
constexpr dword sfmask         = 0xC0000084;
constexpr dword ia32_apic_base = 0x0000001B;
constexpr dword apic_base_bsp  = 0x00000100;
constexpr dword apic_enable    = 0x00000800;
constexpr void set_kernel_gs_base(addr_t value) { write_msr<kernel_gs_base>(value.full); }
constexpr void init_syscall_msrs(addr_t syscall_target, qword flag_mask, word pl0_segbase, word pl3_segbase) { qword star = syscall_target.full; star.hi.lo = pl0_segbase; star.hi.hi = pl3_segbase; qword stbase = syscall_target.full; write_msr<ia32_star>(star); write_msr<cstar>(stbase); write_msr<lstar>(stbase); write_msr<sfmask>(flag_mask); qword efer = read_msr<ia32_efer>(); efer |= 1; write_msr<ia32_efer>(efer); }
#endif
#endif