#ifndef __KMOD
#define __KMOD
#include "typeindex"
namespace ooos_kmod
{
    using namespace std::ext;
    class abstract_module_base
    {
        type_erasure __impl_type;
    protected:
        virtual void init() = 0;
        virtual void exit() = 0;
    public:
        virtual ~abstract_module_base() = 0;
        void post_load();
        void pre_unload();
        type_erasure const& get_type() const;
    };
}
#endif