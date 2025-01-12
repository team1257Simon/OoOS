#ifndef __AHCI_HARD_DISK_ACCESS
#define __AHCI_HARD_DISK_ACCESS
#include "arch/ahci.hpp"
#include "kernel/libk_decls.h"
#include "fs/generic_binary_buffer.hpp"
#include "functional"
#include "vector"
constexpr size_t start_lba_field_offset = 0x8; // Offset, in bytes, of the field pointing to the LBA of the partition table header (should be 1)
typedef struct __pt_header
{
    char sig[8];
    uint32_t revision;
    uint32_t header_size;
    char checksum[4];
    char rsv0[4];
    uint64_t lba_header;
    uint64_t lba_alternate;
    uint64_t lba_first_usable;
    uint64_t lba_last_usable;
    guid_t guid;
    uint64_t lba_partition_entry_array;
    uint32_t num_part_entries;
    uint32_t part_entry_size; // will be a multiple of 128
    uint32_t partition_array_crc32_checksum;
    char rsv1[physical_block_size - 0x5Cu];
} __pack pt_header_t;
typedef struct __part_table_entry
{
    guid_t type_guid;
    guid_t part_guid;
    uint64_t start_lba;
    uint64_t end_lba;
    uint64_t attributes;
    char16_t part_name[36];
} __pack partition_entry_t;
typedef std::vector<partition_entry_t> partition_table;
class ahci_hda
{
    static bool __has_init;
    static ahci_hda __instance;
    wide_binary_buffer __read_buffer;
    wide_binary_buffer __write_buffer;
    ahci_driver* __drv;
    int8_t __port;
    constexpr static std::size_t __bytes_per_sector() { return physical_block_size; }
    constexpr static uint64_t __offset_to_sector(uint64_t offs) { return offs / __bytes_per_sector(); }
    bool __await_disk();
    bool __read_ahci(qword st, dword ct, uint16_t* bf);
    bool __write_ahci(qword st, dword ct, uint16_t const* bf);
protected:
    ahci_hda();
    bool init();
public:
    static bool init_instance();
    static bool is_initialized() noexcept;
    static ahci_hda* get_instance();
    static std::streamsize read(char* out, uint64_t start_sector, uint32_t count);
    static std::streamsize write(uint64_t start_sector, const char* in, uint32_t count);
    static partition_table read_pt();
protected:
    template<trivial_copy T> static size_t obj_read(T* out, uint64_t start_sector, uint32_t num_objs) { return read(std::bit_cast<char*>(out), start_sector, num_objs * sizeof(T) / __bytes_per_sector()); }
};
#endif