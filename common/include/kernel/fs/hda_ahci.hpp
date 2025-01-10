#ifndef __AHCI_HARD_DISK_ACCESS
#define __AHCI_HARD_DISK_ACCESS
#include "arch/ahci.hpp"
#include "kernel/libk_decls.h"
#include "fs/generic_binary_buffer.hpp"
#include "functional"
typedef std::function<bool(qword, dword, uint16_t*)> wide_read_fn;
typedef std::function<bool(qword, dword, uint16_t const*)> wide_write_fn;
class ahci_hda
{
    static bool __has_init;
    static ahci_hda __instance;
    binary_buffer __read_buffer;
    wide_binary_buffer __write_buffer;
    wide_read_fn __read_fn;
    wide_write_fn __write_fn;
    constexpr static std::size_t __bytes_per_sector() { return 512; }
    ahci_hda();
    bool init();
public:
    static bool init_instance();
    static ahci_hda* get_instance();
    std::streamsize read(char* out, uint64_t start_sector, uint32_t count);
    std::streamsize write(uint64_t start_sector, const char* in, uint32_t count);
};
#endif