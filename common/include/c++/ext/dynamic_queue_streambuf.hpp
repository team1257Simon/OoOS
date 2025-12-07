#ifndef __GENERIC_BINARY_BUFFER
#define __GENERIC_BINARY_BUFFER
#include <bits/dynamic_queue.hpp>
#include <bits/char_traits.hpp>
#include <bits/basic_streambuf.hpp>
namespace std
{
	namespace ext
	{
		template<char_type CT, char_traits_type<CT> TT = char_traits<CT>, allocator_object<CT> AT = allocator<CT>>
		class dynamic_queue_streambuf : public virtual basic_streambuf<CT, TT>, protected std::__impl::__dynamic_queue<CT, AT>
		{
			typedef std::__impl::__dynamic_queue<CT, AT> __base;
			using typename __base::__ptr_container;
		public:
			typedef TT                              traits_type;
			typedef typename __base::__value_type   value_type;
			typedef typename __base::__allocator_type   allocator_type;
			typedef typename __base::__pointer          pointer;
			typedef typename __base::__const_pointer    const_pointer;
			typedef typename __base::__size_type    size_type;
			typedef typename __base::__difference_type    difference_type;
			typedef typename traits_type::pos_type  pos_type;
			typedef typename traits_type::off_type  off_type;
			constexpr dynamic_queue_streambuf() = default;
			dynamic_queue_streambuf(dynamic_queue_streambuf const& that) : __base(that) { this->sync(); }
			dynamic_queue_streambuf(dynamic_queue_streambuf&& that) : __base(forward<__base>(that)) {}
			dynamic_queue_streambuf(const_pointer start, const_pointer end, off_type n = 0L, size_type e = 0UZ) : __base(static_cast<size_type>(end - start)) { this->__qcopy(this->__qbeg(), start, static_cast<size_type>(end - start)); if(n > 0L) this->__qsetn(static_cast<size_type>(n)); if(e) this->__qsete(e); }
			dynamic_queue_streambuf& operator=(dynamic_queue_streambuf const& that) { this->__qdestroy(); this->__q_copy_assign(that); this->sync(); return *this; }
			dynamic_queue_streambuf& operator=(dynamic_queue_streambuf&& that) { this->__qdestroy(); this->__q_move_assign(forward<__base>(that)); this->sync(); return *this; }
			dynamic_queue_streambuf(size_type sz, allocator_type alloc = allocator_type{}) : __base(sz, alloc) {}
			virtual void reset() { this->__qrst(); }
			pointer data() { return this->__qbeg(); }
			const_pointer data() const { return this->__qbeg(); }
			streamsize count() const noexcept { return this->__qsize(); }
		protected:
			virtual streamsize xsputn(const_pointer src,  streamsize n) override { pointer old_end = this->__end(); return size_type(this->__push_elements(src, src + n) - old_end); }
			virtual streamsize xsgetn(pointer dest, streamsize n) override { return this->__pop_elements(dest, dest + n); }
			virtual pos_type seekpos(pos_type pos, ios_base::openmode = ios_base::in) noexcept override { this->__qsetn(static_cast<size_type>(pos)); return pos_type(this->__tell()); }
			virtual pos_type seekoff(off_type off, ios_base::seekdir way, ios_base::openmode = ios_base::in) noexcept override { this->__qsetn((way < 0 ? this->__qbeg() : (way > 0 ? this->__end() : this->__qcur())) + off); return pos_type(this->__tell()); }
			virtual void on_modify_queue() override { this->sync(); }
			virtual int sync() override { this->setg(this->__qbeg(), this->__qcur(), this->__qmax()); this->setp(this->__qbeg(), this->__end(), this->__qmax()); return 0; }
			virtual streamsize showmanyc() override { return this->__qrem(); }
		};
	}
}
#endif