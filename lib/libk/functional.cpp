#include "kernel/libk_decls.h"
#include "functional"
namespace std
{
    bad_function_call::~bad_function_call() noexcept = default;
    const char* bad_function_call::what() const noexcept { return "std::bad_function_call"; }
    __isrcall [[noreturn]] void __throw_bad_function_call() { panic("(DBG) bad_function_call"); throw std::bad_function_call{}; }
}