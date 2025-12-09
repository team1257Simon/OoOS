#ifndef __DESCRIBE
#define __DESCRIBE
#include <meta>
#include <ranges>
using namespace std::meta;
template<typename T> consteval size_t member_count() try { return nonstatic_data_members_of(^^T, access_context::current()).size(); } catch(...) { return 0UZ; }
consteval info nth_member_of(info type, size_t i) { return nonstatic_data_members_of(type, access_context::current())[i]; }
template<typename T> consteval member_offset nth_member_offset(size_t i) { return offset_of(nth_member_of(^^T, i)); }
struct member_name_and_offset
{
	std::string_view name;
	std::ptrdiff_t offset_bytes;
	std::ptrdiff_t offset_bits;
    bool bit_flag;
};
struct describe
{
	std::ptrdiff_t offset_mod{};
	consteval std::string_view identifier_if_present(info i)
    {
		if(has_identifier(i))
			return identifier_of(i);
		else return std::string_view("");
	}
	consteval member_name_and_offset operator()(info member)
    {
        ptrdiff_t bytes = offset_of(member).bytes + offset_mod;
        ptrdiff_t bits  = offset_of(member).bits;
		return member_name_and_offset(identifier_if_present(member), bytes, bits, bit_size_of(member) == 1UZ);
	}
    constexpr static describe for_offset(member_offset o) { return describe(o.bytes); }
    constexpr static describe for_offset(member_offset o, ptrdiff_t base) { return describe(o.bytes + base); }
};
template<typename T>
struct sub_object_array
{
	constexpr static std::size_t size = member_count<T>();
	template<typename F>
	struct builder
	{
		typedef std::invoke_result_t<F, info> result_type;
		consteval std::array<result_type, size> operator()(F f)
		{
			std::array<result_type, size> result{};
			constexpr info sequence = reflect_constant_array(std::views::iota(0UZ, size));
			template for(constexpr size_t i : [: sequence :])
				result[i] = f(nth_member_of(^^T, i));
			return result;
		}
        consteval std::array<result_type, size> operator()() requires(std::is_default_constructible_v<F>) { return (*this)(F()); }
	};
};
template<typename T> using describe_fields	= typename sub_object_array<T>::template builder<describe>;
template<typename T> consteval std::size_t index_for_member(std::string_view name)
{
	constexpr info indices = reflect_constant_array(std::views::iota(0UZ, member_count<T>()));
	template for(constexpr std::size_t i : [: indices :])
	{
		constexpr info member = nth_member_of(^^T, i);
		if(name == identifier_of(member))
			return i;
	}
	return size_t(-1);
}
template<typename T, std::size_t I> consteval bool nth_member_has_members()
{
	constexpr info member_type = type_of(nth_member_of(^^T, I));
	using type = [: member_type :];
	return member_count<type>() != 0UZ;
}
template<typename T, size_t I>
consteval auto describe_member_subobjects()
{
	static_assert(nth_member_has_members<T, I>(), "member is not a sub-object with members to describe");
	constexpr member_offset offs	= nth_member_offset<T>(I);
	constexpr info member_type_i	= nth_member_of(^^T, I);
	using member_type				= [: type_of(member_type_i) :];
	return describe_fields<member_type>()(describe::for_offset(offs));
}
#endif