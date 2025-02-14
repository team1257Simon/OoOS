#include "kernel/sha256.hpp"
static const uint32_t sha256_consts[64] = 
{
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, /*  0 */
    0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
    0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3, /*  8 */
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
    0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC, /* 16 */
    0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, /* 24 */
    0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
    0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13, /* 32 */
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
    0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3, /* 40 */
    0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, /* 48 */
    0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
    0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208, /* 56 */
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};
sha256_st::sha256_st() noexcept : st_i32 { 0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A, 0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19 } {}
sha256_st &sha256_hash::operator()(const unsigned char *data, size_t len)
{
    size_t len_bits = len * __CHAR_BIT__;
    size_t total_bits = up_to_nearest(len_bits + 65UL, 512UL);
    size_t k = size_t(total_bits - len_bits);
    size_t total_bytes = total_bits / __CHAR_BIT__;
    uint8_t* buffer = static_cast<uint8_t*>(_mm_malloc(total_bytes, (512UL / __CHAR_BIT__)));
    buffer[len] = 0x80;
    for(size_t i = len; i < total_bytes - 8; i++) buffer[i] = 0;
    for(size_t i = 0; i < 8; i++) { buffer[total_bytes - 8 + i] = reinterpret_cast<uint8_t*>(&k)[8 - i]; }
    size_t blocks = total_bits / 512U;
    new (&state) sha256_st();
    /* TODO run the calculation */
    return state;
}