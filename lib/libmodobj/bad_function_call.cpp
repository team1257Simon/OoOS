#include <functional>
namespace std
{
	bad_function_call::~bad_function_call() noexcept = default;
	const char* bad_function_call::what() const noexcept { return "std::bad_function_call"; }
	[[noreturn]] void __throw_bad_function_call() { throw std::bad_function_call{}; }
}