#ifndef __ENTROPY_DEV
#define __ENTROPY_DEV
#include "ext/dynamic_queue_streambuf.hpp"
extern "C" void* __fill_entropy(void* buffer, size_t n_qwords);
constexpr size_t max_accumulation   = 0x100000;
constexpr size_t accumulation_rate  = 0x100;
class entropy_source : public std::ext::dynamic_queue_streambuf<uint8_t>
{
    using __base = std::ext::dynamic_queue_streambuf<uint8_t>;
protected:
    virtual void on_modify_queue() override;
    void accumulate();
public:
    entropy_source();
};
#endif