#include "module.hpp"
class test_module : public ooos_kernel_module::abstract_module_base
{
    const char* __my_message;
public:
    test_module(const char* msg);
    virtual void initialize() override;
    virtual void finalize() override;
    virtual ~test_module() = default;
};
test_module::test_module(const char* msg) : __my_message(msg) {}
void test_module::initialize() { this->log(__my_message); }
void test_module::finalize() {}
EXPORT_MODULE(test_module, "Hello module world!")