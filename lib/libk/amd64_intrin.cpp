#include "kernel/libk_decls.h"
#include "immintrin.h"
uint32_t crc32_calc(const void* data, size_t len)
{
    uint32_t result = 0xFFFFFFFFU;
    for(uint64_t const* q = reinterpret_cast<uint64_t const*>(data); len >= 8; q++, len -= 8, data = q) result = _mm_crc32_u64(result, *q);
    if(len >= 4) { uint32_t const* d = reinterpret_cast<uint32_t const*>(data); result = _mm_crc32_u32(result, *d); data = ++d; len -= 4; }
    if(len >= 2) { uint16_t const* w = reinterpret_cast<uint16_t const*>(data); result = _mm_crc32_u16(result, *w); data = ++w; len -= 2; }
    if(len) { result = _mm_crc32_u8(result, *reinterpret_cast<uint8_t const*>(data)); }
    return result ^ 0xFFFFFFFFU;
}