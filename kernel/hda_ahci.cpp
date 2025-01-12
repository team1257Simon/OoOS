#include "fs/hda_ahci.hpp"
#include "stdexcept"
bool ahci_hda::__has_init = false;
ahci_hda ahci_hda::__instance{};
bool ahci_hda::__await_disk() { for(size_t i = 0; i < max_wait && !__drv->is_done(__port); __sync_synchronize()) { BARRIER; i++; } return __drv->is_done(__port); }
bool ahci_hda::__read_ahci(qword st, dword ct, uint16_t *bf) { try { __drv->read_sectors(__port, st, ct, bf); return __await_disk(); } catch(std::exception& e) { panic(e.what()); return false; } }
bool ahci_hda::__write_ahci(qword st, dword ct, uint16_t const *bf) { try { __drv->write_sectors(__port, st, ct, bf); return __await_disk(); } catch(std::exception& e) { panic(e.what()); return false; } }
ahci_hda::ahci_hda() : __read_buffer{ __bytes_per_sector() * 4 }, __write_buffer{ __bytes_per_sector() * 4 } {}
bool ahci_hda::init_instance() { if(__has_init) return true; return (__has_init = __instance.init()); }
bool ahci_hda::is_initialized() noexcept { return __has_init; }
bool ahci_hda::init()
{
   if(!ahci_driver::is_initialized()) { panic("no AHCI driver available"); return false; }
   __instance.__drv = ahci_driver::get_instance();
   __instance.__port = __instance.__drv->which_port(sata);
   if(__instance.__port < 0) return false;
   return true;
}
ahci_hda *ahci_hda::get_instance() { return __has_init ? &__instance : nullptr; }
std::streamsize ahci_hda::read(char* out, uint64_t start_sector, uint32_t count)
{
    if(!__instance.__drv) { panic("cannot read disk before initializing read accessor"); return 0; } 
    __instance.__read_buffer.clear();
    size_t n = count * __bytes_per_sector();
    if(!__instance.__read_buffer.__ensure_capacity(n / 2)) { panic("failed to get buffer space"); }
    if(!__instance.__read_ahci(start_sector, count, __instance.__read_buffer.beg())) { panic("bad read"); return 0; }
    __instance.__read_buffer.__update_end(n / 2);
    return __instance.__read_buffer.rgetn<char>(out, n); 
}
std::streamsize ahci_hda::write(uint64_t start_sector, const char *in, uint32_t count)
{
    __instance.__write_buffer.clear();
    std::streamsize result = __instance.__write_buffer.rputn<char>(in, count *  __bytes_per_sector());
    if(!__instance.__drv) { panic("cannot write disk before initializing write accessor"); return 0; }
    if(!__instance.__write_ahci(start_sector, count, __instance.__write_buffer.beg())) { panic("bad write"); return 0; }
    return result;
}
partition_table ahci_hda::read_pt()
{
    if(!__instance.__drv) { panic("cannot read disk before initializing read accessor"); return {}; }
    std::allocator<pt_header_t> alloc_hdr{};
    std::allocator<partition_entry_t> alloc_pt{};
    pt_header_t* hdr;
    try { hdr = alloc_hdr.allocate(1); }
    catch(std::bad_alloc&) { panic("no heap available"); alloc_hdr.deallocate(hdr, 1); return {}; }
    if(!obj_read(hdr, 1ul, 1u)) { panic("bad read on header"); alloc_hdr.deallocate(hdr, 1); return {}; }
    unsigned sz_multi = hdr->part_entry_size / sizeof(partition_entry_t);
    debug_print_num(hdr->part_entry_size, 5);
    direct_write("\n");
    if(!sz_multi) { panic("invalid size for pt entries; is the GPT header corrupted?"); alloc_hdr.deallocate(hdr, 1); return {}; }
    if(sz_multi == 1)
    {
        partition_table result{ size_t(hdr->num_part_entries) };
        if(!obj_read(result.data(), hdr->lba_partition_entry_array, hdr->num_part_entries)) { panic("bad read on partition table"); alloc_hdr.deallocate(hdr, 1); return {}; }
        alloc_hdr.deallocate(hdr, 1);
        return result;
    }
    size_t n = hdr->num_part_entries * sz_multi;
    partition_entry_t* arr = alloc_pt.allocate(n);
    array_zero<partition_entry_t>(arr, n);
    if(!obj_read(arr, hdr->lba_partition_entry_array, n)) { panic("bad read on plus-size partition table"); alloc_hdr.deallocate(hdr, 1); alloc_pt.deallocate(arr, n); return {}; }
    partition_table result{ size_t(hdr->num_part_entries) };
    alloc_hdr.deallocate(hdr, 1);
    for(size_t i = 0; i < n; i += sz_multi) result.push_back(arr[i]);
    alloc_pt.deallocate(arr, n);
    return result;
}