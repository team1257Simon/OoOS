#ifndef __AHCI_HARD_DISK_ACCESS
#define __AHCI_HARD_DISK_ACCESS
#include "kernel/arch/ahci.hpp"
#include "kernel/libk_decls.h"
#include "ext/dynamic_streambuf.hpp"
#include "vector"
constexpr size_t start_lba_field_offset = 0x8; // Offset, in bytes, of the field pointing to the LBA of the partition table header (which is usually LBA 1)
constexpr size_t max_op_sectors = (prdt_entries_count * 16);
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
    char rsv1[physical_block_size - 0x5CU];
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
class hda_ahci
{
    static bool __has_init;
    static hda_ahci __instance;
    ahci* __driver{};
    int8_t __port{};
    partition_table __my_partitions{};
    constexpr static std::size_t __bytes_per_sector() noexcept { return physical_block_size; }
    constexpr static uint64_t __offset_to_sector(uint64_t offs) noexcept(physical_block_size != 0) { return offs / __bytes_per_sector(); }
    constexpr static std::size_t __count_to_wide_streamsize(std::size_t count) noexcept { return (count * __bytes_per_sector()) >> 1; }
    bool __await_disk();
    bool __read_ahci(qword st, dword ct, uint16_t* bf);
    bool __write_ahci(qword st, dword ct, uint16_t const* bf);
    bool __read_pt();
protected:
    hda_ahci();
    bool init();
public:
    static bool init_instance();
    static bool is_initialized() noexcept;
    static hda_ahci* get_instance();
    static bool read(void* out, uint64_t start_sector, uint32_t count);
    static bool write(uint64_t start_sector, const void* in, uint32_t count);
    static partition_table& get_partition_table();
    template<trivial_copy T> static size_t read_objects(T* out, uint64_t start_sector, uint32_t num_objs) { return read(std::bit_cast<char*>(out), start_sector, div_round_up(num_objs * sizeof(T), __bytes_per_sector())); }
    template<trivial_copy T> static bool read_object(T& out, uint64_t from) { return read_objects(std::addressof(out), from, 1U); }
};
#endif