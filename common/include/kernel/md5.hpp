#ifndef __MD5_HASH
#define __MD5_HASH
#include "kernel/kernel_defs.h"
__int128_t md5(uint8_t const* input, size_t len);
#endif