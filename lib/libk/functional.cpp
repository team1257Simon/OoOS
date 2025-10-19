#include "kernel/libk_decls.h"
#include "functional"
namespace std
{
    bad_function_call::~bad_function_call() noexcept = default;
    const char* bad_function_call::what() const noexcept { return "std::bad_function_call"; }
    [[noreturn]] void __throw_bad_function_call() { throw std::bad_function_call{}; }
}
bool await_result(std::function<bool()> const& fn, uint64_t max_dur) { for(uint64_t spin = 0UL; !fn() && spin < max_dur; barrier(), spin++); return fn(); }