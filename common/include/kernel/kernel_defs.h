#ifndef __KERNEL_DEF
#define __KERNEL_DEF
#include "stdint.h"
#include "stddef.h"
#define attribute(x) __attribute__(x)
#ifndef KERNEL_FILENAME
#define KERNEL_FILENAME "\\sys\\core.elf"
#endif
#define PAGESIZE 0x1000
#define PT_LEN 0x200
#define MMAP_MAX_PG 0x100000uL
#define HAVE_SIZE_T 1
#define HAVE_STDINT 1
#define __pack attribute((packed))
#define __align(n) attribute((aligned(n)))
#define __isrcall [[gnu::target("general-regs-only")]]
#ifndef __cplusplus
#ifdef NEED_STDBOOL
#include "stdbool.h"
#endif
#define physical_block_size 512uL
#else
constexpr size_t physical_block_size = 512;
#define restrict
#include "concepts"
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
    bool avl0                    : 1;
    bool avl1                    : 1;
    bool avl2                    : 1;
    bool avl3                    : 1;
    uint64_t physical_address    : 36;
    uint16_t avl4                : 15;
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
    constexpr __vaddr operator+(ptrdiff_t value) const { return { uintptr_t(*this) + value }; }
    constexpr __vaddr& operator+=(ptrdiff_t value) { return *this = (*this + value); }
    constexpr __vaddr operator%(uint64_t unit) const { return { uintptr_t(*this) % unit }; }
    constexpr __vaddr& operator%=(uint64_t unit) { return *this = (*this % unit); }
    constexpr __vaddr operator-(ptrdiff_t value) const { return { uintptr_t(*this) - value }; }
    constexpr __vaddr& operator-=(ptrdiff_t value) { return *this = (*this - value); }
    constexpr operator void*() const noexcept { void* ptr = std::bit_cast<void*>(uintptr_t(*this)); return ptr ? ptr : nullptr; }
    constexpr operator const void*() const noexcept { const void* ptr = std::bit_cast<const void*>(uintptr_t(*this)); return ptr ? ptr : nullptr; }
    constexpr operator volatile void*() const volatile noexcept { volatile void* ptr = std::bit_cast<volatile void*>(uintptr_t(const_cast<__vaddr*>(this))); return ptr ? ptr : nullptr; }
    constexpr operator const volatile void*() const volatile noexcept { const volatile void* ptr = std::bit_cast<const volatile void*>(uintptr_t(const_cast<__vaddr*>(this))); return ptr ? ptr : nullptr;  }
    template<non_void T> constexpr operator T*() const noexcept { return std::bit_cast<std::remove_cv_t<T>*>(uintptr_t(*this)); }
    template<non_void T> constexpr operator const T*() const noexcept { return std::bit_cast<const std::remove_cv_t<T>*>(uintptr_t(*this)); }
    template<non_void T> constexpr operator volatile T*() const volatile noexcept { typedef volatile void* vvptr; return std::bit_cast<volatile std::remove_cv_t<T>*>(vvptr(*this)); }
    template<non_void T> constexpr operator const volatile T*() const volatile noexcept { typedef const volatile void* cvvptr; return std::bit_cast<const volatile std::remove_cv_t<T>*>(cvvptr(*this)); }
    constexpr operator bool() const noexcept { return uintptr_t(*this) != 0; }
    constexpr bool operator!() const noexcept { return uintptr_t(*this) == 0; }
#else
    uint16_t offset     : 12;
    uint16_t page_idx   :  9;
    uint16_t pd_idx     :  9;
    uint16_t pdp_idx    :  9;
    uint16_t pml4_idx   :  9;
    uint16_t ext        : 16;
#endif
} __pack __align(1) vaddr_t;
#ifdef __cplusplus
extern "C" {
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
    uint64_t                ext_firmware_control;
    struct dsdt*            ext_dsdt;
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
    struct xsdt_t* xsdt;
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
    paging_table cr3;         // the value in cr3 when the frame is active
    size_t num_saved_tables;  // size of the following array 
    paging_table tables[];    // paging table pointers for quick access
} __pack page_frame;
typedef struct __pagefile
{
    size_t num_entries;
    page_frame* boot_entry;     // Identity-paged memory mapped by the bootloader
    page_frame* frame_entries;
} __pack pagefile;
// The kernel's side of process-info for use with syscalls. While in ring 0 (i.e. in kernel code) the gs base register will point to this structure.
typedef struct __kpinfo
{
    uintptr_t self_ptr;   // %gs:0x00
    paging_table k_cr3;   // %gs:0x08
    uintptr_t k_rsp;      // %gs:0x10
    uintptr_t k_rbp;      // %gs:0x18
    uintptr_t rtn_rsp;    // %gs:0x20
    uintptr_t rtn_rbp;    // %gs:0x28
    uintptr_t rtn_rip;    // %gs:0x30
    paging_table rtn_cr3; // %gs:0x38
} __pack kpinfo_t;
typedef void (attribute((sysv_abi)) *kernel_entry_fn) (sysinfo_t*, mmap_t*, pagefile*);
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
    constexpr volatile __byte& operator=(__byte const& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this; }
    constexpr volatile __byte& operator=(__byte&& that) volatile noexcept { __atomic_store(this, &that, __ATOMIC_SEQ_CST); return *this;  }
    constexpr bool operator[](uint8_t i) const noexcept { return i == 0 ? b0  : (i == 1 ? b1  : (i == 2 ? b2  : (i == 3 ? b3  : (i == 4 ? b4  : (i == 5 ? b5  : (i == 6 ? b6  : (i == 7 ? b7  : false))))))); }
    constexpr operator uint8_t() const noexcept { return uint8_t((b0 ? 0x01u : 0) | (b1 ? 0x02u : 0) | (b2 ? 0x04u : 0) | (b3 ? 0x08u : 0) | (b4 ? 0x10u : 0) | (b5 ? 0x20u : 0) | (b6 ? 0x40u : 0) | (b7 ? 0x80u : 0)); }
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
constexpr byte operator""ui8(unsigned long long i) noexcept { return byte{ uint8_t(i) }; }
constexpr word operator""ui16(unsigned long long i) noexcept { return word{ uint16_t(i) }; }
#pragma GCC diagnostic pop
#else
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef uint64_t qword;
#endif
#define BIT(n) (1 << n)
#endif