#ifndef __STL_HASH
#define __STL_HASH
#include "bits/dragon.hpp"
#include "bits/stl_function.hpp"
namespace std
{
    namespace __detail{ template<typename T> concept __not_char_type = !basic_char_type<T>; };
    template<typename T> struct hash : public ext::dragon<T> {};
    template<basic_char_type CT> struct hash<const CT*> : public object_elf_hash<const CT*> {};
    template<basic_char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> struct hash<basic_string<CT, TT, AT>> : public object_elf_hash<basic_string<CT, TT, AT>> {};
    template<integral IT> struct hash<IT> : ext::static_cast_t<IT, uint64_t> {};
    template<> struct hash<void*> { uint64_t operator()(void* const& ptr) const noexcept { return elf64_hash()(addressof(ptr), sizeof(void*)); } };
    template<> struct hash<const void*> { uint64_t operator()(const void* const& ptr) const noexcept { return elf64_hash()(addressof(ptr), sizeof(const void*)); } };
    template<__detail::__not_char_type T> struct hash<T*> : hash<void*>{};
    template<__detail::__not_char_type T> struct hash<T const*> : hash<const void*>{};
    template<> struct hash<addr_t> { uint64_t operator()(addr_t const& ptr) const noexcept { return elf64_hash()(addressof(ptr), sizeof(addr_t)); } };
}
#endif