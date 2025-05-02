#ifndef __BASIC_STREAMBUF
#define __BASIC_STREAMBUF
#include "bits/dynamic_buffer.hpp"
#include "bits/char_traits.hpp"
namespace std
{
    template<std::char_type CT, std::char_traits_type<CT> TT = std::char_traits<CT>>
    class basic_streambuf
    {
        friend class device_node;
    public:
        typedef CT                      char_type;
        typedef TT                      traits_type;
        typedef typename TT::int_type   int_type;
        typedef typename TT::pos_type   pos_type;
        typedef typename TT::off_type   off_type;
        typedef basic_streambuf<char_type, traits_type> __sb_type;
    protected:
        using __ptr_container = typename std::__impl::__buf_ptrs<CT>;
        __ptr_container __in_region;
        __ptr_container __out_region;
        basic_streambuf() : __in_region{}, __out_region{} {}
        basic_streambuf(basic_streambuf const&) = default;
        basic_streambuf& operator=(basic_streambuf const&) = default;
        char_type* eback() const { return __in_region.__begin;  }
        char_type* gptr()  const { return __in_region.__end;    }
        char_type* egptr() const { return __in_region.__max;    }
        void gbump(int n)        { __in_region.__end += n;      }
        char_type* pbase() const { return __out_region.__begin; }
        char_type* pptr()  const { return __out_region.__end;   }
        char_type* epptr() const { return __out_region.__max;   }
        void pbump(int n)        { __out_region.__end += n;     }
        void setp(char_type* pbeg, char_type* pend)                         { __out_region.__set_ptrs(pbeg, pbeg, pend); }
        extension void __fullsetp(char_type* pbeg, char_type* pcur, char_type* pend)  { __out_region.__set_ptrs(pbeg, pcur, pend); }
        void setg(char_type* gbeg, char_type* gnext, char_type* gend)       { __in_region.__set_ptrs(gbeg, gnext, gend); }
        virtual basic_streambuf<char_type, traits_type>* setbuf(char_type*, streamsize) { return this; }
        virtual pos_type seekoff(off_type, ios_base::seekdir, ios_base::openmode = ios_base::in | ios_base::out) { return pos_type(off_type(-1)); }
        virtual pos_type seekpos(pos_type, ios_base::openmode = ios_base::in | ios_base::out) { return pos_type(off_type(-1)); }
        virtual int sync() { return 0; }
        virtual streamsize showmanyc() { return 0; }
        virtual int_type underflow() { return traits_type::eof(); }
        virtual int_type uflow() { int_type ret = traits_type::eof(); const bool testeof = traits_type::eq_int_type(this->underflow(), ret); if(!testeof) { ret = traits_type::to_int_type(*this->gptr()); this->gbump(1); } return ret; }
        virtual int_type pbackfail(int_type c = traits_type::eof()) { return traits_type::eof(); }
        virtual int_type overflow(int_type c = traits_type::eof()) { return traits_type::eof(); }
        void swap(basic_streambuf& that) { __in_region.__swap_ptrs(that.__in_region); __out_region.__swap_ptrs(that.__out_region); }
        virtual streamsize xsgetn(char_type* s, streamsize n);
        virtual streamsize xsputn(char_type const* s, streamsize n);
    public:
        virtual ~basic_streambuf() {}
        basic_streambuf* pubsetbuf(char_type* s, streamsize n) { return this->setbuf(s, n); }
        pos_type pubseekoff(off_type off, ios_base::seekdir way, ios_base::openmode mode = ios_base::in | ios_base::out) { return this->seekoff(off, way, mode); }
        pos_type pubseekpos(pos_type pos, ios_base::openmode mode = ios_base::in | ios_base::out) { return this->seekpos(pos, mode); }
        int pubsync() { return this->sync(); }
        streamsize in_avail() { const streamsize result = this->egptr() - this->gptr(); return result ? result : this->showmanyc(); }
        int_type sbumpc() { int_type result; if(__builtin_expect(this->gptr() < this->egptr(), true)) { result = traits_type::to_int_type(*this->gptr()); this->gbump(1); } else result = this->uflow(); return result; }
        int_type sgetc() { int_type result; if(__builtin_expect(this->gptr() < this->egptr(), true)) { result = traits_type::to_int_type(*this->gptr()); } else result = this->underflow(); return result; }
        int_type snextc() { int_type result = traits_type::eof(); if(__builtin_expect(!traits_type::eq_int_type(this->sbumpc(), result), true)) result = this->sgetc(); return result; }
        streamsize sgetn(char_type* s, streamsize n) { return this->xsgetn(s, n); }
        int_type sputbackc(char_type c) { int_type result; const bool testpos = this->eback() < this->gptr(); if (__builtin_expect(!testpos || !traits_type::eq(c, this->gptr()[-1]), false)) result = this->pbackfail(traits_type::to_int_type(c)); else { this->gbump(-1); result = traits_type::to_int_type(*this->gptr()); } return result; }
        int_type sungetc() { int_type result; if (__builtin_expect(this->eback() < this->gptr(), true)) { this->gbump(-1); result = traits_type::to_int_type(*this->gptr()); } else result = this->pbackfail(); return result; }
        int_type sputc(char_type c) { int_type result; if (__builtin_expect(this->pptr() < this->epptr(), true)) { *this->pptr() = c; this->pbump(1); result = traits_type::to_int_type(c); } else result = this->overflow(traits_type::to_int_type(c)); return result; }
        streamsize sputn(char_type const* s, streamsize n) { return this->xsputn(s, n); }
        extension char_type* in_data() { return eback(); }
        extension char_type* out_data() { return pbase(); }
    };
    template <std::char_type CT, std::char_traits_type<CT> TT>
    streamsize std::basic_streambuf<CT, TT>::xsgetn(char_type *s, streamsize n)
    {
        streamsize i = 0;
        while(i < n)
        {
            const streamsize buf_len = this->egptr() - this->gptr();
            if(buf_len)
            {
                const streamsize rem = n - i;
                const streamsize len = std::min(buf_len, rem);
                traits_type::copy(s, this->gptr(), len);
                i += len;
                s += len;
                __in_region.__adv(n);
            }
            if(i >= n) break;
            const int_type c = this->uflow();
            if(!traits_type::eq_int_type(c, traits_type::eof())) { traits_type::assign(*s++, traits_type::to_char_type(c)); i++; }
        }
        return i;
    }
    template <std::char_type CT, std::char_traits_type<CT> TT>
    streamsize std::basic_streambuf<CT, TT>::xsputn(char_type const* s, streamsize n)
    {
        streamsize i = 0;
        while(i < n)
        {
            const streamsize buf_len = this->epptr() - this->pptr();
            if(buf_len)
            {
                const streamsize rem = n - i;
                const streamsize len = std::min(buf_len, rem);
                traits_type::copy(this->pptr(), s, len);
                i += len;
                s += len;
                __out_region.__adv(n);
            }
            if(i >= n) break;
            const int_type c = this->overflow(*s);
            if(!traits_type::eq_int_type(c, traits_type::eof())) { s++; i++; }
        }
        return i;
    }
}
#endif