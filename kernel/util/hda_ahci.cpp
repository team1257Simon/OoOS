#include "fs/hda_ahci.hpp"
#include "stdexcept"
#include "kdebug.hpp"
bool hda_ahci::__has_init = false;
hda_ahci hda_ahci::__instance{};
bool hda_ahci::__await_disk() { for(size_t i = 0; !__drv->is_done(__port); __sync_synchronize()) { BARRIER; i++; } return __drv->is_done(__port); }
bool hda_ahci::__read_ahci(qword st, dword ct, uint16_t* bf) { try { __drv->read_sectors(__port, st, ct, bf); return __await_disk(); } catch(std::exception& e) { panic(e.what()); return false; } }
bool hda_ahci::__write_ahci(qword st, dword ct, uint16_t const* bf) { try { __drv->write_sectors(__port, st, ct, bf); return __await_disk(); } catch(std::exception& e) { panic(e.what()); return false; } }
hda_ahci::hda_ahci() {}
bool hda_ahci::init_instance() { if(__has_init) return true; return (__has_init = __instance.init()); }
bool hda_ahci::is_initialized() noexcept { return __has_init; }
partition_table& hda_ahci::get_partition_table() { return __instance.__my_partitions; }
hda_ahci *hda_ahci::get_instance() { return __has_init ? std::addressof(__instance) : nullptr; }
bool hda_ahci::__read_pt()
{
    std::allocator<pt_header_t> alloc_hdr{};
    std::allocator<partition_entry_t> alloc_pt{};
    pt_header_t* hdr;
    try { hdr = alloc_hdr.allocate(1); }
    catch(std::bad_alloc&) { panic("no heap available"); return false; }
    if(!read(reinterpret_cast<char*>(hdr), 1U, 1U)) { panic("bad read on header"); alloc_hdr.deallocate(hdr, 1); return false; }
    unsigned sz_multi = hdr->part_entry_size / sizeof(partition_entry_t);
    if(!sz_multi) { panic("invalid size for pt entries; is the GPT header corrupted?"); alloc_hdr.deallocate(hdr, 1); return false; }
    size_t n = hdr->num_part_entries * sz_multi;
    size_t actual = div_roundup(div_roundup(n * sizeof(partition_entry_t), physical_block_size) * physical_block_size, sizeof(partition_entry_t));
    partition_entry_t* arr = alloc_pt.allocate(actual);
    array_zero<partition_entry_t>(arr, n);
    if(!read(reinterpret_cast<char*>(arr), hdr->lba_partition_entry_array, div_roundup(n * sizeof(partition_entry_t), physical_block_size))) { panic("bad read on plus-size partition table"); alloc_hdr.deallocate(hdr, 1); alloc_pt.deallocate(arr, actual); return false; }
    for(size_t i = 0; i < n; i += sz_multi) __my_partitions.push_back(arr[i]);
    alloc_hdr.deallocate(hdr, 1);
    alloc_pt.deallocate(arr, actual);
    return true;
}
bool hda_ahci::init()
{
   if(!ahci::is_initialized()) { panic("no AHCI driver available"); return false; }
   __drv = ahci::get_instance();
   __port = __drv->which_port(sata);
   if(__port < 0) return false;
   return __read_pt();
}
bool hda_ahci::write(uint64_t start_sector, const void* in, uint32_t count)
{
    if(!__instance.__drv) { panic("cannot write disk before initializing write accessor"); return false; }
    uint16_t const* src = static_cast<uint16_t const*>(in);
    size_t t_write = 0;
    size_t s_write = 0;
    size_t rem = count;
    while(rem)
    {
        size_t sct = std::min(rem, max_op_sectors);
        if(!__instance.__write_ahci(start_sector + s_write, sct, src + t_write)) { panic("bad write"); return false; }
        t_write += __count_to_wide_streamsize(sct);
        s_write += sct;
        rem -= sct;
    }
    FENCE();
    return true;
}
bool hda_ahci::read(void* out, uint64_t start_sector, uint32_t count)
{
    if(!__instance.__drv) { panic("cannot read disk before initializing read accessor"); return false; }
    uint16_t* target = static_cast<uint16_t*>(out);
    size_t rem = count;
    size_t t_read = 0;
    size_t s_read = 0;
    FENCE();
    while(rem)
    {
        size_t sct = std::min(rem, max_op_sectors);
        if(!__instance.__read_ahci(start_sector + s_read, sct, target + t_read)) { panic("bad read"); return false; }
        t_read += __count_to_wide_streamsize(sct);
        s_read += sct;
        rem -= sct;
    }
    FENCE();
    return true;
}