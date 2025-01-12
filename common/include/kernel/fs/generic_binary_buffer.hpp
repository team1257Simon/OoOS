#ifndef __GENERIC_BINARY_BUFFER
#define __GENERIC_BINARY_BUFFER
#include "bits/dynamic_queue.hpp"
#include "bits/char_traits.hpp"
#include "bits/basic_streambuf.hpp"
// Intermediate buffer for abstracting file/disk IO operations. 
// Features "rebinds" which can be used to handle data IO in sizes wider than 8 bits (e.g. AHCI 16-bit IO) with a char8 buffer.
template<std::char_type CT, std::char_traits_type<CT> TT = std::char_traits<CT>, std::allocator_object<CT> AT = std::allocator<CT>>
class generic_binary_buffer : protected std::__impl::__dynamic_queue<CT, AT>, public virtual std::basic_streambuf<CT, TT>
{
    typedef typename std::__impl::__dynamic_queue<CT, AT> __base;
    using typename __base::__ptr_container;
    friend class ahci_hda;
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
    template<std::char_type DT> using rebind_pointer = std::__ptr_rebind<pointer, DT>;
    template<std::char_type DT> using const_rebind_pointer = std::add_const_t<std::__ptr_rebind<pointer, DT>>;
    constexpr generic_binary_buffer() = default;
    generic_binary_buffer(generic_binary_buffer const&) = delete;
    generic_binary_buffer(generic_binary_buffer&& that) : __base{ std::forward<__base>(that) } {}
    generic_binary_buffer(const_pointer start, const_pointer end, off_type n = size_type(0), size_type e = size_type(0)) : __base{ size_type(end - start) } { this->__qcopy(this->__qbeg(), start, size_type(end - start)); if(n > off_type(0)) this->__qsetn(size_type(n)); if(e) this->__qsete(e); }
    generic_binary_buffer& operator=(generic_binary_buffer const&) = delete;
    generic_binary_buffer& operator=(generic_binary_buffer&& that) { this->__qdestroy(); this->__qmove(std::forward<__base>(that)); }
    generic_binary_buffer(size_type sz, allocator_type alloc = allocator_type{}) : __base{ sz, alloc } {}
protected:
    virtual std::streamsize xsputn(const_pointer src,  std::streamsize n) override { pointer old_end = this->__end(); return size_type(this->__push_elements(src, src + n) - old_end); }
    virtual std::streamsize xsgetn(pointer dest, std::streamsize n) override { return this->__pop_elements(dest, dest + n); }
    virtual pos_type seekpos(pos_type pos, std::ios_base::openmode = std::ios_base::in) noexcept override { this->__qsetn(size_type(pos)); return pos_type(tell()); }
    virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode = std::ios_base::in) noexcept override { this->__qsetn((way < 0 ? this->__qbeg() : (way > 0 ? this->__end() : this->__qcur())) + off); return pos_type(tell()); }
    virtual void __q_on_modify() override { this->sync(); }
    void __update_end(size_t added) { this->__bumpe(added); }
    bool __ensure_capacity(size_t n) { return this->__qcapacity() >= n || this->__q_grow_buffer(size_type(n - this->__qcapacity())); }
    virtual int sync() override { this->setg(this->__qbeg(), this->__qcur(), this->__qmax()); this->__fullsetp(this->__qbeg(), this->__end(), this->__qmax()); return 0; }
