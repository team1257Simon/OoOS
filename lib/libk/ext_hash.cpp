#include "kernel/libk_decls.h"
#include "bits/functional_hash.hpp"
struct hash_buffer { uint32_t dword_buf[8]; };
typedef void (*hash_round_fn)(hash_buffer& buf, const hash_buffer& in);
constexpr uint32_t K1 = 0;
constexpr uint32_t K2 = 013240474631UL;
constexpr uint32_t K3 = 015666365641UL;
constexpr hash_buffer buf_init{ { 0x67452301U, 0xEFCDAB89U, 0x98BDACFEU, 0x10325476U, 0, 0, 0, 0 } };
template<bool is_signed> void process_input(const void* in, hash_buffer& out, size_t len, int64_t num);
template<bool is_signed, hash_round_fn hash_fn, int num, unsigned b_off> static uint64_t gen_ext_hash(uint32_t* seed, const void* data, size_t len);
template<bool is_signed> static uint32_t apply_legacy_hash(const void* in, size_t len);
static void apply_tea_transform(hash_buffer& buf, hash_buffer const& in);
static void apply_md4_half_transform(hash_buffer& buf, hash_buffer const& in);
uint64_t std::ext_legacy_hash_signed::operator()(const void* data, size_t n) { return apply_legacy_hash<true>(data, n); }
uint64_t std::ext_legacy_hash_unsigned::operator()(const void* data, size_t n) { return apply_legacy_hash<false>(data, n); }
uint64_t std::half_md4_hash_signed::operator()(const void* data, size_t n) { return gen_ext_hash<true, &apply_md4_half_transform, 8, 1U>(seed, data, n); }
uint64_t std::half_md4_hash_unsigned::operator()(const void* data, size_t n) { return gen_ext_hash<false, &apply_md4_half_transform, 8, 1U>(seed, data, n); }
uint64_t std::tea_hash_signed::operator()(const void* data, size_t n) { return gen_ext_hash<true, &apply_tea_transform, 4, 0U>(seed, data, n); }
uint64_t std::tea_hash_unsigned::operator()(const void* data, size_t n) { return gen_ext_hash<false, &apply_tea_transform, 4, 0U>(seed, data, n); }
template<bool is_signed> void process_input(const void* in, hash_buffer& out, size_t len, int64_t num)
{
    using data_t = typename std::conditional<is_signed, const signed char*, const unsigned char*>::type;
    uint32_t pad = static_cast<uint32_t>(len) | (static_cast<uint32_t>(len) << 8);
    pad |= pad << 16;
    uint32_t val = pad;
    len = std::min(len, size_t(num * 4));
    data_t buffer = static_cast<data_t>(in);
    size_t n = 0;
    for(size_t i = 0; i < len; i++)
    {
        val = static_cast<int>(buffer[i]) + (val << 8);
        if((i % 4) == 3)
        {
            out.dword_buf[n++] = val;
            val = pad;
            num--;
        }
    }
    if(--num >= 0) out.dword_buf[n++] = val;
    while(--num >= 0) out.dword_buf[n++] = pad;
}
template<bool is_signed, hash_round_fn hash_fn, int num, unsigned b_off> static uint64_t gen_ext_hash(uint32_t* seed, const void* data, size_t len)
{
    const char* name = static_cast<const char*>(data);
    hash_buffer in{};
    hash_buffer buf = buf_init;
    if(seed) { for(int i = 0; i < 4; i++) { if(seed[i]) array_copy(buf.dword_buf, seed, 4); break; } }
    const char* p = name;
    while(len > 0)
    {
        process_input<is_signed>(name, in, len, num);
        (*hash_fn)(buf, in);
        len -= num * 4;
        p += num * 4;
    }
    return qword(buf.dword_buf[b_off], buf.dword_buf[b_off + 1]);
}
template<bool is_signed> static uint32_t apply_legacy_hash(const void* in, size_t len)
{
    uint32_t hash, hash0 = 0x12A3FE2DU, hash1 = 0x37ABE8F9U;
    using data_t = typename std::conditional<is_signed, const signed char*, const unsigned char*>::type;
    data_t buffer = static_cast<data_t>(in);
    for(size_t i = 0; i < len; i++)
    {
        hash = hash1 + (hash0 ^ (static_cast<int>(buffer[i]) * 7152373));
        if(hash & 0x80000000) hash -= 0x7FFFFFFFU;
        hash1 = hash0;
        hash0 = hash;
    }
    return hash0 << 1;
}
static void apply_tea_transform(hash_buffer& buf, hash_buffer const& in)
{
    uint32_t sum = 0;
    uint32_t b0 = buf.dword_buf[0], b1 = buf.dword_buf[1];
    uint32_t a = in.dword_buf[0], b = in.dword_buf[1], c = in.dword_buf[2], d = in.dword_buf[3];
    size_t n = 16;
    do
    {
        sum += 0x9E3779B9;
        b0 += ((b1 << 4) + a) ^ (b1 + sum) ^ ((b1 >> 5) + b);
		b1 += ((b0 << 4) + c) ^ (b0 + sum) ^ ((b0 >> 5) + d);
    } while(--n);
    buf.dword_buf[0] += b0;
    buf.dword_buf[1] += b1;
}
static void apply_md4_half_transform(hash_buffer& buf, hash_buffer const& in)
{
	uint32_t a = buf.dword_buf[0], b = buf.dword_buf[1], c = buf.dword_buf[2], d = buf.dword_buf[3];
	a += ((d) ^ ((b) & ((c) ^ (d)))) + in.dword_buf[0] + K1, a = (a << 3) | (a >> ((~3) & 31));
	d += ((c) ^ ((a) & ((b) ^ (c)))) + in.dword_buf[1] + K1, d = (d << 7) | (d >> ((~7) & 31));
	c += ((b) ^ ((d) & ((a) ^ (b)))) + in.dword_buf[2] + K1, c = (c << 11) | (c >> ((~11) & 31));
	b += ((a) ^ ((c) & ((d) ^ (a)))) + in.dword_buf[3] + K1, b = (b << 19) | (b >> ((~19) & 31));
	a += ((d) ^ ((b) & ((c) ^ (d)))) + in.dword_buf[4] + K1, a = (a << 3) | (a >> ((~3) & 31));
	d += ((c) ^ ((a) & ((b) ^ (c)))) + in.dword_buf[5] + K1, d = (d << 7) | (d >> ((~7) & 31));
	c += ((b) ^ ((d) & ((a) ^ (b)))) + in.dword_buf[6] + K1, c = (c << 11) | (c >> ((~11) & 31));
	b += ((a) ^ ((c) & ((d) ^ (a)))) + in.dword_buf[7] + K1, b = (b << 19) | (b >> ((~19) & 31));
	a += (((b) & (c)) + (((b) ^ (c)) & (d))) + in.dword_buf[1] + K2, a = (a << 3) | (a >> ((~3) & 31));
	d += (((a) & (b)) + (((a) ^ (b)) & (c))) + in.dword_buf[3] + K2, d = (d << 5) | (d >> ((~5) & 31));
	c += (((d) & (a)) + (((d) ^ (a)) & (b))) + in.dword_buf[5] + K2, c = (c << 9) | (c >> ((~9) & 31));
	b += (((c) & (d)) + (((c) ^ (d)) & (a))) + in.dword_buf[7] + K2, b = (b << 13) | (b >> ((~13) & 31));
	a += (((b) & (c)) + (((b) ^ (c)) & (d))) + in.dword_buf[0] + K2, a = (a << 3) | (a >> ((~3) & 31));
	d += (((a) & (b)) + (((a) ^ (b)) & (c))) + in.dword_buf[2] + K2, d = (d << 5) | (d >> ((~5) & 31));
	c += (((d) & (a)) + (((d) ^ (a)) & (b))) + in.dword_buf[4] + K2, c = (c << 9) | (c >> ((~9) & 31));
	b += (((c) & (d)) + (((c) ^ (d)) & (a))) + in.dword_buf[6] + K2, b = (b << 13) | (b >> ((~13) & 31));
	a += ((b) ^ (c) ^ (d)) + in.dword_buf[3] + K3, a = (a << 3) | (a >> ((~3) & 31));
	d += ((a) ^ (b) ^ (c)) + in.dword_buf[7] + K3, d = (d << 9) | (d >> ((~9) & 31));
	c += ((d) ^ (a) ^ (b)) + in.dword_buf[2] + K3, c = (c << 11) | (c >> ((~11) & 31));
	b += ((c) ^ (d) ^ (a)) + in.dword_buf[6] + K3, b = (b << 15) | (b >> ((~15) & 31));
	a += ((b) ^ (c) ^ (d)) + in.dword_buf[1] + K3, a = (a << 3) | (a >> ((~3) & 31));
	d += ((a) ^ (b) ^ (c)) + in.dword_buf[5] + K3, d = (d << 9) | (d >> ((~9) & 31));
	c += ((d) ^ (a) ^ (b)) + in.dword_buf[0] + K3, c = (c << 11) | (c >> ((~11) & 31));
	b += ((c) ^ (d) ^ (a)) + in.dword_buf[4] + K3, b = (b << 15) | (b >> ((~15) & 31));
	buf.dword_buf[0] += a;
	buf.dword_buf[2] += b;
	buf.dword_buf[3] += c;
	buf.dword_buf[4] += d;
}