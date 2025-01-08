#ifndef __HARD_DISK_ACCESS
#define __HARD_DISK_ACCESS
#include "kernel/libk_decls.h"
#include "generic_binary_buffer.hpp"
class hda_base
{
    std::size_t __sector_size;
    binary_buffer __read_buffer;
    wide_binary_buffer __write_buffer;
protected:
    virtual std::size_t __bytes_per_sector() final;
    virtual wide_binary_buffer __read_disk(uint64_t start_sector, uint32_t count) throw() = 0;
    virtual void __write_disk(wide_binary_buffer const& data, uint64_t start_sector, uint32_t count) throw() = 0;
public:
    hda_base(std::size_t sector_size, unsigned int buffer_sectors = 4);
    std::streamsize read(std::basic_streambuf<char>& out, uint64_t start_sector, uint32_t count);
    std::streamsize write(uint64_t start_sector, const char* in, uint32_t count);    
};
#endif