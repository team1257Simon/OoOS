#ifndef __MD5_HASH
#define __MD5_HASH
#include <kernel_defs.h>
struct md5 { __int128_t operator()(const void* input, size_t len) const noexcept; };
#endif