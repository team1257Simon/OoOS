#ifndef __ARCH_AMD64
#define __ARCH_AMD64
#include "kernel/kernel_defs.h"
#ifdef __cplusplus
#include "concepts"
extern "C" 
{
#endif
typedef struct __tss_bits
{
    uint32_t : 32;
    union ddw 
    {
        struct 
        {
            uint32_t lo;
            uint32_t hi;
        } __pack;
        uint64_t full;
    } __pack __align(4) rsp[3];
    uint32_t : 32;
    uint32_t : 32;
    union ddw ist[7];
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
} __align(4) __pack tss;
typedef struct __tss_descriptor
{
    uint16_t limit_lo;
    uint32_t base_lo    : 24;
    uint8_t access;
    uint8_t limit_hi    : 4;
    uint8_t flags       : 4;
    uint8_t base_mid    : 8;
    uint64_t base_hi;
} __pack tss_descriptor;
inline void cli() noexcept { asm volatile("cli" ::: "memory"); }
inline void sti() noexcept { asm volatile("sti" ::: "memory"); }
#ifdef __cplusplus
}
template<std::integral I> [[gnu::always_inline]] constexpr I in(uint16_t from) { I result; asm volatile(" in %1, %0 " : "=a"(result) : "Nd"(from) : "memory"); return result; }
template<std::integral I> [[gnu::always_inline]] constexpr void out(uint16_t to, I value) { asm volatile(" out %0, %1 " :: "a"(value), "Nd"(to) : "memory"); }
[[gnu::always_inline]] constexpr void io_wait() { out(0x80, byte {0}); }
#endif
#endif