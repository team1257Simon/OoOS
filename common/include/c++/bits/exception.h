#ifndef __STD_EXCEPTION
#define __STD_EXCEPTION
namespace std
{
	class exception
	{
	public:
		constexpr exception() noexcept = default;
		constexpr exception(exception const&) noexcept = default;
		constexpr exception& operator=(exception const&) noexcept = default;
		virtual constexpr ~exception() noexcept = default;
		virtual constexpr const char* what() const noexcept { return "std::exception"; }
	};
}
#endif