#ifndef __DUPLEX_STREAMBUF
#define __DUPLEX_STREAMBUF
#include "bits/basic_streambuf.hpp"
#include "bits/basic_string.hpp"
namespace std
{
    extension namespace ext
    {
        template<char_type CT, char_traits_type<CT> TT = char_traits<CT>, allocator_object<CT> AT = allocator<CT>>
        class dynamic_duplex_streambuf : public basic_streambuf<CT, TT>
        {
        protected:
            typedef basic_streambuf<CT, TT> __base;
            typedef std::__impl::__buf_ptrs<CT> __ptr_container;
            typedef AT __alloc_type;
            __alloc_type __allocator;
            bool __readonly_input = false;
        public:
            typedef typename __base::char_type char_type;
            typedef typename __base::int_type int_type;
            typedef typename __base::traits_type traits_type;
            typedef typename __base::pos_type pos_type;
            typedef typename __base::off_type off_type;
            typedef typename __ptr_container::__size_type size_type;
            typedef typename __ptr_container::__diff_type difference_type;
            typedef typename __ptr_container::__ptr pointer;
            typedef typename __ptr_container::__const_ptr const_pointer;
            typedef deref_t<pointer> reference;
            typedef deref_t<const_pointer> const_reference;
        protected:
            virtual void size(size_type target, ios_base::openmode which);
            virtual int_type overflow(int_type c) override;
            virtual int_type uflow() override;
            virtual int_type pbackfail(int_type c = traits_type::eof()) override;
            virtual pos_type seekoff(off_type off, ios_base::seekdir way, ios_base::openmode mode) override;
            virtual pos_type seekpos(pos_type pos, ios_base::openmode mode) override;
        public:
            virtual size_type size(ios_base::openmode which = ios_base::in | ios_base::out);
            virtual size_type count(ios_base::openmode which = ios_base::in | ios_base::out);
            virtual void expand(size_type amount, ios_base::openmode which = ios_base::in | ios_base::out);
            constexpr dynamic_duplex_streambuf() = default;
            virtual ~dynamic_duplex_streambuf();
        };
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        void dynamic_duplex_streambuf<CT, TT, AT>::size(size_type target, ios_base::openmode which)
        {
            if(which.in)
                this->__in_region.__set_ptrs(resize(this->__in_region.__begin, this->__in_region.__capacity(), target, __allocator), target);
            if(which.out)
                this->__out_region.__set_ptrs(resize(this->__out_region.__begin, this->__out_region.__capacity(), target, __allocator), target);
        }
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        typename dynamic_duplex_streambuf<CT, TT, AT>::size_type dynamic_duplex_streambuf<CT, TT, AT>::size(ios_base::openmode which)
        {
            size_type result = 0UZ;
            if(which.in)
                result += this->__in_region.__capacity();
            if(which.out)
                result += this->__out_region.__capacity();
            return result;
        }
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        typename dynamic_duplex_streambuf<CT, TT, AT>::size_type dynamic_duplex_streambuf<CT, TT, AT>::count(ios_base::openmode which)
        {
            size_type result = 0UZ;
            if(which.in)
                result += static_cast<size_type>(this->__in_region.__end - this->__in_region.__begin);
            if(which.out)
                result += static_cast<size_type>(this->__out_region.__end - this->__out_region.__begin);
            return result;
        }
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        void dynamic_duplex_streambuf<CT, TT, AT>::expand(size_type amount, ios_base::openmode which)
        {
            if(which.in)
                size(this->__in_region.__capacity() + amount, ios_base::in);
            if(which.out)
                size(this->__out_region.__capacity() + amount, ios_base::out);
        }
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        typename dynamic_duplex_streambuf<CT, TT, AT>::int_type dynamic_duplex_streambuf<CT, TT, AT>::overflow(int_type c)
        {
            try { expand(max(1UZ, this->__out_region.__capacity()), ios_base::out); } catch(...) { return traits_type::eof(); }
            *this->pptr() = traits_type::to_char_type(c);
            this->pbump(1);
            return traits_type::not_eof(c);
        }
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        typename dynamic_duplex_streambuf<CT, TT, AT>::int_type dynamic_duplex_streambuf<CT, TT, AT>::uflow()
        {
            int_type ret = traits_type::eof();
            streamsize s = this->showmanyc();
            if(!s) return ret;
            try { expand(s, ios_base::in); } catch(...) { return ret; }
            const bool testeof = traits_type::eq_int_type(this->underflow(), ret); 
            if(testeof) return ret;
            ret = traits_type::to_int_type(*this->gptr());
            this->gbump(1);
            return ret;
        }
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        typename dynamic_duplex_streambuf<CT, TT, AT>::int_type dynamic_duplex_streambuf<CT, TT, AT>::pbackfail(int_type c)
        {
            if(pointer p = this->gptr(); __builtin_expect(p && this->eback() < p--, true))
            {
                if(!traits_type::eq_int_type(traits_type::to_int_type(*p), c))
                {
                    if(__readonly_input)
                        return traits_type::eof();
                    *p = traits_type::to_char_type(c);
                }
                this->gbump(-1);
                return traits_type::not_eof(c);
            }
            return traits_type::eof();
        }
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        typename dynamic_duplex_streambuf<CT, TT, AT>::pos_type dynamic_duplex_streambuf<CT, TT, AT>::seekoff(off_type off, ios_base::seekdir way, ios_base::openmode mode)
        {
            off_type result = 0L;
            if(mode.in)
            {
                pointer inptr = (way < 0 ? this->eback() : way > 0 ? this->egptr() : this->gptr()) + off;
                if(inptr > this->egptr()) return pos_type(off_type(-1));
                this->__in_region.__setc(inptr);
                result = this->__in_region.__end - this->__in_region.__begin;
            }
            if(mode.out)
            {
                pointer outptr = (way < 0 ? this->pbase() : way > 0 ? this->epptr() : this->pptr()) + off;
                if(outptr > this->epptr()) return pos_type(off_type(-1));
                this->__out_region.__setc(outptr);
                result = this->__out_region.__end - this->__out_region.__begin;
            }
            if(mode.in && mode.out) return pos_type(off_type(0));
            return pos_type(result);
        }
        template<char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        typename dynamic_duplex_streambuf<CT, TT, AT>::pos_type dynamic_duplex_streambuf<CT, TT, AT>::seekpos(pos_type pos, ios_base::openmode mode)
        {
            off_type result = 0L;
            if(mode.in)
            {
                pointer inptr = this->eback() + pos;
                if(__unlikely(inptr > this->egptr())) return pos_type(off_type(-1));
                this->__in_region.__setc(inptr);
                result = this->__in_region.__end - this->__in_region.__begin;
            }
            if(mode.out)
            {
                pointer outptr = this->pbase() + pos;
                if(__unlikely(outptr > this->epptr())) return pos_type(off_type(-1));
                this->__out_region.__setc(outptr);
                result = this->__out_region.__end - this->__out_region.__begin;
            }
            if(__unlikely(mode.in && mode.out)) return pos_type(off_type(0));
            return pos_type(result);
        }
        template <char_type CT, char_traits_type<CT> TT, allocator_object<CT> AT>
        dynamic_duplex_streambuf<CT, TT, AT>::~dynamic_duplex_streambuf()
        { 
            if(this->__in_region.__begin)
                __allocator.deallocate(this->__in_region.__begin, this->__in_region.__capacity());
            if(this->__out_region.__begin)
                __allocator.deallocate(this->__out_region.__begin, this->__out_region.__capacity());
        }
    }
}
#endif