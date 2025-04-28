#define INST
#include "ext/dynamic_queue_streambuf.hpp"
#include "ext/dynamic_streambuf.hpp"
namespace std
{
    template class __impl::__dynamic_buffer<char, std::allocator<char>, true>;
    template class __impl::__dynamic_queue<char, std::allocator<char>>;
    template class ext::dynamic_queue_streambuf<char>;
    template class ext::dynamic_queue_streambuf<uint8_t>;
    template class ext::dynamic_streambuf<char>;
    template class ext::dynamic_streambuf<uint8_t>;
    template class basic_streambuf<char>;
}