#ifndef __KERNEL_DEF
#define __KERNEL_DEF
/* Copyright (C) 2008-2024 Free Software Foundation, Inc.
This file is part of GCC.
GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.
GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
Under Section 7 of GPL version 3, you are granted additional
permissions described in the GCC Runtime Library Exception, version
3.1, as published by the Free Software Foundation.
You should have received a copy of the GNU General Public License and
a copy of the GCC Runtime Library Exception along with this program;
see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
<http://www.gnu.org/licenses/>.  */
#ifndef _GCC_STDINT_H
#define _GCC_STDINT_H
#ifdef __INT8_TYPE__
typedef __INT8_TYPE__ int8_t;
#endif
#ifdef __INT16_TYPE__
typedef __INT16_TYPE__ int16_t;
#endif
#ifdef __INT32_TYPE__
typedef __INT32_TYPE__ int32_t;
#endif
#ifdef __INT64_TYPE__
typedef __INT64_TYPE__ int64_t;
#endif
#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ uint8_t;
#endif
#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ uint16_t;
#endif
#ifdef __UINT32_TYPE__
typedef __UINT32_TYPE__ uint32_t;
#endif
#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ uint64_t;
#endif
typedef __INT_LEAST8_TYPE__ int_least8_t;
typedef __INT_LEAST16_TYPE__ int_least16_t;
typedef __INT_LEAST32_TYPE__ int_least32_t;
typedef __INT_LEAST64_TYPE__ int_least64_t;
typedef __UINT_LEAST8_TYPE__ uint_least8_t;
typedef __UINT_LEAST16_TYPE__ uint_least16_t;
typedef __UINT_LEAST32_TYPE__ uint_least32_t;
typedef __UINT_LEAST64_TYPE__ uint_least64_t;
typedef __INT_FAST8_TYPE__ int_fast8_t;
typedef __INT_FAST16_TYPE__ int_fast16_t;
typedef __INT_FAST32_TYPE__ int_fast32_t;
typedef __INT_FAST64_TYPE__ int_fast64_t;
typedef __UINT_FAST8_TYPE__ uint_fast8_t;
typedef __UINT_FAST16_TYPE__ uint_fast16_t;
typedef __UINT_FAST32_TYPE__ uint_fast32_t;
typedef __UINT_FAST64_TYPE__ uint_fast64_t;
#ifdef __INTPTR_TYPE__
typedef __INTPTR_TYPE__ intptr_t;
#endif
#ifdef __UINTPTR_TYPE__
typedef __UINTPTR_TYPE__ uintptr_t;
#endif
typedef __INTMAX_TYPE__ intmax_t;
typedef __UINTMAX_TYPE__ uintmax_t;
#if (!defined __cplusplus || __cplusplus >= 201103L || defined __STDC_LIMIT_MACROS)
#ifdef __INT8_MAX__
#undef INT8_MAX
#define INT8_MAX __INT8_MAX__
#undef INT8_MIN
#define INT8_MIN (-INT8_MAX - 1)
#endif
#ifdef __UINT8_MAX__
#undef UINT8_MAX
#define UINT8_MAX __UINT8_MAX__
#endif
#ifdef __INT16_MAX__
#undef INT16_MAX
#define INT16_MAX __INT16_MAX__
#undef INT16_MIN
#define INT16_MIN (-INT16_MAX - 1)
#endif
#ifdef __UINT16_MAX__
#undef UINT16_MAX
#define UINT16_MAX __UINT16_MAX__
#endif
#ifdef __INT32_MAX__
#undef INT32_MAX
#define INT32_MAX __INT32_MAX__
#undef INT32_MIN
#define INT32_MIN (-INT32_MAX - 1)
#endif
#ifdef __UINT32_MAX__
#undef UINT32_MAX
#define UINT32_MAX __UINT32_MAX__
#endif
#ifdef __INT64_MAX__
#undef INT64_MAX
#define INT64_MAX __INT64_MAX__
#undef INT64_MIN
#define INT64_MIN (-INT64_MAX - 1)
#endif
#ifdef __UINT64_MAX__
#undef UINT64_MAX
#define UINT64_MAX __UINT64_MAX__
#endif
#undef INT_LEAST8_MAX
#define INT_LEAST8_MAX __INT_LEAST8_MAX__
#undef INT_LEAST8_MIN
#define INT_LEAST8_MIN (-INT_LEAST8_MAX - 1)
#undef UINT_LEAST8_MAX
#define UINT_LEAST8_MAX __UINT_LEAST8_MAX__
#undef INT_LEAST16_MAX
#define INT_LEAST16_MAX __INT_LEAST16_MAX__
#undef INT_LEAST16_MIN
#define INT_LEAST16_MIN (-INT_LEAST16_MAX - 1)
#undef UINT_LEAST16_MAX
#define UINT_LEAST16_MAX __UINT_LEAST16_MAX__
#undef INT_LEAST32_MAX
#define INT_LEAST32_MAX __INT_LEAST32_MAX__
#undef INT_LEAST32_MIN
#define INT_LEAST32_MIN (-INT_LEAST32_MAX - 1)
#undef UINT_LEAST32_MAX
#define UINT_LEAST32_MAX __UINT_LEAST32_MAX__
#undef INT_LEAST64_MAX
#define INT_LEAST64_MAX __INT_LEAST64_MAX__
#undef INT_LEAST64_MIN
#define INT_LEAST64_MIN (-INT_LEAST64_MAX - 1)
#undef UINT_LEAST64_MAX
#define UINT_LEAST64_MAX __UINT_LEAST64_MAX__
#undef INT_FAST8_MAX
#define INT_FAST8_MAX __INT_FAST8_MAX__
#undef INT_FAST8_MIN
#define INT_FAST8_MIN (-INT_FAST8_MAX - 1)
#undef UINT_FAST8_MAX
#define UINT_FAST8_MAX __UINT_FAST8_MAX__
#undef INT_FAST16_MAX
#define INT_FAST16_MAX __INT_FAST16_MAX__
#undef INT_FAST16_MIN
#define INT_FAST16_MIN (-INT_FAST16_MAX - 1)
#undef UINT_FAST16_MAX
#define UINT_FAST16_MAX __UINT_FAST16_MAX__
#undef INT_FAST32_MAX
#define INT_FAST32_MAX __INT_FAST32_MAX__
#undef INT_FAST32_MIN
#define INT_FAST32_MIN (-INT_FAST32_MAX - 1)
#undef UINT_FAST32_MAX
#define UINT_FAST32_MAX __UINT_FAST32_MAX__
#undef INT_FAST64_MAX
#define INT_FAST64_MAX __INT_FAST64_MAX__
#undef INT_FAST64_MIN
#define INT_FAST64_MIN (-INT_FAST64_MAX - 1)
#undef UINT_FAST64_MAX
#define UINT_FAST64_MAX __UINT_FAST64_MAX__
#ifdef __INTPTR_MAX__
#undef INTPTR_MAX
#define INTPTR_MAX __INTPTR_MAX__
#undef INTPTR_MIN
#define INTPTR_MIN (-INTPTR_MAX - 1)
#endif
#ifdef __UINTPTR_MAX__
#undef UINTPTR_MAX
#define UINTPTR_MAX __UINTPTR_MAX__
#endif
#undef INTMAX_MAX
#define INTMAX_MAX __INTMAX_MAX__
#undef INTMAX_MIN
#define INTMAX_MIN (-INTMAX_MAX - 1)
#undef UINTMAX_MAX
#define UINTMAX_MAX __UINTMAX_MAX__
#undef PTRDIFF_MAX
#define PTRDIFF_MAX __PTRDIFF_MAX__
#undef PTRDIFF_MIN
#define PTRDIFF_MIN (-PTRDIFF_MAX - 1)
#undef SIG_ATOMIC_MAX
#define SIG_ATOMIC_MAX __SIG_ATOMIC_MAX__
#undef SIG_ATOMIC_MIN
#define SIG_ATOMIC_MIN __SIG_ATOMIC_MIN__
#undef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#undef WCHAR_MAX
#define WCHAR_MAX __WCHAR_MAX__
#undef WCHAR_MIN
#define WCHAR_MIN __WCHAR_MIN__
#undef WINT_MAX
#define WINT_MAX __WINT_MAX__
#undef WINT_MIN
#define WINT_MIN __WINT_MIN__
#endif
#if (!defined __cplusplus || __cplusplus >= 201103L || defined __STDC_CONSTANT_MACROS)
#undef INT8_C
#define INT8_C(c) __INT8_C(c)
#undef INT16_C
#define INT16_C(c) __INT16_C(c)
#undef INT32_C
#define INT32_C(c) __INT32_C(c)
#undef INT64_C
#define INT64_C(c) __INT64_C(c)
#undef UINT8_C
#define UINT8_C(c) __UINT8_C(c)
#undef UINT16_C
#define UINT16_C(c) __UINT16_C(c)
#undef UINT32_C
#define UINT32_C(c) __UINT32_C(c)
#undef UINT64_C
#define UINT64_C(c) __UINT64_C(c)
#undef INTMAX_C
#define INTMAX_C(c) __INTMAX_C(c)
#undef UINTMAX_C
#define UINTMAX_C(c) __UINTMAX_C(c)
#endif
#if (defined __STDC_WANT_IEC_60559_BFP_EXT__  || (defined (__STDC_VERSION__) && __STDC_VERSION__ > 201710L))
#ifdef __INT8_TYPE__
#undef INT8_WIDTH
#define INT8_WIDTH 8
#endif
#ifdef __UINT8_TYPE__
#undef UINT8_WIDTH
#define UINT8_WIDTH 8
#endif
#ifdef __INT16_TYPE__
#undef INT16_WIDTH
#define INT16_WIDTH 16
#endif
#ifdef __UINT16_TYPE__
#undef UINT16_WIDTH
#define UINT16_WIDTH 16
#endif
#ifdef __INT32_TYPE__
#undef INT32_WIDTH
#define INT32_WIDTH 32
#endif
#ifdef __UINT32_TYPE__
#undef UINT32_WIDTH
#define UINT32_WIDTH 32
#endif
#ifdef __INT64_TYPE__
#undef INT64_WIDTH
#define INT64_WIDTH 64
#endif
#ifdef __UINT64_TYPE__
#undef UINT64_WIDTH
#define UINT64_WIDTH 64
#endif
#undef INT_LEAST8_WIDTH
#define INT_LEAST8_WIDTH __INT_LEAST8_WIDTH__
#undef UINT_LEAST8_WIDTH
#define UINT_LEAST8_WIDTH __INT_LEAST8_WIDTH__
#undef INT_LEAST16_WIDTH
#define INT_LEAST16_WIDTH __INT_LEAST16_WIDTH__
#undef UINT_LEAST16_WIDTH
#define UINT_LEAST16_WIDTH __INT_LEAST16_WIDTH__
#undef INT_LEAST32_WIDTH
#define INT_LEAST32_WIDTH __INT_LEAST32_WIDTH__
#undef UINT_LEAST32_WIDTH
#define UINT_LEAST32_WIDTH __INT_LEAST32_WIDTH__
#undef INT_LEAST64_WIDTH
#define INT_LEAST64_WIDTH __INT_LEAST64_WIDTH__
#undef UINT_LEAST64_WIDTH
#define UINT_LEAST64_WIDTH __INT_LEAST64_WIDTH__
#undef INT_FAST8_WIDTH
#define INT_FAST8_WIDTH __INT_FAST8_WIDTH__
#undef UINT_FAST8_WIDTH
#define UINT_FAST8_WIDTH __INT_FAST8_WIDTH__
#undef INT_FAST16_WIDTH
#define INT_FAST16_WIDTH __INT_FAST16_WIDTH__
#undef UINT_FAST16_WIDTH
#define UINT_FAST16_WIDTH __INT_FAST16_WIDTH__
#undef INT_FAST32_WIDTH
#define INT_FAST32_WIDTH __INT_FAST32_WIDTH__
#undef UINT_FAST32_WIDTH
#define UINT_FAST32_WIDTH __INT_FAST32_WIDTH__
#undef INT_FAST64_WIDTH
#define INT_FAST64_WIDTH __INT_FAST64_WIDTH__
#undef UINT_FAST64_WIDTH
#define UINT_FAST64_WIDTH __INT_FAST64_WIDTH__
#ifdef __INTPTR_TYPE__
#undef INTPTR_WIDTH
#define INTPTR_WIDTH __INTPTR_WIDTH__
#endif
#ifdef __UINTPTR_TYPE__
#undef UINTPTR_WIDTH
#define UINTPTR_WIDTH __INTPTR_WIDTH__
#endif
#undef INTMAX_WIDTH
#define INTMAX_WIDTH __INTMAX_WIDTH__
#undef UINTMAX_WIDTH
#define UINTMAX_WIDTH __INTMAX_WIDTH__
#undef PTRDIFF_WIDTH
#define PTRDIFF_WIDTH __PTRDIFF_WIDTH__
#undef SIG_ATOMIC_WIDTH
#define SIG_ATOMIC_WIDTH __SIG_ATOMIC_WIDTH__
#undef SIZE_WIDTH
#define SIZE_WIDTH __SIZE_WIDTH__
#undef WCHAR_WIDTH
#define WCHAR_WIDTH __WCHAR_WIDTH__
#undef WINT_WIDTH
#define WINT_WIDTH __WINT_WIDTH__
#endif
#if defined __STDC_VERSION__ && __STDC_VERSION__ > 201710L
#define __STDC_VERSION_STDINT_H__	202311L
#endif
#endif /* _GCC_STDINT_H */
#include "stddef.h"
#define attribute(x) __attribute__((x))
#define extension __extension__
#ifndef KERNEL_FILENAME
#define KERNEL_FILENAME "\\SYS\\CORE.ELF"
#endif
#define PAGESIZE 0x1000UL
#define PT_LEN 0x200UL
#define MMAP_MAX_PG 0x100000UL
#define HAVE_SIZE_T 1
#define HAVE_STDINT 1
#define __pack attribute(packed)
#define __align(n) attribute(aligned(n))
#define __isrcall [[gnu::target("general-regs-only")]]
#ifndef __cplusplus
#ifdef NEED_STDBOOL
#include "stdbool.h"
#endif
#define physical_block_size 512UL
#define __may_alias
#else
constexpr size_t physical_block_size = 512UL;
#define __may_alias [[gnu::may_alias]]
#define restrict
#include "concepts"
#include "compare"
#include "bits/move.h"
template<class T> concept not_void_ptr = !std::same_as<std::remove_cvref_t<T>, void*>;
template<class T> concept non_void = !std::is_void_v<T>;
#endif
#define PAUSE asm volatile ("pause" ::: "memory")
#define BARRIER asm volatile ("" ::: "memory")
typedef enum mem_type
{
    AVAILABLE = 1,
    RESERVED = 2,
    ACPI_RECLAIMABLE = 3,
    NVS = 4,
    BADRAM = 5,
    MMIO = 6
} memtype_t;
typedef struct __pt_entry
{
    bool present                 : 1;
    bool write                   : 1;
    bool user_access             : 1;
    bool write_thru              : 1;
    bool cache_disable           : 1;
    bool accessed                : 1;
    bool dirty                   : 1;
    bool page_size               : 1;
    bool global                  : 1;
    bool avl0                    : 1;
    bool avl1                    : 1;
    bool avl2                    : 1;
    uint64_t physical_address    : 36;
    uint16_t avl4                : 11;
    uint8_t pk                   : 4;
    bool execute_disable         : 1;
} __pack __align(1) pt_entry;
typedef pt_entry* paging_table;
typedef struct __vaddr
{
#ifdef __cplusplus 
    uint16_t offset     : 12 { 0 };
    uint16_t page_idx   :  9 { 0 };
    uint16_t pd_idx     :  9 { 0 };
    uint16_t pdp_idx    :  9 { 0 };
    uint16_t pml4_idx   :  9 { 0 };
    uint16_t ext        : 16 { 0 };
    constexpr explicit __vaddr(uint16_t offs, uint16_t idx0, uint16_t idx1, uint16_t idx2, uint16_t idx3, uint16_t sign) noexcept :
        offset      { offs },
        page_idx    { idx0 },
        pd_idx      { idx1 },
        pdp_idx     { idx2 },
        pml4_idx    { idx3 },
        ext         { sign } 
                    {}
    constexpr explicit __vaddr(uint64_t i) noexcept :
        offset      { static_cast<uint16_t>(i & 0x0FFF) },
        page_idx    { static_cast<uint16_t>(uint64_t(i >> 12) & 0x1FFuL) }, 
        pd_idx      { static_cast<uint16_t>(uint64_t(i >> 21) & 0x1FFuL) },
        pdp_idx     { static_cast<uint16_t>(uint64_t(i >> 30) & 0x1FFuL) },
        pml4_idx    { static_cast<uint16_t>(uint64_t(i >> 39) & 0x1FFuL) },
        ext         { static_cast<uint16_t>(pml4_idx & 0x100 ? 0xFFFFu : 0u) }
                    {}
    constexpr __vaddr(nullptr_t) noexcept : __vaddr{ 0UL } {}
    constexpr __vaddr(void* ptr) noexcept : __vaddr{ std::bit_cast<uintptr_t>(ptr) } {}
    constexpr __vaddr(const void* ptr) noexcept : __vaddr{ std::bit_cast<uintptr_t>(ptr) } {}
    constexpr explicit __vaddr(volatile void* ptr) noexcept : __vaddr{ std::bit_cast<uintptr_t>(ptr) } {}
    constexpr explicit __vaddr(const volatile void* ptr) noexcept : __vaddr{ std::bit_cast<uintptr_t>(ptr) } {}
    template<non_void T> requires(!std::is_function_v<T>) constexpr __vaddr(T* ptr) noexcept : __vaddr{ static_cast<void*>(ptr) } {}
    template<non_void T> requires(!std::is_function_v<T>) constexpr __vaddr(const T* ptr) noexcept : __vaddr{ static_cast<const void*>(ptr) } {}
    template<non_void T> requires(!std::is_function_v<T>) constexpr explicit __vaddr(volatile T* ptr) noexcept : __vaddr{ static_cast<volatile void*>(ptr) } {}
    template<non_void T> requires(!std::is_function_v<T>) constexpr explicit __vaddr(const volatile T* ptr) noexcept : __vaddr{ static_cast<const volatile void*>(ptr) } {}
    template<typename RT, typename ... Args> constexpr explicit __vaddr(RT (*funcptr)(Args...)) noexcept : __vaddr{ std::bit_cast<void*>(funcptr) } {}
    constexpr __vaddr() = default;
    constexpr ~__vaddr() = default;
    constexpr __vaddr(__vaddr const&) = default;
    constexpr __vaddr(__vaddr &&) = default;
    constexpr __vaddr& operator=(__vaddr const&) = default;
    constexpr __vaddr& operator=(__vaddr &&) = default;
    constexpr uintptr_t val() const noexcept
    {
        return static_cast<uintptr_t>
        (
            (static_cast<uint64_t>(offset))         |
            (static_cast<uint64_t>(page_idx) << 12) |
            (static_cast<uint64_t>(pd_idx)   << 21) |
            (static_cast<uint64_t>(pdp_idx)  << 30) |
            (static_cast<uint64_t>(pml4_idx) << 39) |
            (static_cast<uint64_t>(ext)      << 48)
        );
    }
    constexpr operator uintptr_t() const noexcept { return val(); }
    constexpr __vaddr operator+(ptrdiff_t value) const { return __vaddr{ uintptr_t(val() + value) }; }
    constexpr __vaddr& operator+=(ptrdiff_t value) { return *this = (*this + value); }
    constexpr __vaddr operator%(uint64_t unit) const { return __vaddr{ val() % unit }; }
    constexpr __vaddr& operator%=(uint64_t unit) { return *this = (*this % unit); }
    constexpr __vaddr operator-(ptrdiff_t value) const { return __vaddr{ uintptr_t(val() - value) }; }
    constexpr __vaddr& operator-=(ptrdiff_t value) { return *this = (*this - value); }
    constexpr __vaddr plus(ptrdiff_t value) const { return *this + ptrdiff_t(value); }
    constexpr __vaddr minus(ptrdiff_t value) const { return *this - ptrdiff_t(value); }
    constexpr __vaddr page_aligned() const noexcept { return minus(uintptr_t(*this % PAGESIZE)); }
    typedef const void* cvptr;
    typedef volatile void* vvptr;
    typedef const volatile void* cvvptr;
    template<non_void T> using ctptr = const T*;
    template<non_void T> using vtptr = volatile T*;
    template<non_void T> using cvtptr = const volatile T*;
    constexpr operator void*() const noexcept { void* ptr = std::bit_cast<void*>(val()); return ptr ? ptr : nullptr; }
    constexpr operator cvptr() const noexcept { const void* ptr = std::bit_cast<const void*>(val()); return ptr ? ptr : nullptr; }
    constexpr operator vvptr() const volatile noexcept { volatile void* ptr = std::bit_cast<volatile void*>(uintptr_t(const_cast<__vaddr*>(this))); return ptr ? ptr : nullptr; }
    constexpr operator cvvptr() const volatile noexcept { const volatile void* ptr = std::bit_cast<const volatile void*>(uintptr_t(const_cast<__vaddr*>(this))); return ptr ? ptr : nullptr;  }
    template<non_void T> constexpr operator T*() const noexcept { return std::bit_cast<std::remove_cv_t<T>*>(val()); }
    template<non_void T> constexpr operator ctptr<T>() const noexcept { return std::bit_cast<const std::remove_cv_t<T>*>(val()); }
    template<non_void T> constexpr operator vtptr<T>() const volatile noexcept { return std::bit_cast<volatile std::remove_cv_t<T>*>(static_cast<vvptr>(*this)); }
    template<non_void T> constexpr operator cvtptr<T>() const volatile noexcept { return std::bit_cast<const volatile std::remove_cv_t<T>*>(static_cast<cvvptr>(*this)); }
    constexpr operator bool() const noexcept { return bool(this->operator void*()); }
    constexpr bool operator!() const noexcept { return !(this->operator void*()); }
    friend constexpr bool operator==(__vaddr const& __this, __vaddr const& __that) noexcept { return __this.val() == __that.val(); }
    friend constexpr ptrdiff_t operator-(uintptr_t __this, __vaddr const& __that) noexcept { return __this - __that.val(); }
    friend constexpr ptrdiff_t operator-(__vaddr const& __this, __vaddr const& __that) noexcept { return __this.val() - __that.val(); }
    friend constexpr std::strong_ordering operator<=>(__vaddr const& __this, __vaddr const& __that) noexcept { return __this.val() <=> __that.val(); }
    friend constexpr std::strong_ordering operator<=>(uintptr_t __this, __vaddr const& __that) noexcept { return __this <=> __that.val(); }
    friend constexpr std::strong_ordering operator<=>(__vaddr const& __this, uintptr_t __that) noexcept { return __this.val() <=> __that; }
    constexpr bool is_canonical() const noexcept { return (((pml4_idx & 0x100) == 0) && (ext == 0)) || (((pml4_idx & 0x100) != 0) && (ext == 0xFFFF)); }
#else
    uint16_t offset     : 12;
    uint16_t page_idx   :  9;
    uint16_t pd_idx     :  9;
    uint16_t pdp_idx    :  9;
    uint16_t pml4_idx   :  9;
    uint16_t ext        : 16;
#endif
} __pack __align(1) addr_t;
#ifdef __cplusplus
extern "C" {
#define CXX_INI(val) { val }
#else 
#define CXX_INI(val)
#endif
struct acpi_header 
{
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __pack;
struct xsdp_t 
{
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;      // deprecated since version 2.0
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __pack;
struct xsdt_t
{
    struct acpi_header hdr;
    uintptr_t __align(4) sdt_pointers[];
} __pack;
void* find_system_table(struct xsdt_t* xsdt, const char* expected_sig);
typedef struct __generic_address_structure
{
  uint8_t address_space;
  uint8_t bit_width;
  uint8_t bit_offset;
  uint8_t access_size;
  uint64_t address;
}__attribute__ ((packed)) generic_address_structure;
struct dsdt
{
    struct acpi_header h;
    uint8_t data[];
} __pack;
struct fadt_t
{
    struct   acpi_header h; // "FACP"
    uint32_t firmware_ctrl;
    uint32_t dsdt_legacy;
    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  rsv0;
    uint8_t  preferred_power_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  s4bios_req;
    uint8_t  pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t  pm1_event_length;
    uint8_t  pm1_control_length;
    uint8_t  pm2_control_length;
    uint8_t  pm_timer_length;
    uint8_t  gpe0_length;
    uint8_t  gpe1_length;
    uint8_t  gpe1_base;
    uint8_t  cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t  duty_offset;
    uint8_t  duty_width;
    uint8_t  day_alarm;
    uint8_t  month_alarm;
    uint8_t  century_register;
    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t arch_flags;
    uint8_t  rsv1;
    uint32_t flags;
    // 12 byte structure; see below for details
    generic_address_structure reset_register;
    uint8_t  reset_value;
    uint8_t  rsv3[3];
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                  ext_firmware_control;
    struct dsdt*              ext_dsdt;
    generic_address_structure ext_pm1a_event_block;
    generic_address_structure ext_pm1b_event_block;
    generic_address_structure ext_pm1a_control_block;
    generic_address_structure ext_pm1b_control_block;
    generic_address_structure ext_pm2_control_block;
    generic_address_structure ext_pm_timer_block;
    generic_address_structure ext_gpe0_block;
    generic_address_structure ext_gpe1_block;
} __pack;
struct madt_t
{
    struct acpi_header header; // "APIC"
    uint32_t local_apic_physical_address;
    uint32_t multiple_apic_flags;
    uint8_t record_data[];
} __pack;
enum madt_record_type
#ifdef __cplusplus 
: uint8_t
#endif
{
    LOCAL_APIC = 0x0,
    IO_APIC = 0x1,
    INTERRUPT_SOURCE_OVERRIDE = 0x2,
    NMI_SOURCE_OVERRIDE = 0x3,
    LOCAL_APIC_NMI = 0x4,
    APIC_ADDRESS_OVERRIDE = 0x5,
    IO_SAPIC = 0x6,
    LOCAL_SAPIC = 0x7,
    PLATFORM_INTERRUPT_SOURCE = 0x8,
    LOCAL_2XAPIC = 0x9,
    LOCAL_2XAPIC_NMI = 0xA,
    GICC = 0xB,
    GICD = 0xC,
    GIC_MSI_FRAME = 0xD,
    GICR = 0xE,
    GIC_ITS = 0xF,
    MULTUPROCESSOR_WAKEUP = 0x10,
    CORE_PIC = 0x11,
    LIO_PIC = 0x12,
    HT_PIC = 0x13,
    EIO_PIC = 0x14,
    MSI_PIC = 0x15,
    BIO_PIC = 0x16,
    LPI_PIC = 0x17
};
struct madt_record_header
{
    enum madt_record_type type;
    uint8_t length;
} __pack;
typedef union
{
    struct
    {
        bool     enabled         : 1;
        bool     online_capable  : 1;
        uint32_t                 : 30;
    } __pack;
    uint32_t align;
 } __pack apic_flags;
typedef union 
{
    struct 
    {
        uint8_t polarity        : 2;
        uint8_t trigger_mode    : 2;
        uint16_t                : 12;
    } __pack;
    uint32_t align;
} __pack nmi_flags;
struct local_apic_data
{
    uint8_t processor_uid;
    uint8_t apic_id;
    apic_flags flags;
} __pack;
struct io_apic_data
{
    uint8_t apic_id;
    uint8_t rsv;
    uint32_t io_apic_physical_address;
    uint32_t global_system_interrupt_base;
} __pack;
struct interrupt_src_override_data
{
    uint8_t bus;
    uint8_t src;
    uint32_t global_system_interrupt;
    nmi_flags flags;
} __pack;
struct nmi_source_override_data
{
    nmi_flags flags;
    uint32_t global_system_interrupt;
} __pack;
struct local_apic_nmi_data
{
    uint8_t processor_uid;
    nmi_flags flags;
    uint8_t local_apic_lint;
} __pack;
struct local_apic_addr_override
{
    uint16_t rsv;
    uint64_t local_apic_physical_addr;
} __pack;
struct io_sapic_data
{
    uint8_t apic_id;
    uint8_t rsv;
    uint32_t global_system_interrupt_base;
    uint64_t io_sapic_physical_addr;
} __pack;
struct local_sapic_data
{
    uint8_t processor_id;
    uint8_t sapic_id;
    uint8_t sapic_eid;
    uint8_t rsv[3];
    apic_flags flags;
    uint32_t processor_uid_value;
    char uid_string[];
} __pack;
struct platform_interrupt_source_data
{
    nmi_flags flags;
    uint8_t interrupt_type;
    uint8_t processor_id;
    uint8_t processor_eid;
    uint8_t io_sapic_vector;
    uint32_t global_system_interrupt;
    uint32_t isrc_flags;
} __pack;
struct local_x2apic_data
{
    uint16_t srv;
    uint32_t x2apic_id;
    apic_flags flags;
    uint32_t processor_uid;
} __pack;
#ifdef __cplusplus
}
template<typename T>
struct madt_record
{
    madt_record_header header;
    T data;
} __pack;
typedef madt_record<local_apic_data> local_apic;
typedef madt_record<io_apic_data> io_apic;
typedef madt_record<local_sapic_data> local_sapic;
typedef madt_record<io_sapic_data> io_sapic;
typedef madt_record<interrupt_src_override_data> isrc_override;
typedef madt_record<nmi_source_override_data> nmi_override;
typedef madt_record<local_apic_addr_override> apic_override;
typedef madt_record<local_apic_nmi_data> lapic_nmi;
#endif
typedef union __may_alias __idx_addr
{
    addr_t idx;
    uintptr_t addr;
} __pack indexed_address;
typedef union __may_alias __guid
{
    struct
    {
        uint32_t data_a;
        uint16_t data_b;
        uint16_t data_c;
        uint8_t data_d[8];
    }__pack;
    uint64_t data_full[2];
    uint8_t data_bytes[16];
} __pack __align(1) guid_t;
typedef struct __mmap_entry
{
    uintptr_t addr;     // Physical start address
    uint32_t len;       // Length in pages
    memtype_t type;     // Type
} __pack mmap_entry;
typedef struct __system_info 
{
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint32_t* fb_ptr;
    struct xsdt_t* xsdt;
} __pack sysinfo_t;
typedef struct __mmap
{
    size_t total_memory;
    size_t num_entries;
    mmap_entry entries[];
} __pack mmap_t;
typedef void (attribute(sysv_abi) *kernel_entry_fn) (sysinfo_t*, mmap_t*);
#ifdef __cplusplus
typedef struct __byte
{
	bool b0 : 1;
	bool b1 : 1;
	bool b2 : 1;
	bool b3 : 1;
	bool b4 : 1;
	bool b5 : 1;
	bool b6 : 1;
	bool b7 : 1;
    constexpr __byte(bool v0, bool v1, bool v2, bool v3, bool v4, bool v5, bool v6, bool v7) noexcept : b0{ v0 }, b1{ v1 }, b2{ v2 }, b3{ v3 }, b4{ v4 }, b5{ v5 }, b6{ v6 }, b7{ v7 } {}
	constexpr __byte(uint8_t i) noexcept :
		b0 	{ (i & 0x01) != 0 },
		b1 	{ (i & 0x02) != 0 },
		b2 	{ (i & 0x04) != 0 },
		b3 	{ (i & 0x08) != 0 },
		b4 	{ (i & 0x10) != 0 },
		b5 	{ (i & 0x20) != 0 },
		b6 	{ (i & 0x40) != 0 },
		b7 	{ (i & 0x80) != 0 }
			{}
    template<std::convertible_to<uint8_t> IT> requires (!std::is_same_v<IT, uint8_t>) constexpr __byte(IT it) noexcept : __byte{ uint8_t(it) } {}
	constexpr __byte() noexcept = default;
    constexpr __byte(__byte const&) noexcept = default;
    constexpr __byte(__byte&&) noexcept = default;
    constexpr __byte& operator=(__byte const&) noexcept = default;
    constexpr __byte& operator=(__byte&&) noexcept = default;
    constexpr ~__byte() noexcept = default;
    constexpr volatile __byte& operator=(__byte const& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this; }
    constexpr volatile __byte& operator=(__byte&& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this;  }
    constexpr operator uint8_t() const noexcept { return uint8_t((b0 ? 0x01u : 0) | (b1 ? 0x02u : 0) | (b2 ? 0x04u : 0) | (b3 ? 0x08u : 0) | (b4 ? 0x10u : 0) | (b5 ? 0x20u : 0) | (b6 ? 0x40u : 0) | (b7 ? 0x80u : 0)); }
    constexpr bool operator[](uint8_t i) const noexcept { return i == 0 ? b0  : (i == 1 ? b1  : (i == 2 ? b2  : (i == 3 ? b3  : (i == 4 ? b4  : (i == 5 ? b5  : (i == 6 ? b6  : (i == 7 ? b7  : false))))))); }
    constexpr __byte& operator|=(__byte const& that) noexcept { return *this = (*this | that); }
    constexpr __byte& operator&=(__byte const& that) noexcept { return *this = (*this & that); }
    constexpr __byte& operator+=(__byte const& that) noexcept { return *this = (*this + that); }
    constexpr __byte& operator-=(__byte const& that) noexcept { return *this = (*this - that); }
    constexpr __byte& operator*=(__byte const& that) noexcept { return *this = (*this * that); }
    constexpr __byte& operator/=(__byte const& that) noexcept { return *this = (*this / that); }
    constexpr __byte& operator>>=(int that) noexcept { return *this = (*this >> that); }
    constexpr __byte& operator<<=(int that) noexcept { return *this = (*this << that); }
    constexpr void set(uint8_t i) volatile noexcept { __byte that{ *const_cast<__byte*>(this) }; that |= (1 << i); __atomic_store(this, &that, __ATOMIC_SEQ_CST); }
    constexpr void clear(uint8_t i) volatile noexcept { __byte that{ *const_cast<__byte*>(this) }; that &= ~(1 << i); __atomic_store(this, &that, __ATOMIC_SEQ_CST); }
    constexpr bool get(uint8_t i) volatile noexcept { return (__atomic_load_n(std::bit_cast<const uint8_t*>(this), __ATOMIC_SEQ_CST) & (1 << i)) != 0; }
} __pack byte;
typedef struct __word
{ 
    byte lo;
    byte hi;
    constexpr __word() noexcept = default;
    constexpr __word(byte l, byte h) noexcept : lo{ l }, hi{ h } {}
    constexpr __word(uint16_t value) noexcept : lo{ byte(value & 0x00FFu) }, hi{ byte((value & 0xFF00) >> 8) } {}
    template<std::convertible_to<uint16_t> IT> requires (!std::is_same_v<IT, uint16_t>) constexpr __word(IT it) noexcept : __word{ uint16_t(it) } {}
    constexpr __word(__word const&) noexcept = default;
    constexpr __word(__word&&) noexcept = default;
    constexpr __word& operator=(__word const&) noexcept = default;
    constexpr __word& operator=(__word&&) noexcept = default;
    constexpr ~__word() noexcept = default;
    constexpr volatile __word& operator=(__word const& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this; }
    constexpr volatile __word& operator=(__word&& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this;  }
    constexpr operator uint16_t() const noexcept { return (uint16_t(hi) << 8) | lo; }    
    constexpr __word& operator|=(__word const& that) noexcept { return *this = (*this | that); }
    constexpr __word& operator&=(__word const& that) noexcept { return *this = (*this & that); }
    constexpr __word& operator+=(__word const& that) noexcept { return *this = (*this + that); }
    constexpr __word& operator-=(__word const& that) noexcept { return *this = (*this - that); }
    constexpr __word& operator*=(__word const& that) noexcept { return *this = (*this * that); }
    constexpr __word& operator/=(__word const& that) noexcept { return *this = (*this / that); }
    constexpr __word& operator>>=(int that) noexcept { return *this = (*this >> that); }
    constexpr __word& operator<<=(int that) noexcept { return *this = (*this << that); }
    constexpr bool operator[](uint8_t i) const noexcept { return (i >= 8 ? hi : lo)[i % 8]; }
} __pack word;
typedef struct __dword
{
    word lo;
    word hi;
    constexpr __dword() noexcept = default;
    constexpr __dword(word l, word h) noexcept : lo{ l }, hi{ h } {}
    constexpr __dword(uint32_t value) noexcept : lo{ uint16_t(value & 0x0000FFFFu) }, hi{ uint16_t((value & 0xFFFF0000) >> 16) } {}
    template<std::convertible_to<uint32_t> IT> requires (!std::is_same_v<IT, uint32_t>) constexpr __dword(IT it) noexcept : __dword{ uint32_t(it) } {}
    constexpr __dword(__dword const&) noexcept = default;
    constexpr __dword(__dword&&) noexcept = default;
    constexpr __dword& operator=(__dword const&) noexcept = default;
    constexpr __dword& operator=(__dword&&) noexcept = default;
    constexpr ~__dword() noexcept = default;
    constexpr volatile __dword& operator=(__dword const& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this; }
    constexpr volatile __dword& operator=(__dword&& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this;  }
    constexpr operator uint32_t() const noexcept { return uint32_t(uint16_t(lo) | (uint32_t(uint16_t(hi)) << 16)); }
    constexpr __dword& operator|=(__dword const& that) noexcept { return *this = (*this | that); }
    constexpr __dword& operator&=(__dword const& that) noexcept { return *this = (*this & that); }
    constexpr __dword& operator+=(__dword const& that) noexcept { return *this = (*this + that); }
    constexpr __dword& operator-=(__dword const& that) noexcept { return *this = (*this - that); }
    constexpr __dword& operator*=(__dword const& that) noexcept { return *this = (*this * that); }
    constexpr __dword& operator/=(__dword const& that) noexcept { return *this = (*this / that); }
    constexpr __dword& operator>>=(int that) noexcept { return *this = (*this >> that); }
    constexpr __dword& operator<<=(int that) noexcept { return *this = (*this << that); }
    constexpr bool operator[](uint8_t i) const noexcept { return (i >= 16 ? hi : lo)[i % 16]; }
}__pack dword;
typedef struct __qword
{
    dword lo;
    dword hi;
    constexpr __qword() noexcept = default;
    constexpr __qword(dword l, dword h) noexcept : lo{ l }, hi{ h } {}
    constexpr __qword(uint64_t value) noexcept : lo{ uint32_t(value & 0x00000000FFFFFFFFu) }, hi{ uint32_t((value & 0xFFFFFFFF00000000u) >> 32) } {}
    template<std::convertible_to<uint64_t> IT> requires (!std::is_same_v<IT, uint64_t>) constexpr __qword(IT it) noexcept : __qword{ uint64_t(it) } {}
    constexpr __qword(__qword const&) noexcept = default;
    constexpr __qword(__qword&&) noexcept = default;
    constexpr __qword& operator=(__qword const&) noexcept = default;
    constexpr __qword& operator=(__qword&&) noexcept = default;
    constexpr ~__qword() noexcept = default;
    constexpr volatile __qword& operator=(__qword const& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this; }
    constexpr volatile __qword& operator=(__qword&& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this; }
    constexpr operator uint64_t() const noexcept { return uint64_t(uint32_t(lo) | (uint64_t(uint32_t(hi)) << 32)); }
    constexpr __qword& operator|=(__qword const& that) noexcept { return *this = (*this | that); }
    constexpr __qword& operator&=(__qword const& that) noexcept { return *this = (*this & that); }
    constexpr __qword& operator+=(__qword const& that) noexcept { return *this = (*this + that); }
    constexpr __qword& operator-=(__qword const& that) noexcept { return *this = (*this - that); }
    constexpr __qword& operator*=(__qword const& that) noexcept { return *this = (*this * that); }
    constexpr __qword& operator/=(__qword const& that) noexcept { return *this = (*this / that); }
    constexpr __qword& operator>>=(int that) noexcept { return *this = (*this >> that); }
    constexpr __qword& operator<<=(int that) noexcept { return *this = (*this << that); }
    constexpr bool operator[](uint8_t i) const noexcept { return (i >= 32 ? hi : lo)[i % 32]; }
} __pack qword;
/**
 * These structures are for use with drivers that require working with numbers in big endian (amd64 is little endian).
 * The region pragma is for making it easier to use preprocessor directives in case I ever decide to add another supported architecture (and that arch uses big endian).
 * For instance, ext4's journaling structs are in big endian. Since the struct is read from disk as a 1024-byte block, the constructors that convert from little endian will not be called at that time.
 * When accessing those members as numbers, the conversions will be applied such that assigning (for instance) an int to a __be32 will automatically be converted to big endian as the writes occur in memory,
 * and comparing a a __be64 to an unsigned long will convert the __be64 to little endian before the comparison occurs (assuming this works the way I intend, which can be a longshot sometimes)
 */
#pragma region big-endian structs
typedef struct __s_be16
{
    uint8_t hi;
    uint8_t lo;
    constexpr explicit __s_be16(uint16_t i) noexcept : hi{ uint8_t((i >> 8) & 0xFF) }, lo{ uint8_t(i & 0xFF) } {}
    constexpr __s_be16() noexcept = default;
    constexpr __s_be16(__s_be16&&) noexcept = default;
    constexpr __s_be16(__s_be16 const&) noexcept = default;
    constexpr ~__s_be16() noexcept = default;
    constexpr operator uint16_t() const noexcept { return word(lo, hi); }
    constexpr __s_be16& operator=(__s_be16 const&) noexcept = default;
    constexpr __s_be16& operator=(__s_be16&&) noexcept = default;
    constexpr __s_be16& operator=(uint16_t i) noexcept { return (*this = __s_be16(i)); }
} __pack __be16;
typedef struct __s_be32
{
    __be16 hi;
    __be16 lo;
    constexpr explicit __s_be32(uint32_t i) noexcept : hi{ uint16_t((i >> 16) & 0xFFFF) }, lo{ uint16_t(i & 0xFFFF) } {}
    constexpr __s_be32() noexcept = default;
    constexpr __s_be32(__s_be32&&) noexcept = default;
    constexpr __s_be32(__s_be32 const&) noexcept = default;
    constexpr ~__s_be32() noexcept = default;
    constexpr operator uint32_t() const noexcept { return dword(lo, hi); }
    constexpr __s_be32& operator=(__s_be32&&) noexcept = default;
    constexpr __s_be32& operator=(__s_be32 const&) noexcept = default;
    constexpr __s_be32& operator=(uint32_t i) noexcept { return (*this = __s_be32(i)); }
} __pack __be32;
typedef struct __s_be64
{
    __be32 hi;
    __be32 lo;
    constexpr explicit __s_be64(uint64_t i) noexcept : hi{ uint32_t((i >> 32) & 0xFFFFFFFF) }, lo{ uint32_t(i & 0xFFFFFFFF) } {}
    constexpr __s_be64() noexcept = default;
    constexpr __s_be64(__s_be64&&) noexcept = default;
    constexpr __s_be64(__s_be64 const&) noexcept = default;
    constexpr ~__s_be64() noexcept = default;
    constexpr operator uint64_t() const noexcept { return qword(lo, hi); }
    constexpr __s_be64& operator=(__s_be64&&) noexcept = default;
    constexpr __s_be64& operator=(__s_be64 const&) noexcept = default;
    constexpr __s_be64& operator=(uint64_t i) noexcept { return (*this = __s_be64(i)); }
} __pack __be64;

constexpr __be16 operator""_be16(unsigned long long i) noexcept { return __be16(uint16_t(i)); }
constexpr __be32 operator""_be32(unsigned long long i) noexcept { return __be32(uint32_t(i)); }
constexpr __be64 operator""_be64(unsigned long long i) noexcept { return __be64(uint64_t(i)); }
#pragma endregion
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
constexpr byte operator""ui8(unsigned long long i) noexcept { return byte{ uint8_t(i) }; }
constexpr word operator""ui16(unsigned long long i) noexcept { return word{ uint16_t(i) }; }
constexpr dword operator""ui32(unsigned long long i) noexcept { return dword{ uint32_t(i) }; }
#pragma GCC diagnostic pop
#else
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
typedef uint16_t __be16;
typedef uint32_t __be32;
typedef uint64_t __be64;
#endif
#define BIT(n) (1 << n)
#endif