#ifndef __KERNEL_DEF
#define __KERNEL_DEF
#include "stdint.h"
#include "stddef.h"
#ifndef KERNEL_FILENAME
#define KERNEL_FILENAME "\\sys\\core.elf"
#endif
#define PAGESIZE 0x1000
#define PT_LEN 0x200
#define MMAP_MAX_PG 0x100000uL
#define HAVE_SIZE_T 1
#define HAVE_STDINT 1
#define __pack __attribute__((packed))
#define __align(n) __attribute__((aligned(n)))
#ifndef __cplusplus
typedef unsigned char bool;
#else
#define restrict
#include "concepts"
#include "bits/move.h"
template<class T> concept NotVoidPointer = !std::same_as<std::remove_cvref_t<T>, void*>;
template<class T> concept non_void = !std::is_void_v<T>;
#endif
#define PAUSE asm volatile ("pause" ::: "memory")
#define BARRIER asm volatile ("" ::: "memory")
typedef enum mme_type
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
    uint8_t                      : 4;
    uint64_t physical_address    : 36;
    uint16_t                     : 15;
    bool execute_disable         : 1;
} __pack __align(1) pt_entry;
typedef pt_entry* paging_table;
typedef struct __vaddr
{
   #ifdef __cplusplus 
    uint16_t offset     : 12 {0};
    uint16_t page_idx   :  9 {0};
    uint16_t pd_idx     :  9 {0};
    uint16_t pdp_idx    :  9 {0};
    uint16_t pml4_idx   :  9 {0};
    uint16_t ext        : 16 {0};
    constexpr __vaddr(uint16_t offs, uint16_t idx0, uint16_t idx1, uint16_t idx2, uint16_t idx3, uint16_t sign) noexcept :
        offset      { offs },
        page_idx    { idx0 },
        pd_idx      { idx1 },
        pdp_idx     { idx2 },
        pml4_idx    { idx3 },
        ext         { sign } 
                    {}
    constexpr __vaddr(unsigned long i) noexcept : 
        offset      { static_cast<uint16_t>(i & 0x0FFF) },
        page_idx    { static_cast<uint16_t>(uint64_t(i >> 12) & 0x1FFuL) }, 
        pd_idx      { static_cast<uint16_t>(uint64_t(i >> 21) & 0x1FFuL) },
        pdp_idx     { static_cast<uint16_t>(uint64_t(i >> 30) & 0x1FFuL) },
        pml4_idx    { static_cast<uint16_t>(uint64_t(i >> 39) & 0x1FFuL) },
        ext         { static_cast<uint16_t>(pml4_idx & 0x100 ? 0xFFFFu : 0u) } 
                    {}    
    constexpr __vaddr(void* ptr) noexcept : __vaddr { std::bit_cast<uintptr_t>(ptr) } {}
    constexpr __vaddr() = default;
    constexpr ~__vaddr() = default;
    constexpr __vaddr(__vaddr const&) = default;
    constexpr __vaddr(__vaddr &&) = default;
    constexpr __vaddr& operator=(__vaddr const&) = default;
    constexpr __vaddr& operator=(__vaddr &&) = default;
    constexpr __vaddr operator+(ptrdiff_t value) const { return { std::bit_cast<uintptr_t>(*this) + value }; }
    constexpr __vaddr& operator+=(ptrdiff_t value) { return *this = (*this + value); }
    constexpr __vaddr operator%(uint64_t unit) const { return { std::bit_cast<uintptr_t>(*this) % unit }; }
    constexpr __vaddr& operator%=(uint64_t unit) { return *this = (*this % unit); }
    constexpr __vaddr operator-(ptrdiff_t value) const { return { std::bit_cast<uintptr_t>(*this) - value }; }
    constexpr __vaddr& operator-=(ptrdiff_t value) { return *this = (*this - value); }
    constexpr operator void*() const noexcept { void* ptr = std::bit_cast<void*>(operator uintptr_t()); return ptr ? ptr : nullptr; }
    constexpr operator const void*() const noexcept { const void* ptr = std::bit_cast<const void*>(operator uintptr_t()); return ptr ? ptr : nullptr; }
    constexpr operator volatile void*() const volatile noexcept { volatile void* ptr = std::bit_cast<volatile void*>((const_cast<__vaddr*>(this))->operator uintptr_t()); return ptr ? ptr : nullptr; }
    constexpr operator const volatile void*() const volatile noexcept { const volatile void* ptr = std::bit_cast<const volatile void*>((const_cast<__vaddr*>(this))->operator uintptr_t()); return ptr ? ptr : nullptr;  }
    template<non_void T> constexpr operator T*() const noexcept { return std::bit_cast<std::remove_cv_t<T>*>(operator void*()); }
    template<non_void T> constexpr operator const T*() const noexcept { return std::bit_cast<const std::remove_cv_t<T>*>(operator const void*()); }
    template<non_void T> constexpr operator volatile T*() const volatile noexcept { return std::bit_cast<volatile std::remove_cv_t<T>*>(operator volatile void*()); }
    template<non_void T> constexpr operator const volatile T*() const volatile noexcept { return std::bit_cast<const volatile std::remove_cv_t<T>*>(operator const volatile void*()); }
    constexpr operator uintptr_t() const noexcept
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
    constexpr operator bool() const noexcept { return static_cast<const void*>(*this) != nullptr; }
    constexpr bool operator!() const noexcept { return static_cast<const void*>(*this) == nullptr; }
#else
    uint16_t offset     : 12;
    uint16_t page_idx   :  9;
    uint16_t pd_idx     :  9;
    uint16_t pdp_idx    :  9;
    uint16_t pml4_idx   :  9;
    uint16_t ext        : 16;
#endif
} __pack __align(1) vaddr_t;
typedef union __idx_addr
{
    vaddr_t idx;
    uintptr_t addr;
} __pack indexed_address;
typedef union __guid
{
    struct
    {
        uint32_t data_a;
        uint16_t data_b;
        uint16_t data_c;
        uint8_t data_d[8];
    }__pack;
    uint64_t data_full[2];
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
} __pack sysinfo_t;
typedef struct __mmap
{
    size_t total_memory;
    size_t num_entries;
    mmap_entry entries[];
} __pack mmap_t;
// Pointers to page frames.
typedef struct __pageframe_t
{
    indexed_address start_idx;
    size_t num_tables;
    paging_table cr3;
    paging_table tables[];
} __pack page_frame;
typedef struct __pagefile
{
    size_t num_entries;
    page_frame* boot_entry;     // Identity-paged memory mapped by the bootloader
    page_frame* frame_entries;
} __pack pagefile;
typedef void(__attribute__((sysv_abi)) *kernel_entry_fn)(sysinfo_t*, mmap_t*, pagefile*);
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
#endif