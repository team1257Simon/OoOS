#include "hda_base.hpp"
#include "bits/stdexcept.h"
std::size_t hda_base::__bytes_per_sector() { return __sector_size; }
hda_base::hda_base(std::size_t sector_size, unsigned int buffer_sectors) : __sector_size{ sector_size }, __read_buffer{ sector_size * buffer_sectors }, __write_buffer{ sector_size * buffer_sectors / 2 } {}
std::streamsize hda_base::read(std::basic_streambuf<char>& out, uint64_t start_sector, uint32_t count)
{
    try
    {
        __read_buffer.clear();
        if(__read_buffer.rcopy(this->__read_disk(start_sector, count))) return __read_buffer.rxfer(out);
        else panic("could not copy data buffer");
    }
    catch(std::exception& e) { panic(e.what()); }
    return 0;
}

std::streamsize hda_base::write(uint64_t start_sector, const char *in, uint32_t count)
{
    try
    {
        __write_buffer.clear();
        std::streamsize result = __write_buffer.rputn<char>(in, count * __sector_size);
        __write_disk(__write_buffer, start_sector, count);
        return result;
    }
    catch(std::exception& e) { panic(e.what()); }
    return 0;
}
