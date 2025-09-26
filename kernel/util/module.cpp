#include "module.hpp"
namespace ooos_kmod
{
    void abstract_module_base::post_load() { new(std::addressof(__impl_type)) type_erasure(typeid(*this)); init(); }
    type_erasure const& abstract_module_base::get_type() const { return __impl_type; }
    void ooos_kmod::abstract_module_base::pre_unload() { exit(); }
}