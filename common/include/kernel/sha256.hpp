#ifndef __XMM_SHA256
#define __XMM_SHA256
#include "kernel/libk_decls.h"
#include "libgcc_immintrin.h"
union [[gnu::may_alias]] sha256_st
{
    uint8_t st_bytes[32];
    uint32_t st_i32[8];
    __m128i st_xmm[2];
    sha256_st() noexcept;
};
struct sha256_hash
{
    sha256_st state;
    sha256_st& operator()(const unsigned char* data, size_t len);
};
#endif