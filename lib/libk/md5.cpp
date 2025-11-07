#include "kernel/md5.hpp"
#include "kernel/libk_decls.h"
#include "bits/stl_allocator.h"
__int128_t md5_hash(const void* input, size_t len);
void md5_step(uint32_t* buffer, uint32_t* input);
__int128_t md5::operator()(const void* input, size_t len) const noexcept { return md5_hash(input, len); }
constexpr static uint32_t lrotate(uint32_t x, uint32_t n) { return (x << n) | (x >> (32 - n)); }
struct md5_ctx
{
	uint64_t size       { 0UL };
	uint32_t buffer[4]  { 0x67452301U, 0xEFCDAB89U, 0x98BADCFEU, 0x10325476U };
	uint8_t input[64]   {};
	uint8_t digest[16]  {};
};
static std::allocator<uint8_t> buf_alloc{};
static uint32_t __s[] 
{
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};
static uint32_t __k[]
{
	0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
	0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
	0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
	0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
	0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
	0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
	0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
	0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
	0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
	0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
	0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
	0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
	0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
	0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
	0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
	0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};
static uint8_t padding[]
{
	0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
/*
* Add some amount of input to the context
* If the input fills out a block of 512 bits, apply the algorithm (md5_step)
* and save the result in the buffer. Also updates the overall size.
*/
void md5_update(md5_ctx* ctx, uint8_t const* input_buffer, size_t input_len)
{
	uint32_t input[16];
	unsigned int offset = ctx->size % 64;
	ctx->size += input_len;
	// Copy each byte in input_buffer into the next space in our context input
	for(unsigned i = 0; i < input_len; ++i)
	{
		ctx->input[offset++] = input_buffer[i];
		// If we've filled our context input, copy it into our local array input
		// then reset the offset to 0 and fill in a new buffer.
		// Every time we fill out a chunk, we run it through the algorithm
		// to enable some back and forth between cpu and i/o
		if(offset % 64 == 0)
		{
			for(unsigned j = 0; j < 16; ++j) { input[j] = static_cast<uint32_t>(ctx->input[(j * 4) + 3]) << 24 | static_cast<uint32_t>(ctx->input[(j * 4) + 2]) << 16 | static_cast<uint32_t>(ctx->input[(j * 4) + 1]) <<  8 | static_cast<uint32_t>(ctx->input[(j * 4)]); }
			md5_step(ctx->buffer, input);
			offset = 0;
		}
	}
}
/*
* Pad the current input to get to 448 bytes, append the size in bits to the very end,
* and save the result of the final iteration into digest.
*/
void md5_finalize(md5_ctx* ctx)
{
	uint32_t input[16];
	unsigned int offset = ctx->size % 64;
	unsigned int padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;
	// Fill in the padding and undo the changes to size that resulted from the update
	md5_update(ctx, padding, padding_length);
	ctx->size -= static_cast<uint64_t>(padding_length);
	// Do a final update (internal to this function)
	// Last two 32-bit words are the two halves of the size (converted from bytes to bits)
	for(unsigned j = 0; j < 14; ++j) { input[j] = static_cast<uint32_t>(ctx->input[(j * 4) + 3]) << 24 | static_cast<uint32_t>(ctx->input[(j * 4) + 2]) << 16 | static_cast<uint32_t>(ctx->input[(j * 4) + 1]) <<  8 | static_cast<uint32_t>(ctx->input[(j * 4)]); }
	input[14] = static_cast<uint32_t>((ctx->size * 8) & 0xFFFFFFFF);
	input[15] = static_cast<uint32_t>((ctx->size * 8) >> 32);
	md5_step(ctx->buffer, input);
	// Move the result into digest (convert from little-endian)
	for(unsigned int i = 0; i < 4; ++i)
	{
		ctx->digest[(i * 4) + 0] = static_cast<uint8_t>((ctx->buffer[i] & 0x000000FF));
		ctx->digest[(i * 4) + 1] = static_cast<uint8_t>((ctx->buffer[i] & 0x0000FF00) >>  8);
		ctx->digest[(i * 4) + 2] = static_cast<uint8_t>((ctx->buffer[i] & 0x00FF0000) >> 16);
		ctx->digest[(i * 4) + 3] = static_cast<uint8_t>((ctx->buffer[i] & 0xFF000000) >> 24);
	}
}
void md5_step(uint32_t* buffer, uint32_t* input)
{
	uint32_t __a = buffer[0];
	uint32_t __b = buffer[1];
	uint32_t __c = buffer[2];
	uint32_t __d = buffer[3];
	uint32_t __e;
	unsigned int j;
	for(unsigned i = 0; i < 64U; ++i)
	{
		switch(i / 16)
		{
			case 0:
				__e = ((__b & __c) | (~__b & __d));
				j = i;
				break;
			case 1:
				__e = ((__b & __d) | (__c & ~__d));
				j = ((i * 5) + 1) % 16;
				break;
			case 2:
				__e = (__b ^ __c ^ __d);
				j = ((i * 3) + 5) % 16;
				break;
			default:
				__e = (__c ^ (__b | ~__d));
				j = (i * 7) % 16;
				break;
		}
		uint32_t temp = __d;
		__d = __c;
		__c = __b;
		__b = __b + lrotate(__a + __e + __k[i] + input[j], __s[i]);
		__a = temp;
	}

	buffer[0] += __a;
	buffer[1] += __b;
	buffer[2] += __c;
	buffer[3] += __d;
}
__int128_t md5_hash(const void* input, size_t len)
{
	md5_ctx ctx{};
	uint8_t* buff = buf_alloc.allocate(len);
	array_copy<uint8_t>(buff, static_cast<const uint8_t*>(input), len);
	md5_update(&ctx, buff, len);
	md5_finalize(&ctx);
	buf_alloc.deallocate(buff, len);
	return *reinterpret_cast<__int128_t*>(ctx.digest);
}