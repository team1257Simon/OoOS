#ifndef __STL_QUEUE
#define __STL_QUEUE
#include "bits/dynamic_queue.hpp"
namespace std
{
    namespace ext
    {
        template<typename T, allocator_object<T> A>
        struct stale_settings
        {
            unsigned int op_threshold;
            typename __impl::__dynamic_queue<T, A>::__diff_type size_threshold;
        };
        template<typename T, allocator_object<T> A = std::allocator<T>>
        class single_queue : protected __impl::__dynamic_queue<T, A>
        {
            typedef __impl::__dynamic_queue<T, A> __base;
            bool __enable_trim_stale{ false };
            // copy- and move-assign and construct functions implicitly inherit from base
        protected:
            virtual void __q_on_modify() override { if(__enable_trim_stale) this->__trim_stale(); }
        public:
            typedef T value_type;
            typedef typename __base::__alloc_type allocator_type;
            typedef typename __base::__size_type size_type;
            typedef typename __base::__diff_type difference_type;
            typedef typename __base::__ref reference;
            typedef typename __base::__const_ref const_reference;
            typedef typename __base::__ptr pointer;
            typedef typename __base::__const_ptr const_pointer;
            constexpr single_queue() noexcept = default;
            constexpr single_queue(size_t st_cap, allocator_type const& alloc = allocator_type{}) : __base{ st_cap, alloc } {}
            constexpr size_type size() const noexcept { return this->__qrem(); }
            constexpr bool empty() const noexcept { return !this->__qsize(); }
            constexpr bool at_end() const noexcept { return !this->__qrem(); }
            constexpr void restart() noexcept { this->__qrst(); }
            constexpr reference front() { return *(this->__qcur()); }
            constexpr const_reference front() const { return *(this->__qcur()); }
            constexpr reference back() { return *(this->__end() - 1); }
            constexpr const_reference back() const { return *(this->__end() - 1); }
            constexpr void push(value_type const& value) { this->__push_elements(value); }
            constexpr void push(value_type&& value) { this->__push_elements(move(value)); }
            template<typename ... Args> requires std::constructible_from<T, Args...> constexpr reference emplace(Args&& ... args) { return *(this->__emplace_element(forward<Args>(args)...)); }
            constexpr reference pop() { return *(this->__pop_next()); }
            constexpr pointer unpop() noexcept { return this->__unpop(); }
            constexpr void swap(single_queue& that) noexcept { this->__qswap(that); }
            constexpr void set_trim_stale(stale_settings<T, A> const& settings) noexcept { __enable_trim_stale = true; this->__set_stale_op_threshold(settings.op_threshold); this->__set_stale_size_threshold(settings.size_threshold); }
        };
    }
    
}
#endif