public:
    // Copy-transfer the contents to a "normal" stream buffer.
    std::streamsize xfer(std::basic_streambuf<CT, TT>& that) const { return that.sputn(this->__qbeg(), this->__qsize()); }
    constexpr void clear() { this->__qclear(); }
    constexpr const_pointer data() const noexcept { return this->__qbeg(); }
    constexpr pointer beg() noexcept { return this->__qbeg(); }
    constexpr const_pointer beg() const noexcept { return this->__qbeg(); }
    constexpr pointer cur() noexcept { return this->__qcur(); }
    constexpr const_pointer cur() const noexcept { return this->__qcur(); }
    constexpr pointer end() noexcept { return this->__end(); }
    constexpr const_pointer end() const noexcept { return this->__end(); }
    constexpr pointer max() noexcept { return this->__qmax(); }
    constexpr const_pointer max() const noexcept { return this->__qmax(); }
    constexpr size_type size() const noexcept { return this->__qsize(); }
    constexpr size_type capacity() const noexcept { return this->__qcapacity(); }
    constexpr off_type tell() const noexcept { return off_type(this->__tell()); }
    constexpr size_type rem() const noexcept { return this->__qrem(); }
    constexpr size_type avail_cap() const noexcept { return this->__cap_rem(); }
    template<std::char_type DT> constexpr rebind_pointer<DT> rebind_beg() noexcept { return std::bit_cast<rebind_pointer<DT>>(beg()); }
    template<std::char_type DT> constexpr const_rebind_pointer<DT> rebind_beg() const noexcept { return std::bit_cast<const_rebind_pointer<DT>>(beg()); }
    template<std::char_type DT> constexpr rebind_pointer<DT> rebind_cur() noexcept { return std::bit_cast<rebind_pointer<DT>>(cur()); }
    template<std::char_type DT> constexpr const_rebind_pointer<DT> rebind_cur() const noexcept { return std::bit_cast<const_rebind_pointer<DT>>(cur()); }
    template<std::char_type DT> constexpr rebind_pointer<DT> rebind_end() noexcept { return std::bit_cast<rebind_pointer<DT>>(end()); }
    template<std::char_type DT> constexpr const_rebind_pointer<DT> rebind_end() const noexcept { return std::bit_cast<const_rebind_pointer<DT>>(end()); }
    template<std::char_type DT> constexpr rebind_pointer<DT> rebind_max() noexcept { return std::bit_cast<rebind_pointer<DT>>(max()); }
    template<std::char_type DT> constexpr const_rebind_pointer<DT> rebind_max() const noexcept { return std::bit_cast<const_rebind_pointer<DT>>(max()); }
    // Computes the size a buffer containing the exact same data as this buffer would have if it were of type DT.
    template<std::char_type DT> constexpr typename generic_binary_buffer<DT>::size_type size_as() const noexcept { return typename generic_binary_buffer<DT>::size_type(this->template rebind_end<DT>() - this->template rebind_beg<DT>()); }
    // Computes the capacity a buffer containing the exact same data as this buffer would have if it were of type DT.
    template<std::char_type DT> constexpr typename generic_binary_buffer<DT>::size_type capacity_as() const noexcept { return typename generic_binary_buffer<DT>::size_type(this->template rebind_max<DT>() - this->template rebind_beg<DT>()); }
    // Computes the current position of a theoretical buffer whose pointers point to the exact same location but whose type is DT.
    template<std::char_type DT> constexpr typename generic_binary_buffer<DT>::off_type tell_as() const noexcept { return typename generic_binary_buffer<DT>::off_type(this->template rebind_cur<DT>() - this->template rebind_beg<DT>()); }
    // Computes the number of remaining elements this buffer would have if they were interpreted as type DT.
    template<std::char_type DT> constexpr typename generic_binary_buffer<DT>::size_type rem_as() const noexcept { return typename generic_binary_buffer<DT>::size_type(this->template rebind_end<DT>() - this->template rebind_cur<DT>()); }
    /**
     * Inserts n elements of type DT into the buffer as follows:
     *  1. Creates a temporary buffer containing the same data as this buffer, with pointers computed to the same byte offsets, but with type DT.
     *  2. Inserts the elements by invoking sputn on the temporary buffer.
     *  3. Overwrites the contents of this buffer with the new, modified data.
     *  4. Updates the pointers in this buffer to match the byte offsets of the modified data pointers, growing the buffer if necessary.
     *  5. The temporary buffer will be deallocated as it leaves scope.
     */
    template<std::char_type DT, std::char_traits_type<DT> UT = std::char_traits<DT>, std::allocator_object<DT> BT = std::allocator<DT>> typename generic_binary_buffer<DT, UT, BT>::size_type rputn(typename generic_binary_buffer<DT, UT, BT>::const_pointer src, typename generic_binary_buffer<DT, UT, BT>::size_type n);
    /**
     * Extracts up to n elements of type DT from this buffer's data as follows:
     *  1. Creates a temporary buffer containing the same data as this buffer, with pointers computed to the same byte offsets, but with type DT.
     *  2. Extracts the elements by invoking sgetn on the temporary buffer.
     *  3. Adjusts the current-element pointer to match the new position of the temporary buffer after the extraction.
     *  4. The temporary buffer will be deallocated as it leaves scope.
     */
    template<std::char_type DT, std::char_traits_type<DT> UT = std::char_traits<DT>, std::allocator_object<DT> BT = std::allocator<DT>> typename generic_binary_buffer<DT, UT, BT>::size_type rgetn(typename generic_binary_buffer<DT, UT, BT>::pointer dest, typename generic_binary_buffer<DT, UT, BT>::size_type n);
    // A shortcut to this.sputn(that.rebind_beg<CT>(), that.size_as<CT>()) to alleviate the slightly odd syntax of such a call
    template<std::char_type DT, std::char_traits_type<DT> UT = std::char_traits<DT>, std::allocator_object<DT> BT = std::allocator<DT>> typename generic_binary_buffer<DT, UT, BT>::size_type rcopy(generic_binary_buffer<DT, UT, BT> const& that) { return this->sputn(that.template rebind_beg<CT>(), that.template size_as<CT>()); }
    template<std::char_type DT, std::char_traits_type<DT> UT = std::char_traits<DT>> std::streamsize rxfer(std::basic_streambuf<DT, UT>& that) const { return that.sputn(this->template rebind_beg<DT>(), this->template size_as<DT>()); }
};
template <std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
template <std::char_type DT, std::char_traits_type<DT> UT, std::allocator_object<DT> BT>
typename generic_binary_buffer<DT, UT, BT>::size_type generic_binary_buffer<CT, TT, AT>::rputn(typename generic_binary_buffer<DT, UT, BT>::const_pointer src, typename generic_binary_buffer<DT, UT, BT>::size_type n)
{
    if(!this->template capacity_as<DT>()) return typename generic_binary_buffer<DT, UT, BT>::size_type(0);
    generic_binary_buffer<DT, UT, BT> that{ this->template rebind_beg<DT>(), this->template rebind_max<DT>(), this->template tell_as<DT>(), this->template size_as<DT>() };
    typename generic_binary_buffer<DT, UT, BT>::size_type result = that.sputn(src, n);
    size_type nsz = that.template size_as<CT>();
    size_type ncap = that.template capacity_as<CT>();
    off_type np = that.template tell_as<CT>();
    if(ncap > this->__qcapacity() && !this->__q_grow_buffer(size_type(ncap - this->__qcapacity()))) return typename generic_binary_buffer<DT, UT, BT>::size_type(0);
    this->__qcopy(this->__qbeg(), that.template rebind_beg<CT>(), nsz);
    this->__qsetn(size_type(np));
    this->__qsete(nsz);
    return result;
}
template <std::char_type CT, std::char_traits_type<CT> TT, std::allocator_object<CT> AT>
template <std::char_type DT, std::char_traits_type<DT> UT, std::allocator_object<DT> BT>
typename generic_binary_buffer<DT, UT, BT>::size_type generic_binary_buffer<CT, TT, AT>::rgetn(typename generic_binary_buffer<DT, UT, BT>::pointer dest, typename generic_binary_buffer<DT, UT, BT>::size_type n)
{
    if(!this->template capacity_as<DT>()) return typename generic_binary_buffer<DT, UT, BT>::size_type(0);
    generic_binary_buffer<DT, UT, BT> that{ this->template rebind_beg<DT>(), this->template rebind_max<DT>(), this->template tell_as<DT>(), this->template size_as<DT>() };
    typename generic_binary_buffer<DT, UT, BT>::size_type result = that.sgetn(dest, n);
    this->__qsetn(size_type(that.template tell_as<CT>()));
    return result;
}
typedef generic_binary_buffer<uint8_t> binary_buffer;
typedef generic_binary_buffer<uint16_t> wide_binary_buffer;
#endif