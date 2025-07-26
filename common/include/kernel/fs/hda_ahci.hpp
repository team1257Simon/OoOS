#ifndef __AHCI_HARD_DISK_ACCESS
#define __AHCI_HARD_DISK_ACCESS
#include "kernel/arch/ahci.hpp"
#include "kernel/libk_decls.h"
#include "ext/dynamic_streambuf.hpp"
#include "fs/block_device.hpp"
constexpr size_t start_lba_field_offset = 0x8; // Offset, in bytes, of the field pointing to the LBA of the partition table header (which is usually LBA 1)
constexpr size_t max_op_sectors = (prdt_entries_count * 16);
class hda_ahci : public partitioned_block_device
{
    static bool __has_init;
    static hda_ahci __instance;
    ahci* __driver                  {};
    int8_t __port                   {};
    partition_table __part_table    {};
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
    virtual bool read(void* out, uint64_t start_sector, uint32_t count) override;
    virtual bool write(uint64_t start_sector, const void* in, uint32_t count) override;
    virtual size_t sector_size() const override;
    virtual partition_table& get_partition_table() override;
};
#endif