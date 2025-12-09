#ifndef __FNHASH_DRAGON
#define __FNHASH_DRAGON
#include <bits/functional_hash.hpp>
#include <bits/aligned_buffer.hpp>
#include <bits/basic_string.hpp>
#include <md5.hpp>
/**
 * DRAGON, aka Decompose and Rehash to Avoid Grossly Oversized Numbers, is a sort of hash-function adapter that will take a large-number hash (such as md5) and apply a smaller, secondary hash to the result of that function.
 * The rehash-function should be one that expects a text string as input, and outputs 64 bits or fewer. Hence, I chose to use an elf64 hash.
 * In all likelihood, GCC will optimize the temporary storage buffer out and use the A and D registers, but in the event it doesn't I'm not afraid of gobbling up 16 bytes from the stack for this.
 * Its name is a backronym I chose because dragons are cool, and it's in-keeping with the fantasy theme of the System V ABI and the various unices (ELF, DWARF, GNOME, etc).
 */
namespace std
{
	namespace ext
	{
		namespace __detail
		{
			template<typename FT, typename RT> using __invocable_as_large_hash = typename  __is_invocable_impl<__invoke_result<decay_t<FT>&, const void*, size_t>, RT>::type;
			template<typename FT, typename RT = __int128_t> concept __large_hash = __invocable_as_large_hash<FT, RT>::value && larger<RT, uint64_t> && is_default_constructible_v<FT>;
			template<typename CT, typename VT = void> concept __stl_array_like = requires { { declval<const CT>().data() } -> std::convertible_to<VT const*>; { declval<CT>().data() } -> std::convertible_to<VT*>; { declval<const CT>().size() } -> std::unsigned_integral; };
			template<__large_hash HT>
			class __dragon_base
			{
				mutable ::__impl::__aligned_buffer<__int128_t> __tmp_result;
			public:
				__dragon_base() : __tmp_result() {}
				__dragon_base(__dragon_base const&) : __tmp_result() {}
				__dragon_base(__dragon_base&&) : __tmp_result() {}
				__dragon_base& operator=(__dragon_base const&) { return *this; }
				__dragon_base& operator=(__dragon_base&&) { return *this; }
				uint64_t apply(const void* bytes, size_t len) const noexcept
				{
					construct_at(__tmp_result.__get_ptr(), HT()(bytes, len));
					uint8_t* rbytes = static_cast<uint8_t*>(__tmp_result.__get_addr());
					// We chomp on the bytes a little bit beforehand so that the elf64 hash "sees" a string entirely composed of text characters (even if they'd make no sense when read out).
					for(size_t i = 0; i < sizeof(__int128_t); i++) rbytes[i] = (rbytes[i] | 0x20) & 0x7E;
					uint64_t result = elf64_hash()(rbytes, sizeof(__int128_t));
					construct_at(__tmp_result.__get_ptr());	// temporary result buffer is to be treated as though it doesn't exist except to provide storage space during calculations
					return result;
				}
			};
		}
		template<typename T, __detail::__large_hash HT = md5> struct dragon : private __detail::__dragon_base<HT> { uint64_t operator()(T const& t) const noexcept { return this->apply(&t, sizeof(T)); } };
		template<__detail::__stl_array_like CT, __detail::__large_hash HT> struct dragon<CT, HT> : private __detail::__dragon_base<HT> { uint64_t operator()(CT const& container) const noexcept { return this->apply(container.data(), container.size()); } };
		template<std::char_type CT, __detail::__large_hash HT> struct dragon<basic_string<CT>, HT> : private __detail::__dragon_base<HT> { uint64_t operator()(std::basic_string<CT> const& str) const noexcept { return this->apply(str.c_str(), str.size()); } };
	}
}
#endif