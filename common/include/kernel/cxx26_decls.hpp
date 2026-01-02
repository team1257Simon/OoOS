#ifndef __CXX26_DECLS
#define __CXX26_DECLS
/**
 * VSCode's intellisense currently freaks out with C++26 things like template for and pack indexing.
 * Extracting that stuff here makes other files easier to read.
 */
#include <tuple>
namespace std
{
	template<bool ... Bs, size_t ... Is>
	consteval size_t __first_false_helper(tuple<bool_constant<Bs>...>, index_sequence<Is...>)
	{
		template for(constexpr size_t i : { Is ... })
			if constexpr(!Bs...[i])
				return i;
		return sizeof...(Bs);
	}
	template<bool ... Bs> consteval size_t first_false_in() { return __first_false_helper(tuple<bool_constant<Bs>...>(), make_index_sequence<sizeof...(Bs)>()); }
}
#endif