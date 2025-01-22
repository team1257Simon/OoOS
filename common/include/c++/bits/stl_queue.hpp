#ifndef __STL_QUEUE
#define __STL_QUEUE
#include "bits/dynamic_queue.hpp"
namespace std
{
    extension namespace ext
    {
        template<typename T, allocator_object<T> A>
        struct stale_settings
        {
            unsigned int op_threshold;
            typename __impl::__dynamic_queue<T, A>::__diff_type size_threshold;
        };
        template<typename T, allocator_object<T> A = std::allocator<T>>
        class resettable_queue : protected __impl::__dynamic_queue<T, A>
        {
            typedef __impl::__dynamic_queue<T, A> __base;
            bool __enable_trim_stale{ false };
            // copy- and move-assign and construct functions implicitly inherit from base
        protected:
            virtual void __q_on_modify() override { if(__enable_trim_stale) { this->__trim_stale(); } }
        public:
            typedef T value_type;
            typedef typename __base::__alloc_type allocator_type;
            typedef typename __base::__size_type size_type;
            typedef typename __base::__diff_type difference_type;
            typedef typename __base::__ref reference;
            typedef typename __base::__const_ref const_reference;
            typedef typename __base::__ptr pointer;
            typedef typename __base::__const_ptr const_pointer;
            typedef ::__impl::__iterator<pointer, resettable_queue> iterator;
            typedef ::__impl::__iterator<const_pointer, resettable_queue> const_iterator;
            typedef std::reverse_iterator<iterator> reverse_iterator;
            typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
            constexpr resettable_queue() noexcept : __base{} {}
            constexpr resettable_queue(size_t st_cap, allocator_type const& alloc = allocator_type{}) : __base{ st_cap, alloc } {}
            constexpr size_type size() const noexcept { return this->__qrem(); }
            constexpr bool empty() const noexcept { return !this->__qsize(); }
            constexpr bool at_end() const noexcept { return !this->__qrem(); }
            constexpr void restart() noexcept { this->__qrst(); }
            constexpr reference front() { return *(this->__qbeg()); }
            constexpr const_reference front() const { return *(this->__qbeg()); }
            constexpr reference next() { return *(this->__qcur()); }
            constexpr const_reference next() const { return *(this->__qcur()); }
            constexpr reference back() { return *(this->__end() - 1); }
            constexpr const_reference back() const { return *(this->__end() - 1); }
            constexpr iterator begin() { return iterator{ this->__qbeg() }; }
            constexpr const_iterator cbegin() const { return const_iterator{ this->__qbeg() }; }
            constexpr const_iterator begin() const { return cbegin(); }
            extension constexpr iterator current() { return iterator{ this->__qcur() }; }
            extension constexpr const_iterator ccurrent() const { return const_iterator{this->__qcur() }; }
            extension constexpr const_iterator current() const { return ccurrent(); }
            constexpr iterator end() { return iterator{ this->__end() }; }
            constexpr const_iterator cend() const { return const_iterator{ this->__end() }; }
            constexpr const_iterator end() const { return cend(); }
            constexpr reverse_iterator rbegin() { return reverse_iterator{ begin() }; }
            constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator{ cbegin() }; }
            constexpr const_reverse_iterator rbegin() const { return crbegin(); }
            extension constexpr reverse_iterator rcurrent() { return reverse_iterator{ current() }; }
            extension constexpr const_reverse_iterator crcurrent() const { return const_reverse_iterator{ ccurrent() }; }
            extension constexpr const_reverse_iterator rcurrent() const { return crcurrent(); }
            constexpr reverse_iterator rend() { return reverse_iterator{ end() }; }
            constexpr const_reverse_iterator crend() const { return const_reverse_iterator{ cend() }; }
            constexpr const_reverse_iterator rend() const { return crend(); }
            constexpr void push(value_type const& value) { this->__push_elements(value, 1UL); }
            constexpr void push(value_type&& value) { this->__push_elements(move(value), 1UL); }
            constexpr void swap(resettable_queue& that) noexcept { this->__qswap(that); }            
            template<typename ... Args> requires std::constructible_from<T, Args...> constexpr reference emplace(Args&& ... args) { return *(this->__emplace_element(forward<Args>(args)...)); }
            constexpr reference pop() { pointer result = this->__pop_next(); if(result) return *result; throw std::out_of_range{ "nothing to pop" }; }
            constexpr size_type erase(const_iterator start, const_iterator end) { return this->__erase_elements(start.base(), std::distance(start, end)); }
            constexpr size_type erase(const_iterator where) { return this->__erase_elements(where.base()); }
            extension constexpr pointer unpop() noexcept { return this->__unpop(); }
            extension constexpr size_type trim() { return this->__force_trim(); }
            extension constexpr void set_trim_stale(stale_settings<T, A> const& settings) noexcept { __enable_trim_stale = true; this->__set_stale_op_threshold(settings.op_threshold); this->__set_stale_size_threshold(settings.size_threshold); }
            extension constexpr size_type transfer(resettable_queue& to_whom, const_iterator start, const_iterator end) { to_whom.__push_elements(start.base(), end.base()); return erase(start, end); }
            extension constexpr size_type transfer(resettable_queue& to_whom, const_iterator what) { to_whom.__push_elements(what.base()); return erase(what); }
            extension constexpr size_type transfer(resettable_queue& to_whom, size_type how_many) { to_whom.__push_elements(this->__qcur(), this->__qcur() + how_many); return this->__erase_elements(this->__qcur(), how_many); }
        };
    }
}
#endif