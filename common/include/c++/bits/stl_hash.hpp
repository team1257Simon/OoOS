#ifndef __STL_HASH
#define __STL_HASH
#include "bits/dragon.hpp"
namespace std
{
    template<typename T> struct hash : public ext::dragon<T> {};
    template<basic_char_type CT> struct hash<const CT*> : public object_elf_hash<const CT*> {};
    template<basic_char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT> struct hash<basic_string<CT, TT, AT>> : public object_elf_hash<basic_string<CT, TT, AT>> {};
    template<integral IT> struct hash<IT> { uint64_t operator()(IT const& it) const noexcept { return static_cast<uint64_t>(it); } };
    template<> struct hash<void*> { uint64_t operator()(void* const& ptr) const noexcept { return elf64_hash()(addressof(ptr), sizeof(void*)); } };
    template<> struct hash<addr_t> { uint64_t operator()(addr_t const& ptr) const noexcept { return elf64_hash()(addressof(ptr), sizeof(addr_t)); } };
}
#endif