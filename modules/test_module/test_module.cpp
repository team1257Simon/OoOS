#include "module.hpp"
class test_module : public ooos::abstract_module_base
{
    const char* __my_message;
public:
    test_module(const char* msg);
    virtual bool initialize() override;
    virtual void finalize() override;
    virtual ~test_module() = default;
};
test_module::test_module(const char* msg) : __my_message(msg) {}
bool test_module::initialize() { this->log(__my_message); return true; }
void test_module::finalize() {}
EXPORT_MODULE(test_module, "Hello module world!");