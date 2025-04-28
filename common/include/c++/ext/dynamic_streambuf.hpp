#ifndef __DATA_BUFFER
#define __DATA_BUFFER
#include "bits/basic_streambuf.hpp"
#include "bits/basic_string.hpp"
namespace std
{
    extension namespace ext
    {
        template<std::char_type CT, std::char_traits_type<CT> TT = std::char_traits<CT>, std::allocator_object<CT> AT = std::allocator<CT>>
        class dynamic_streambuf : public virtual std::basic_streambuf<CT, TT>, protected std::__impl::__dynamic_buffer<CT, AT>
        {
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
            bool is_dirty{ false };
            virtual std::basic_string<CT, TT, AT> __str() const& { return std::basic_string<CT, TT, AT>{ this->__beg(), this->__max() }; }
            virtual std::basic_string<CT, TT, AT> __str() && { size_t s = this->__capacity(); std::basic_string<CT, TT, AT> result{ this->__beg(), this->__max() }; this->__clear(); this->__allocate_storage(s); this->on_modify(); return result; }
            virtual void __str(std::basic_string<CT, TT, AT> const& that) { this->__clear(); this->__allocate_storage(that.size() + 1); this->__copy(this->__beg(), that.data(), that.size()); this->__advance(that.size()); this->on_modify(); }
            virtual void __str(std::basic_string<CT, TT, AT> && that) { this->__clear(); this->__allocate_storage(that.size() + 1); this->__copy(this->__beg(), that.data(), that.size()); this->__advance(that.size()); that.clear(); this->on_modify();  }
            virtual int write_dev() { return 0; } // Sync the put region to the file if this stream is output-buffered.
            virtual std::streamsize read_dev(std::streamsize) { return std::streamsize(0); } // Get more bytes from the file if there are any left.
            virtual std::streamsize unread_size() { return 0; } // How many unread bytes are in the file's data sequence.
            virtual std::streamsize sector_size() { return physical_block_size; }
            virtual std::streamsize showmanyc() override { return unread_size(); }
            virtual std::streamsize on_overflow(std::streamsize n) { if(this->__grow_buffer(n)) return n; return 0; }
            /**
             * Called whenever the end and/or max pointers are changed after initial construction, other than through the advance and backtrack hooks.
             * Inheritors can override to add functionality that needs to be invoked whenever these pointers move.
             */
            virtual void on_modify() { if(this->__beg()) { this->__fullsetp(this->__beg(), this->__cur(), this->__max()); is_dirty = true; } }
            virtual int sync() override { if(this->__beg()) this->__fullsetp(this->__beg(), this->__cur(), this->__max()); if(is_dirty) { int result = write_dev(); is_dirty = (result >= 0); return result; } return 0; }
            virtual __sb_type* setbuf(char_type* s, std::streamsize n) { this->__setn(s, n); return this; }
            virtual int_type underflow() override { std::streamsize n = std::min(sector_size(), showmanyc()); if(n && read_dev(n)) { this->on_modify(); return traits_type::to_int_type(*this->gptr()); } return traits_type::eof(); } 
            virtual int_type overflow(int_type c = traits_type::eof()) override { if(!traits_type::eq_int_type(c, traits_type::eof())) { if(this->__append_element(traits_type::to_char_type(c))) { this->on_modify(); return c; } } return traits_type::eof(); }
            virtual int_type pbackfail(int_type c = traits_type::eof()) override { if(!traits_type::eq_int_type(c, traits_type::eof())) { this->gbump(-1); *this->gptr() = traits_type::to_char_type(c); this->on_modify(); return c; } return traits_type::eof(); }
            virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
            virtual pos_type seekpos(pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
            virtual std::streamsize xsgetn(char_type* s, std::streamsize n) override;
            virtual std::streamsize xsputn(char_type const* s, std::streamsize n) override;
        public:
            dynamic_streambuf(AT const& alloc = AT{}) : __sb_type{}, __dynamic_base{ alloc } {};
            dynamic_streambuf(std::size_t how_much, AT const& alloc = AT{}) : __sb_type{}, __dynamic_base{ how_much, alloc } { this->on_modify(); }
            dynamic_streambuf(dynamic_streambuf const&) = delete;
            dynamic_streambuf(dynamic_streambuf&& that) : __sb_type{}, __dynamic_base{ std::move(that) } { this->on_modify(); }
            virtual ~dynamic_streambuf() = default;
            dynamic_streambuf& operator=(dynamic_streambuf const&) = delete;
            dynamic_streambuf& operator=(dynamic_streambuf&& that) { this->__realloc_move(std::move(that)); return *this; }
            pos_type tell() const noexcept { return pos_type(this->__cur() - this->__beg()); }
            char_type* data() noexcept { return this->__beg(); }
            char_type const* data() const noexcept { return this->__beg(); }
            void clear() { this->__clear(); }
            std::streamsize count() const noexcept { return this->__size(); }
        };
        template<std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
        typename dynamic_streambuf<CT, TT, AT>::pos_type dynamic_streambuf<CT, TT, AT>::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
        {
            char_type* ptr = (way == std::ios_base::cur ? this->__cur() : (way == std::ios_base::end ? this->__max() : this->__beg())) + off;
            if(ptr > this->__max()) return pos_type(off_type(-1));
            if(__builtin_expect(which.in || which.out, true) && ptr != this->__cur())
            {
                this->__setc(ptr);
                if(which.in) this->__in_region.__end = ptr;
                if(which.out) this->__out_region.__end = ptr;
                this->on_modify();
            }
            return pos_type(off_type(this->__size()));
        }
        template<std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
        typename dynamic_streambuf<CT, TT, AT>::pos_type dynamic_streambuf<CT, TT, AT>::seekpos(pos_type pos, std::ios_base::openmode which)
        {
            char_type* ptr = this->__get_ptr(pos);
            if(ptr > this->__max()) return pos_type(off_type(-1));
            if(__builtin_expect(which.in || which.out, true) && ptr != this->__cur())
            {
                this->__setc(ptr);
                if(which.in) this->__in_region.__end = this->__cur();
                if(which.out) this->__out_region.__end = this->__cur();
                this->on_modify();
            }
            return pos_type(off_type(this->__size()));
        }
        template <std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
        std::streamsize dynamic_streambuf<CT, TT, AT>::xsgetn(char_type *s, std::streamsize n)
        {
            std::streamsize l = std::min(n, std::streamsize(this->egptr() - this->gptr()));
            if(l < n) { l += this->read_dev(std::min(std::streamsize(l - n), this->unread_size())); }
            if(l) array_copy(s, this->gptr(), l);
            this->gbump(l);
            if(l) this->on_modify();
            return l;
        }
        template <std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
        std::streamsize dynamic_streambuf<CT, TT, AT>::xsputn(char_type const *s, std::streamsize n)
        {
            std::streamsize l = std::min(n, std::streamsize(this->__max() - this->__cur()));
            if(l < n) { l += this->on_overflow(std::streamsize(n - l)); }
            char_type* old = this->__cur();
            char_type* result = this->__append_elements(s, s + l);
            if(!result) return std::streamsize(0);
            this->on_modify();
            return std::streamsize(result - old);
        }
#ifndef INST
        extern template class dynamic_streambuf<char>;
        extern template class dynamic_streambuf<uint8_t>;
#endif
    }
}
#endif