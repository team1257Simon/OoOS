#ifndef __ASSERT_H
#define __ASSERT_H
#ifdef __cplusplus
extern "C"
#endif
[[noreturn]] void __on_fail_assert(const char*, const char*, const char*, int);
#define assert(exp) if(!(exp)) __on_fail_assert(#exp, __builtin_FUNCTION(), __builtin_FILE(), __builtin_LINE())
#endif