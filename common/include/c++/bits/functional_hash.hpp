#ifndef __FUNCTIONAL_HASH
#define __FUNCTIONAL_HASH
#include "bits/move.h"
#include "string"
namespace std
{
    extension struct elf64_hash { constexpr uint64_t operator()(const void* data, size_t n) const noexcept { uint64_t h = 0; for(size_t i = 0; i < n; i++) { h = (h << 4) + static_cast<uint8_t const*>(data)[i]; if (uint64_t g = (h & 0xF0000000)) { h ^= g >> 24; h &= ~g; } } return h; } };
    extension template<basic_char_type CT> struct basic_string_elf_hash { constexpr uint64_t operator()(basic_string<CT> const& str) const noexcept { return elf64_hash{}(str.c_str(), str.size() * sizeof(CT)); } constexpr uint64_t operator()(const CT* str) const noexcept { return elf64_hash{}(str, std::strlen(str) * sizeof(CT)); } };
    extension struct ext_legacy_hash_signed { uint64_t operator()(const void* data, size_t n); };
    extension struct ext_legacy_hash_unsigned{ uint64_t operator()(const void* data, size_t n); };
    extension struct half_md4_hash_signed { uint32_t seed[4]{}; uint64_t operator()(const void* data, size_t n); };
    extension struct half_md4_hash_unsigned { uint32_t seed[4]{}; uint64_t operator()(const void* data, size_t n); };
    extension struct tea_hash_signed { uint32_t seed[4]{}; uint64_t operator()(const void* data, size_t n); };
    extension struct tea_hash_unsigned { uint32_t seed[4]{}; uint64_t operator()(const void* data, size_t n); };
}
#endif