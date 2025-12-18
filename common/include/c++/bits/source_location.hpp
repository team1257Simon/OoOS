#ifndef __SRC_LOC
#define __SRC_LOC
#include <bits/move.h>
namespace std
{
	struct source_location
	{
	private:
		using uint_least32_t	= __UINT_LEAST32_TYPE__;
		struct __impl
		{
			const char* __file_name;
			const char* __function_name;
			unsigned __line_no;
			unsigned __column_no;
		};
		using __builtin_ret_type	= decltype(__builtin_source_location());
		__impl const* __my_impl		= nullptr;
	public:
		static consteval source_location current(__builtin_ret_type __p = __builtin_source_location()) noexcept
		{
			source_location __ret;
			__ret.__my_impl			= static_cast<__impl const*>(__p);
			return __ret;
		}
		constexpr source_location() noexcept {}
		constexpr uint_least32_t line() const noexcept { return __my_impl ? __my_impl->__line_no : 0U; }
		constexpr uint_least32_t column() const noexcept { return __my_impl ? __my_impl->__column_no : 0U; }
		constexpr const char* file_name() const noexcept { return __my_impl ? __my_impl->__file_name : ""; }
		constexpr const char* function_name() const noexcept { return __my_impl ? __my_impl->__function_name : ""; }
	};

}
#endif