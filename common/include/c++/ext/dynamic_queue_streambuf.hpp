#ifndef __GENERIC_BINARY_BUFFER
#define __GENERIC_BINARY_BUFFER
#include "bits/dynamic_queue.hpp"
#include "bits/char_traits.hpp"
#include "bits/basic_streambuf.hpp"
namespace std
{
    namespace ext
    {
        template<char_type CT, char_traits_type<CT> TT = char_traits<CT>, allocator_object<CT> AT = allocator<CT>>
        class dynamic_queue_streambuf : public virtual basic_streambuf<CT, TT>, protected __impl::__dynamic_queue<CT, AT>
        {
            typedef __impl::__dynamic_queue<CT, AT> __base;
            using typename __base::__ptr_container;
        public:
            typedef TT                              traits_type;
            typedef typename __base::__value_type   value_type;
            typedef typename __base::__alloc_type   allocator_type;
            typedef typename __base::__ptr          pointer;
            typedef typename __base::__const_ptr    const_pointer;
            typedef typename __base::__size_type    size_type;
            typedef typename __base::__diff_type    difference_type;
            typedef typename traits_type::pos_type  pos_type;
            typedef typename traits_type::off_type  off_type;
            constexpr dynamic_queue_streambuf() = default;
            dynamic_queue_streambuf(dynamic_queue_streambuf const&) = delete;
            dynamic_queue_streambuf(dynamic_queue_streambuf&& that) : __base{ forward<__base>(that) } {}
            dynamic_queue_streambuf(const_pointer start, const_pointer end, off_type n = static_cast<off_type>(0), size_type e = static_cast<size_type>(0)) : __base{ static_cast<size_type>(end - start) } { this->__qcopy(this->__qbeg(), start, static_cast<size_type>(end - start)); if(n > static_cast<off_type>(0)) this->__qsetn(static_cast<size_type>(n)); if(e) this->__qsete(e); }
            dynamic_queue_streambuf& operator=(dynamic_queue_streambuf const&) = delete;
            dynamic_queue_streambuf& operator=(dynamic_queue_streambuf&& that) { this->__qdestroy(); this->__qmove(forward<__base>(that)); }
            dynamic_queue_streambuf(size_type sz, allocator_type alloc = allocator_type{}) : __base{ sz, alloc } {}
        protected:
            virtual streamsize xsputn(const_pointer src,  streamsize n) override { pointer old_end = this->__end(); return size_type(this->__push_elements(src, src + n) - old_end); }
            virtual streamsize xsgetn(pointer dest, streamsize n) override { return this->__pop_elements(dest, dest + n); }
            virtual pos_type seekpos(pos_type pos, ios_base::openmode = ios_base::in) noexcept override { this->__qsetn(static_cast<size_type>(pos)); return pos_type(tell()); }
            virtual pos_type seekoff(off_type off, ios_base::seekdir way, ios_base::openmode = ios_base::in) noexcept override { this->__qsetn((way < 0 ? this->__qbeg() : (way > 0 ? this->__end() : this->__qcur())) + off); return pos_type(tell()); }
            virtual void on_modify_queue() override { this->sync(); }
            virtual int sync() override { this->setg(this->__qbeg(), this->__qcur(), this->__qmax()); this->__fullsetp(this->__qbeg(), this->__end(), this->__qmax()); return 0; }
        public:
            void advance_end(size_t added) { this->__bumpe(added); }
            bool reserve_at_least(size_t n) { return this->__qcapacity() >= n || this->__q_grow_buffer(static_cast<size_type>(n - this->__qcapacity())); }
            constexpr void clear() { this->__qclear(); }
            constexpr pointer data() noexcept { return this->__qbeg(); }
            constexpr const_pointer data() const noexcept { return this->__qbeg(); }
            constexpr pointer begin() noexcept { return this->__qbeg(); }
            constexpr const_pointer begin() const noexcept { return this->__qbeg(); }
            constexpr pointer cur() noexcept { return this->__qcur(); }
            constexpr const_pointer cur() const noexcept { return this->__qcur(); }
            constexpr pointer end() noexcept { return this->__end(); }
            constexpr const_pointer end() const noexcept { return this->__end(); }
            constexpr pointer max() noexcept { return this->__qmax(); }
            constexpr const_pointer max() const noexcept { return this->__qmax(); }
            constexpr size_type size() const noexcept { return this->__qsize(); }
            constexpr size_type capacity() const noexcept { return this->__qcapacity(); }
            constexpr off_type tell() const noexcept { return static_cast<off_type>(this->__tell()); }
            constexpr size_type rem() const noexcept { return this->__qrem(); }
            constexpr size_type avail_cap() const noexcept { return this->__cap_rem(); }
        }; 
    }
}
#endif