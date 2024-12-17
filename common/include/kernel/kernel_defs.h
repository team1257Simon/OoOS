#ifndef __KERNEL_DEF
#define __KERNEL_DEF
#ifndef _GCC_STDINT_H
#include "stdint.h"
#endif
#include "stddef.h"
#ifndef KERNEL_FILENAME
#define KERNEL_FILENAME "\\sys\\core.elf"
#endif
#define HAVE_SIZE_T 1
#define HAVE_STDINT 1
#define __pack __attribute__((packed))
#define __align(n) __attribute__((aligned(n)))
#ifndef __cplusplus
typedef unsigned char bool;
#else
#define restrict
template<class T> struct __is_void_ptr { constexpr static bool value = false; };
template<> struct __is_void_ptr<void*> { constexpr static bool value = true; };
template<> struct __is_void_ptr<const void*> { constexpr static bool value = true; };
template<> struct __is_void_ptr<volatile void*> { constexpr static bool value = true; };
template<> struct __is_void_ptr<const volatile void*> { constexpr static bool value = true; };
template<class T> concept NotVoidPointer = !__is_void_ptr<T>::value;
#endif
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
typedef struct __vaddr48
{
    uint16_t offset     : 12;
    uint16_t page_idx   :  9;
    uint16_t pd_idx     :  9;
    uint16_t pdp_idx    :  9;
    uint16_t pml4_idx   :  9;
    uint16_t ext        : 16;
} __pack __align(1) vaddr48_t;
typedef union __vaddr
{
    vaddr48_t idx;
    uintptr_t addr;
} __pack vaddr_t;
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
typedef struct fb_info 
{
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t* ptr;
} __pack framebuf_t;
typedef struct __mmap
{
    size_t num_entries;
    mmap_entry entries[];
} __pack mmap_t;
// Pointers to sequential page tables.
typedef struct __pagefile_entry
{
    struct __pagefile_entry* prev;
    struct __pagefile_entry* next;
    vaddr_t start_idx;
    size_t num_tables;
    paging_table tables[];
} __pack pagefile_entry;
typedef struct __pagefile
{
    size_t num_entries;
    pagefile_entry* tail;
    pagefile_entry* head;
} __pack pagefile;
typedef void(__attribute__((sysv_abi)) *kernel_entry_fn)(framebuf_t*, mmap_t*, pagefile*);
#endif