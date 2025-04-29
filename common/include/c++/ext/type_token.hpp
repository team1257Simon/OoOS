#ifndef __TYPE_TOKEN
#define __TYPE_TOKEN
#include "typeinfo"
namespace std
{
    namespace ext
    {
        template<typename T>
        struct type_token
        {
            type_info const* type;
            constexpr type_token() : type(addressof(typeid(T))) {}
            constexpr ~type_token() = default;
            constexpr bool operator==(type_info const& ti) const { return *type == ti; }
        };
    }
}
#endif