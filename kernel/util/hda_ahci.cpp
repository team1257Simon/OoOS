#include "fs/hda_ahci.hpp"
#include "stdexcept"
bool ahci_hda::__has_init = false;
ahci_hda ahci_hda::__instance{};
bool ahci_hda::__await_disk() { for(size_t i = 0; !__drv->is_done(__port); __sync_synchronize()) { BARRIER; i++; } return __drv->is_done(__port); }
bool ahci_hda::__read_ahci(qword st, dword ct, uint16_t* bf) { try { __drv->read_sectors(__port, st, ct, bf); return __await_disk(); } catch(std::exception& e) { panic(e.what()); return false; } }
bool ahci_hda::__write_ahci(qword st, dword ct, uint16_t const* bf) { try { __drv->write_sectors(__port, st, ct, bf); return __await_disk(); } catch(std::exception& e) { panic(e.what()); return false; } }
bool ahci_hda::__read_pt()
{
    std::allocator<pt_header_t> alloc_hdr{};
    std::allocator<partition_entry_t> alloc_pt{};
    pt_header_t* hdr;
    try { hdr = alloc_hdr.allocate(1); }
    catch(std::bad_alloc&) { panic("no heap available"); return false; }
    if(!read_objects(hdr, 1ul, 1u)) { panic("bad read on header"); alloc_hdr.deallocate(hdr, 1); return false; }
    unsigned sz_multi = hdr->part_entry_size / sizeof(partition_entry_t);
    if(!sz_multi) { panic("invalid size for pt entries; is the GPT header corrupted?"); alloc_hdr.deallocate(hdr, 1); return false; }
    size_t n = hdr->num_part_entries * sz_multi;
    partition_entry_t* arr = alloc_pt.allocate(n);
    array_zero<partition_entry_t>(arr, n);
    if(!read_objects(arr, hdr->lba_partition_entry_array, n)) { panic("bad read on plus-size partition table"); alloc_hdr.deallocate(hdr, 1); alloc_pt.deallocate(arr, n); return false; }
    __my_partitions.reserve(size_t(hdr->num_part_entries));
    for(size_t i = 0; i < n; i += sz_multi) __my_partitions.push_back(arr[i]);
    alloc_hdr.deallocate(hdr, 1);
    alloc_pt.deallocate(arr, n);
    return true;
}
ahci_hda::ahci_hda() : __read_buffer{ __bytes_per_sector() * 4 }, __write_buffer{ __bytes_per_sector() * 4 } {}
bool ahci_hda::init_instance() { if(__has_init) return true; return (__has_init = __instance.init()); }
bool ahci_hda::is_initialized() noexcept { return __has_init; }
partition_table &ahci_hda::get_partition_table() { return __instance.__my_partitions; }
ahci_hda *ahci_hda::get_instance() { return __has_init ? &__instance : nullptr; }
bool ahci_hda::init()
{
   if(!ahci_driver::is_initialized()) { panic("no AHCI driver available"); return false; }
   __drv = ahci_driver::get_instance();
   __port = __drv->which_port(sata);
   if(__port < 0) return false;
   return __read_pt();
}
std::streamsize ahci_hda::read(char* out, uint64_t start_sector, uint32_t count)
{
    if(!__instance.__drv) { panic("cannot read disk before initializing read accessor"); return 0; } 
    __instance.__read_buffer.clear();
    size_t n = __count_to_wide_streamsize(count);
    if(!__instance.__read_buffer.__ensure_capacity(n)) { panic("failed to get buffer space"); }
    if(!__instance.__read_ahci(start_sector, count, __instance.__read_buffer.beg())) { panic("bad read"); return 0; }
    __instance.__read_buffer.__update_end(n);
    __builtin_memcpy(out, __instance.__read_buffer.beg(), n * 2);
    return n * 2;
}
std::streamsize ahci_hda::write(uint64_t start_sector, const char *in, uint32_t count)
{
    if(!__instance.__drv) { panic("cannot write disk before initializing write accessor"); return 0; }
    __instance.__write_buffer.clear();
    std::streamsize result = __instance.__write_buffer.sputn(reinterpret_cast<uint16_t const*>(in), __count_to_wide_streamsize(count)) * 2;
    if(!__instance.__write_ahci(start_sector, count, __instance.__write_buffer.beg())) { panic("bad write"); return 0; }
    return result;
}