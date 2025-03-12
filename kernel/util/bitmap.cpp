#include "bitmap.hpp"
#include "string"
constexpr off_t ulsize = CHAR_BIT * sizeof(unsigned long);
off_t bitmap_scan_sz(const unsigned long* bitmap, size_t num_ulongs) { for(size_t i = 0; i < num_ulongs; i++) { if(unsigned long ul = ~(bitmap[i])) return (ulsize * i) + __builtin_ctzl(ul); } return -1L; }
void bitmap_set_sbit(unsigned long *bitmap, off_t bit_pos) { bitmap[bit_pos / ulsize] |= (1 << (bit_pos % ulsize)); }
void bitmap_clear_sbit(unsigned long *bitmap, off_t bit_pos) { bitmap[bit_pos / ulsize] &= ~(1 << (bit_pos % ulsize)); }
off_t bitmap_scan_cz(const unsigned long* bitmap, size_t num_ulongs, size_t num_zeroes)
{
    if(num_zeroes == 1) return bitmap_scan_sz(bitmap, num_ulongs);
    if(__builtin_expect(num_zeroes > ulsize, false)) 
    {
        size_t num_needed_ulongs = div_roundup(num_zeroes, ulsize);
        for(size_t i = 0; i < num_ulongs; )
        {
            size_t n = std::strnlen<unsigned long>(bitmap + i, num_needed_ulongs);
            if(n == num_needed_ulongs) return i * ulsize;
            i += n;
        }
    }
    else for(size_t i = 0; i < num_ulongs; i++)
    {
        unsigned long cur = bitmap[i];
        if(num_zeroes == ulsize && !cur) return i * ulsize; 
        unsigned long mask = ~(~0UL >> num_zeroes);
        for(off_t j = 0; j < static_cast<off_t>(ulsize - num_zeroes); j++) if(!(cur & (mask << j))) return i * ulsize + j;
    }
    return -1;
}
void bitmap_set_cbits(unsigned long* bitmap, off_t bit_pos, size_t num_bits)
{
    if(num_bits == 1) { bitmap_set_sbit(bitmap, bit_pos); return; }
    off_t ulpos = bit_pos / ulsize;
    off_t bit_off = bit_pos % ulsize;
    if(__builtin_expect(num_bits >= ulsize, false)) 
    { 
        size_t actual = std::min(num_bits, size_t(ulsize - bit_off));
        bitmap[ulpos] |= (~0UL << bit_off); 
        if(num_bits > actual) { bitmap_set_cbits(bitmap, bit_pos + actual, size_t(num_bits - actual)); }
    }
    else
    {
        uint64_t mask = num_bits == 1 ? 1UL : (~0UL >> (ulsize - num_bits));
        bitmap[ulpos] |= (mask << bit_off);
        if(bit_off + num_bits > ulsize) bitmap[ulpos + 1] |= (mask >> ((num_bits + bit_off) % ulsize));
    }
}
void bitmap_clear_cbits(unsigned long *bitmap, off_t bit_pos, size_t num_bits)
{
    if(num_bits == 1) { bitmap_clear_sbit(bitmap, bit_pos); return; }
    off_t ulpos = bit_pos / ulsize;
    off_t bit_off = bit_pos % ulsize;
    if(__builtin_expect(num_bits >= ulsize, false))
    { 
        size_t actual = std::min(num_bits, size_t(ulsize - bit_off)); 
        bitmap[ulpos] &= ~(~0UL << bit_off); 
        if(num_bits > actual) { bitmap_set_cbits(bitmap, bit_pos + actual, size_t(num_bits - actual)); } 
    }
    else
    {
        uint64_t mask = num_bits == 1 ? 1UL : (~0UL >> (ulsize - num_bits));
        bitmap[ulpos] &= ~(mask << bit_off);
        if(bit_off + num_bits > ulsize) bitmap[ulpos + 1] &= ~(mask >> ((num_bits + bit_off) % ulsize));
    }
}