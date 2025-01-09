#include "fs/hda_ahci.hpp"
bool ahci_hda::__has_init = false;
ahci_hda ahci_hda::__instance{};
ahci_hda::ahci_hda() : __read_buffer{__bytes_per_sector() * 4}, __write_buffer{ __bytes_per_sector() * 2 } {}
bool ahci_hda::init_instance() { if(__has_init) return true; return (__has_init = __instance.init()); }
bool ahci_hda::init()
{
   if(!ahci_driver::is_initialized()) { panic("no AHCI driver available"); return false; }
   ahci_driver* dr = ahci_driver::get_instance();
   int8_t p = dr->which_port(sata);
   if(p < 0) return false;
   // Calling which_port every time we read or write is needlessly costly because the result won't change after startup.
   // To avoid looping through the ports every time, we cache that value and the pointer here. 
   // While using function pointers is not technically necessary in the base case, it opens the door for extensibility later.
   this->__read_fn = [&](qword start, dword count, uint16_t* data) -> bool { return dr->read_sectors(p, start, count, data); };
   this->__write_fn = [&](qword start, dword count, const uint16_t* data) -> bool { return dr->write_sectors(p, start, count, data); };
   return true;
}
ahci_hda *ahci_hda::get_instance() { return __has_init ? &__instance : nullptr; }
std::streamsize ahci_hda::read(char* out, uint64_t start_sector, uint32_t count)
{
    __read_buffer.clear();
    if(!__read_fn) { panic("cannot read disk before initializing read accessor"); return 0; } 
    wide_binary_buffer result{ this->__bytes_per_sector() * count / 2 }; 
    if(!__read_fn(start_sector, count, result.beg())) { panic("bad read"); return 0; } 
    if(__read_buffer.rcopy(result)) { return __read_buffer.rgetn<char>(out, count *  __bytes_per_sector()); }
    else panic("could not copy data buffer");
    return 0;
}
std::streamsize ahci_hda::write(uint64_t start_sector, const char *in, uint32_t count)
{
    __write_buffer.clear();
    std::streamsize result = __write_buffer.rputn<char>(in, count *  __bytes_per_sector());
    if(!__write_fn) { panic("cannot write disk before initializing write accessor"); return 0; }
    if(!__write_fn(start_sector, count, __write_buffer.data())) { panic("bad write"); return 0; }
    return result;
}