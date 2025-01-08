#ifndef __DATA_BUFFER
#define __DATA_BUFFER
#include "bits/basic_streambuf.hpp"
#include "bits/basic_string.hpp"
template<std::char_type CT, std::char_traits_type<CT> TT = std::char_traits<CT>, std::allocator_object<CT> AT = std::allocator<CT>>
class data_buffer : public std::basic_streambuf<CT, TT>, protected std::__impl::__dynamic_buffer<CT, AT>
{
    friend void buffer_test();
    using __dynamic_base = typename std::__impl::__dynamic_buffer<CT, AT>;
public:
    using typename std::basic_streambuf<CT, TT>::char_type;
    using typename std::basic_streambuf<CT, TT>::traits_type;
    using typename std::basic_streambuf<CT, TT>::pos_type;
    using typename std::basic_streambuf<CT, TT>::off_type;
    using typename std::basic_streambuf<CT, TT>::int_type;
protected:
    using typename std::basic_streambuf<CT, TT>::__sb_type;
    using typename std::basic_streambuf<CT, TT>::__ptr_container;
    virtual std::basic_string<CT, TT, AT> __str() const& { return std::basic_string<CT, TT, AT> { this->__beg(), this->__max() }; }
    virtual std::basic_string<CT, TT, AT> __str() && { size_t s = this->__capacity(); std::basic_string<CT, TT, AT> result { this->__beg(), this->__max() }; this->__clear(); this->__allocate_storage(s); this->__on_modify(); return result; }
    virtual void __str(std::basic_string<CT, TT, AT> const& that) { this->__clear(); this->__allocate_storage(that.size() + 1); this->__copy(this->__beg(), that.data(), that.size()); this->__advance(that.size()); this->__on_modify(); }
    virtual void __str(std::basic_string<CT, TT, AT> && that) { this->__clear(); this->__allocate_storage(that.size() + 1); this->__copy(this->__beg(), that.data(), that.size()); this->__advance(that.size()); that.clear(); this->__on_modify();  }
    virtual void __on_modify() override { if(this->__beg()) { this->setg(this->__beg(), this->__cur(), this->__max()); this->__fullsetp(this->__beg(), this->pptr(), this->__max()); } }
    virtual int sync() override { this->__on_modify(); return 0; }
    virtual __sb_type* setbuf(char_type* s, std::streamsize n) { this->__setn(s, n); return this; }
    virtual int_type overflow(int_type c = traits_type::eof()) override { if(!traits_type::eq_int_type(c, traits_type::eof())) { if(this->__append_element(traits_type::to_char_type(c))) return c; } return traits_type::eof(); }
    virtual int_type pbackfail(int_type c = traits_type::eof()) override { if(!traits_type::eq_int_type(c, traits_type::eof())) { this->gbump(-1); *this->gptr() = traits_type::to_char_type(c); return c; } return traits_type::eof(); }
    virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    virtual pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    virtual std::streamsize xsgetn(char_type *s, std::streamsize n) override;
    virtual std::streamsize xsputn(char_type const* s, std::streamsize n) override;
public:
    data_buffer(AT const& alloc = AT{}) : __sb_type{}, __dynamic_base{ alloc } {};
    data_buffer(std::size_t how_much, AT const& alloc = AT{}) : __sb_type{}, __dynamic_base{ how_much, alloc } { this->__on_modify(); }
    data_buffer(data_buffer const&) = delete;
    data_buffer(data_buffer&& that) : __sb_type{}, __dynamic_base{ std::move(that) } { this->__on_modify(); }
    data_buffer& operator=(data_buffer const&) = delete;
    data_buffer& operator=(data_buffer&& that) { this->__realloc_move(std::move(that)); return *this; }
};
template<std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
typename data_buffer<CT, TT, AT>::pos_type data_buffer<CT, TT, AT>::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
    char_type* ptr = (way == std::ios_base::cur ? this->__cur() : (way == std::ios_base::end ? this->__max() : this->__beg())) + off;
    if(__builtin_expect(which.in || which.out, true))
    {
        this->__setc(ptr);
        if(which.in) this->__in_region.__end = ptr;
        if(which.out) this->__out_region.__end = ptr;
    }
    return pos_type(off_type(this->__size()));
}
template <std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
typename data_buffer<CT, TT, AT>::pos_type data_buffer<CT, TT, AT>::seekpos(pos_type pos, std::ios_base::openmode which)
{
    if(__builtin_expect(which.in || which.out, true))
    {
        this->__bumpc(off_type(pos));
        if(which.in) this->__in_region.__end = this->__cur();
        if(which.out) this->__out_region.__end = this->__cur();
    }
    return pos_type(off_type(this->__size()));
}
template <std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
std::streamsize data_buffer<CT, TT, AT>::xsgetn(char_type *s, std::streamsize n)
{
    std::streamsize l = std::min(n, std::streamsize(this->egptr() - this->gptr()));
    if(l) traits_type::copy(s, this->gptr(), l);
    this->gbump(l);
    return l;
}
template <std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
std::streamsize data_buffer<CT, TT, AT>::xsputn(char_type const *s, std::streamsize n)
{
    char_type* old = this->__cur();
    char_type* result = this->__append_elements(s, s + n);
    if(!result) return std::streamsize(0);
    return std::streamsize(result - old);
}
#endif