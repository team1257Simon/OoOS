#ifndef __FUNCTIONAL_HASH
#define __FUNCTIONAL_HASH
#include "bits/move.h"
#include "string"
namespace std
{
    namespace __detail
    {
        template<typename T, bool = __builtin_is_constant_evaluated()> struct __hash_byte_cnt;
        template<typename T> struct __hash_byte_cnt<T, true> { constexpr size_t operator()(T) const noexcept { return sizeof(T); } };
        template<typename T> struct __hash_byte_cnt<T, false> { size_t operator()(T) const noexcept { return sizeof(T); } };
        template<typename T> struct __hash_byte_cnt<const T*, true> { constexpr size_t operator()(const T* t) const noexcept { return sizeof(T); } };
        template<typename T> struct __hash_byte_cnt<const T*, false> { size_t operator()(const T* t) const noexcept { return sizeof(*t); } };
        template<basic_char_type CT> struct __hash_byte_cnt<CT*, false> { size_t operator()(CT* t) const noexcept { return std::strlen(t) * sizeof(CT); } };
        template<basic_char_type CT> struct __hash_byte_cnt<basic_string<CT>, false> { size_t operator()(basic_string<CT> const& str) { return str.size() * sizeof(CT); } };
        template<basic_char_type CT> struct __hash_byte_cnt<CT*, true> { constexpr size_t operator()(CT* t) const noexcept { return std::strlen(t) * sizeof(CT); } };
        template<basic_char_type CT> struct __hash_byte_cnt<basic_string<CT>, true> { constexpr size_t operator()(basic_string<CT> const& str) { return str.size() * sizeof(CT); } };
        template<typename T> struct __hash_byte_cnt<const T[], true> { constexpr size_t operator()(const T t[]) { return sizeof(t); } };
        template<typename FT> using __invoke_as_byte_hash = typename __is_invocable_impl<__invoke_result<decay_t<FT>&, uint8_t*, size_t>, uint64_t>::type;
        template<typename FT> concept __bytewise_hash = __invoke_as_byte_hash<FT>::value && is_default_constructible_v<FT>;
    }
    template<typename T, __detail::__bytewise_hash FT, bool = __builtin_is_constant_evaluated()> struct __byte_hash_adapter;
    template<typename T, __detail::__bytewise_hash FT> struct __byte_hash_adapter<T, FT, true> { constexpr uint64_t operator()(T const& t) const noexcept { const void* ptr = &t; return FT{}(static_cast<uint8_t const*>(ptr), __detail::__hash_byte_cnt<T>{}(t)); } };
    template<typename T, __detail::__bytewise_hash FT> struct __byte_hash_adapter<T, FT, false> { uint64_t operator()(T const& t) const noexcept { return FT{}(reinterpret_cast<uint8_t const*>(&t), __detail::__hash_byte_cnt<T>{}(t)); } };
}
#endif