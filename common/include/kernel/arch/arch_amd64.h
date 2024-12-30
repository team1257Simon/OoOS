#ifndef __ARCH_AMD64
#define __ARCH_AMD64
#include "kernel/kernel_defs.h"
#ifdef __cplusplus
#include "concepts"
constexpr byte sig_pic_eoi       =   0x20;	    /* End-of-interrupt command code */
constexpr word command_pic1      =   0x20;	    /* IO base address for master PIC*/
constexpr word command_pic2      =   0xA0;		/* IO base address for slave PIC */
constexpr word data_pic1         =   0x21;      /* IO data address for master PIC*/
constexpr word data_pic2         =   0xA1;      /* IO data address for slave PIC */
constexpr byte icw1_icw4         =   0x01;		/* Indicates that ICW4 will be present */
constexpr byte icw1_single       =   0x02;		/* Single (cascade) mode */
constexpr byte icw1_interval_4   =   0x04;	    /* Call address interval 4 (8) */
constexpr byte icw1_edge         =   0x08;		/* Level triggered (edge) mode */
constexpr byte icw1_init         =   0x10;		/* Initialization - required! */
constexpr byte icw4_8086_mode    =   0x01;		/* 8086/88 (MCS-80/85) mode */
constexpr byte icw4_auto_eoi     =   0x02;		/* Auto (normal) EOI */
constexpr byte icw4_mode_slv     =   0x08;	    /* Buffered mode/slave */
constexpr byte icw4_mode_mst     =   0x0C;	    /* Buffered mode/master */
constexpr byte icw4_sfnm         =   0x10;		/* Special fully nested (not) */
constexpr byte sig_read_irr      =   0x0A;      /* OCW3 irq ready next CMD read */
constexpr byte sig_read_isr      =   0x0B;      /* OCW3 irq service next CMD read */
constexpr word command_keybd     =   0x64;      /* IO base address for keyboard controller */
constexpr word data_keybd        =   0x60;      /* IO data address for keyboard controller */
constexpr word command_rtc       =   0x70;      /* IO base address for the CMOS realtime clock */
constexpr word data_rtc          =   0x71;      /* IO data address for the CMOS realtime clock */
constexpr byte sig_keybd_ping    =   0xEE;
constexpr byte sig_keybd_rst     =   0xFF;
constexpr byte sig_keybd_enable  =   0xF4;
extern "C" 
{
#endif
typedef struct __tss_bits
{
    dword : 32;
    union ddw 
    {
        struct 
        {
            dword lo;
            dword hi;
        } __pack;
        qword full;
    } __pack __align(4) rsp[3];
    dword : 32;
    dword : 32;
    union ddw ist[7];
    dword : 32;
    dword : 32;
    union
    {
        struct 
        {
            word : 16;
            word iopb;
        } __pack;
        dword : 32;
    } __pack;
} __align(4) __pack tss;
typedef struct __tss_descriptor
{
    word limit_lo;
    dword base_lo    : 24;
    byte access;
    byte limit_hi    : 4;
    byte flags       : 4;
    byte base_mid    : 8;
    qword base_hi;
} __pack tss_descriptor;
inline void cli() noexcept { asm volatile("cli" ::: "memory"); }
inline void sti() noexcept { asm volatile("sti" ::: "memory"); }
#ifdef __cplusplus
}
template<std::integral I = byte> [[gnu::always_inline]] constexpr I in(word from) { I result; asm volatile(" in %1, %0 " : "=a"(result) : "Nd"(from) : "memory"); return result; }
template<std::integral I = byte> [[gnu::always_inline]] constexpr void out(word to, I value) { asm volatile(" out %0, %1 " :: "a"(value), "Nd"(to) : "memory"); }
[[gnu::always_inline]] constexpr void outb(word to, byte value) { out(to, value); }
[[gnu::always_inline]] constexpr byte inb(word from) { return in(from); }
[[gnu::always_inline]] constexpr void io_wait() { outb(0x80, 0); }
[[gnu::always_inline]] constexpr void outbw(word to, byte value) { outb(to, value); io_wait(); }
[[gnu::always_inline]] constexpr byte inbw(word from) { byte result = inb(from); io_wait(); return result; }
[[gnu::always_inline]] constexpr void kb_wait() { while(!(inb(command_keybd) & 0x02)); }
[[gnu::always_inline]] constexpr void kb_put(byte b) { kb_wait(); outb(data_keybd, b); kb_wait(); }
[[gnu::always_inline]] constexpr byte kb_get() { return inb(data_keybd); }
[[gnu::always_inline]] constexpr void nmi_enable() { outb(command_rtc, inb(command_rtc) & 0x7F); inb(data_rtc); }
[[gnu::always_inline]] constexpr void nmi_disable() { outb(command_rtc, inb(command_rtc) | 0x80); inb(data_rtc); }
template<byte I> [[gnu::always_inline]] constexpr byte irq_mask() { if constexpr(I < 8) return 1 << I; else return (1 << (I - 8));   }
void pic_eoi(byte irq);
template<byte O1, byte O2> constexpr void pic_remap() { byte a1 = inb(data_pic1), a2 = inb(data_pic2); outbw(command_pic1, icw1_init | icw1_icw4); outbw(command_pic2, icw1_init | icw1_icw4); outbw(data_pic1, O1); outbw(data_pic2, O2); outbw(data_pic1, 4); outbw(data_pic2, 2); outbw(data_pic1, icw4_8086_mode); outbw(data_pic2, icw4_8086_mode); outb(data_pic1, a1); outb(data_pic2, a2); }
template<byte I> constexpr void irq_set_mask() { outb(data_pic1, inb(data_pic1) | irq_mask<I>()); }
template<byte I> constexpr void irq_clear_mask() { outb(data_pic1, inb(data_pic1) & ~(irq_mask<I>())); }
template<byte R> constexpr void rtc_select() { byte prev = inbw(command_rtc); outbw(command_rtc, (prev & 0x80) | R); }
template<byte R> constexpr byte read_rtc_register() { rtc_select<R>(); return inb(data_rtc); }
template<byte R> constexpr void write_rtc_register(byte val) { rtc_select<R>(); outb(data_rtc, val); }
constexpr bool is_cmos_update_in_progress() { return (read_rtc_register<0x0A>() & 0x80) != 0; }
[[gnu::always_inline]] constexpr byte kb_ping() { kb_put(sig_keybd_ping); return kb_get(); }
[[gnu::always_inline]] constexpr byte kb_rst() { kb_put(sig_keybd_rst); return kb_get(); }
[[gnu::always_inline]] constexpr byte kb_enable() { irq_clear_mask<1>(); do { kb_rst(); } while (kb_ping() != sig_keybd_ping); kb_put(sig_keybd_enable); return kb_get(); }
template<dword R> [[gnu::always_inline]] constexpr qword read_msr() { dword lo, hi;  asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(R) : "memory"); return static_cast<qword>(static_cast<qword>(lo) | (static_cast<qword>(hi) << 32)); }
template<dword R> [[gnu::always_inline]] constexpr void write_msr(qword value) { asm volatile("wrmsr" :: "a"(static_cast<dword>(value & 0xFFFFFFFF)), "d"(static_cast<dword>((value >> 32) & 0xFFFFFFFF)), "c"(R) : "memory"); }
#endif
#endif