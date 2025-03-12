#ifndef __BITMAP_UTILS
#define __BITMAP_UTILS
#include "kernel/libk_decls.h"
#include "tuple"
#include "sys/types.h"
/**
 * Finds the first string of zeroes in a bitmap of size num_ulongs * sizeof(unsigned long) whose length L fulfills (L >= num_zeroes && (L < sizeof(unsigned long) || L % sizeof(unsigned long) == 0)).
 * The return value is the offset of the first bit in that string if it is present, or -1 if none is found.
 */
off_t bitmap_scan_cz(const unsigned long* bitmap, size_t num_ulongs, size_t num_zeroes);
void bitmap_set_cbits(unsigned long* bitmap, off_t bit_pos, size_t num_bits);
void bitmap_clear_cbits(unsigned long* bitmap, off_t bit_pos, size_t num_bits);
// Finds the position of the first zero in a bitmap of size num_ulongs * sizeof(unsigned long). The return value is the offset in total bits.
off_t bitmap_scan_sz(const unsigned long* bitmap, size_t num_ulongs);
void bitmap_set_sbit(unsigned long* bitmap, off_t bit_pos);
void bitmap_clear_sbit(unsigned long* bitmap, off_t bit_pos);
#endif