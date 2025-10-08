#include "module.hpp"
namespace ooos_kmod
{
    void abstract_module_base::on_unload() { exit(); __api_hooks->destroy_mm(__allocated_mm); }
    void abstract_module_base::on_load()
    {
        __api_hooks     = get_api_instance();
        __allocated_mm  = __api_hooks->create_mm();
        init();
    }
}
