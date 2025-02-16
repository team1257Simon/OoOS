#ifndef __FNHASH_DRAGON
#define __FNHASH_DRAGON
#include "bits/functional_hash.hpp"
#include "bits/aligned_buffer.hpp"
#include "md5.hpp"
/**
 * DRAGON, aka Decompose and Rehash to Avoid Grossly Oversized Numbers, is a sort of hash-function adapter that will take a large-number hash (such as md5) and apply a smaller-result rehash to the result of that function.
 * The rehash-function should be one that expects a text string as input, and outputs 64 bits or fewer.
 */
namespace std
{
    namespace __detail
    {
        template<typename FT, typename RT> using __invocable_as_large_hash = typename  __is_invocable_impl<__invoke_result<decay_t<FT>&, const void*, size_t>, RT>::type;
        template<typename FT, typename RT> concept __large_hash = __invocable_as_large_hash<FT, RT>::value && larger<RT, uint64_t> && is_default_constructible_v<FT>;
    }
    template<__detail::__large_hash<__int128_t> HT>
    class dragon
    {
        ::__impl::__aligned_buffer<__int128_t> __tmp_result{};
    public:
        uint64_t operator()(const void* bytes, size_t len) noexcept
        {
            construct_at(__tmp_result.__get_ptr(), HT{}(bytes, len));
            uint8_t* rbytes = reinterpret_cast<uint8_t*>(__tmp_result.__get_addr());
            for(size_t i = 0; i < sizeof(__int128_t); i++)
            {
                rbytes[i] &= 0x7F;
                rbytes[i] |= 0x20;
            }
            uint64_t result = elf64_hash{}(rbytes, sizeof(__int128_t));
            return result;
        }
    };
    template<basic_char_type CT> struct basic_string_dragon { uint64_t operator()(basic_string<CT> const& str) const noexcept { return dragon<md5>{}(str.c_str(), str.size() * sizeof(CT)); } };
}
#endif