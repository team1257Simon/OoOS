#ifndef __HASH_POLICY
#define __HASH_POLICY
#include "bits/stl_pair.hpp"
namespace std
{
    namespace __impl
    {
        struct __prime_number_policy
        {
            float __my_max_load;
            mutable size_t __next_resize;
            typedef size_t __state_type;
            constexpr __prime_number_policy(float z = 1.0f) noexcept : __my_max_load{ z }, __next_resize{ 0 } {}
            constexpr float __max_load_factor() const noexcept { return __my_max_load; }
            constexpr void __reset_state() noexcept { __next_resize = 0; }
            constexpr void __recall_state(__state_type s) { __next_resize = s; }
            constexpr static size_t __growth_factor = 2;
            size_t __buckets_for_elements(size_t n) const;
            size_t __next_bucket(size_t n) const;
            pair<bool, size_t> __need_rehash(size_t n_buckets, size_t n_elems, size_t n_ins) const;
        };
        struct __pow2_policy
        {
            float __my_max_load;
            mutable size_t __next_resize;
            typedef size_t __state_type;
            constexpr __pow2_policy(float z = 1.0f) noexcept : __my_max_load{ z }, __next_resize{ 0 } {}
            constexpr float __max_load_factor() const noexcept { return __my_max_load; }
            constexpr void __reset_state() noexcept { __next_resize = 0; }
            constexpr void __recall_state(__state_type s) { __next_resize = s; }
            constexpr static size_t __growth_factor = 2;
            size_t __buckets_for_elements(size_t n) const;
            size_t __next_bucket(size_t n) const;
            pair<bool, size_t> __need_rehash(size_t n_buckets, size_t n_elems, size_t n_ins) const;
        };
        struct __modulus_hash_range { constexpr size_t operator()(size_t idx, size_t nb) const noexcept { if(nb) return idx % nb; return 0; } };
        template<typename T, typename U> struct __pair_key_extract { constexpr T const& operator()(pair<T, U> const& p) const noexcept { return p.first; } };
        template<typename T> struct __identity_key { constexpr T const& operator()(T const& t) const noexcept { return t; } };
    }
}
#endif