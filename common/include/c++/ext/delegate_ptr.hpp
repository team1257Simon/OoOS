#ifndef __DELEGATE_PTR
#define __DELEGATE_PTR
#include "bits/stl_allocator.h"
#include "bits/typeinfo.h"
#include "ext/type_token.hpp"
#include "vector"
#include "unordered_map"
#include "functional"
/**
 * A delegate pointer is a non-owning pointer (as a weak pointer) to a shared object, the owner of which need not be known to the requestor.
 * When an object is created as a shared object via this interface, the owner "delegates" to the system the responsibility of tracking shared references.
 * Each such object is itself managed via an owning pointer structure, which is tied to the typeid of the object's type.
 */
namespace std
{
    namespace ext
    {
        namespace __impl
        {
            typedef function<void()> __action;
            typedef function<void(size_t)> __idx_fn;
            struct [[gnu::may_alias]] __managed_object_node_base
            {
                size_t __idx;
                size_t __ref_count;
                __idx_fn const& __dealloc_fn;
                __action const& __acquire_fn;
                __action const& __release_fn;
                constexpr void __b_acquire() { if(__acquire_fn) __acquire_fn(); ++__ref_count; }
                constexpr void __release() { if(__release_fn) __release_fn(); --__ref_count; if(!__ref_count) __dealloc_fn(__idx); }
                constexpr __managed_object_node_base(size_t idx, __idx_fn const& dealloc_fn, __action const& acq, __action const& rel) :
                    __idx           { idx },
                    __ref_count     { 0UZ },
                    __dealloc_fn    { dealloc_fn },
                    __acquire_fn    { acq },
                    __release_fn    { rel }
                                    {}
            };
            template<typename T> struct __managed_object_node : __managed_object_node_base
            {
                typedef remove_reference_t<T> value_type;
                typedef add_lvalue_reference_t<value_type> reference;
                typedef add_pointer_t<value_type> pointer;
                typedef add_pointer_t<add_const_t<value_type>> const_pointer;
                typedef add_lvalue_reference_t<add_const_t<value_type>> const_reference;
            private:
                ::__impl::__aligned_buffer<value_type> __data{};
            public:
                pointer __ptr() & { return __data.template __get_ptr<value_type>(); }
                const_pointer __ptr() const& { return __data.template __get_ptr<value_type>(); }
                reference __ref() & { return *__ptr(); }
                const_reference __ref() const& { return *__ptr(); }
                template<typename ... Args> requires constructible_from<T, Args...> __managed_object_node(size_t idx, __idx_fn const& dealloc, __action const& acq, __action const& rel, Args&& ... args) : __managed_object_node_base(idx, dealloc, acq, rel) { construct_at(__data.__get_ptr(), forward<Args>(args)...); }
                __managed_object_node& __acquire() & { __b_acquire(); return *this; }
            };
            template<typename T> 
            struct __alloc_mgr
            {
                static allocator<__managed_object_node<T>> __alloc{};
                static void* __allocate() { return __alloc.allocate(1UZ); }
                static void __deallocate(void* p) { __alloc.deallocate(static_cast<__managed_object_node<T>*>(p), 1UZ); }
            };
            struct __generic_ptr_container : vector<void*> 
            {
                using __node_alloc_ftor = void* (*)();
                using __node_dealloc_ftor = void (*)(void*);
                __action __acquire_fn;
                __action __release_fn;
                __node_alloc_ftor __alloc_node;
                __node_dealloc_ftor __dealloc_node;
                __idx_fn __destroy_node;
                size_t __last_idx{ 0UZ };
                void __erase_at_index(size_t idx);
                size_t __target_idx();
                __generic_ptr_container(__action acq, __action rel, __node_alloc_ftor alloc, __node_dealloc_ftor dealloc);
                __generic_ptr_container(__node_alloc_ftor alloc, __node_dealloc_ftor dealloc);
                inline void* __addr() & { return this; }
            };
            template<typename T> struct __object_type_managed_ptrs : __generic_ptr_container
            {
                template<typename ... Args> requires constructible_from<T, Args...>
                size_t add_new(type_token<T>, Args&& ... args) &
                {
                    size_t result = __target_idx();
                    at(result) = new((*__alloc_node)()) __managed_object_node<T>(result, __destroy_node, __acquire_fn, __release_fn, forward<Args>(args)...);
                    return result;
                }
                size_t add(T&& t) &
                requires move_constructible<T>
                {
                    size_t result = __target_idx();
                    at(result) = new((*__alloc_node)()) __managed_object_node<T>(result, __destroy_node, __acquire_fn, __release_fn, move(t));
                    return result;
                }
                size_t add(T const& t) &
                requires copy_constructible<T>
                {
                    size_t result = __target_idx();
                    at(result) = new((*__alloc_node)()) __managed_object_node<T>(result, __destroy_node, __acquire_fn, __release_fn, t);
                    return result;
                }
                __managed_object_node<T>& operator[](size_t idx) & { if(idx > size()) throw std::out_of_range{ "no such index" }; if(void* ptr = at(idx)) { return *static_cast<__managed_object_node<T>*>(ptr); } throw std::logic_error{ "that index is no longer valid" }; }
            };
            extern unordered_map<const type_info*, __generic_ptr_container> __delegate_ptr_mgr_inst;
            template<typename T> __object_type_managed_ptrs<T>& __get_ptrs()
            {
                if(__delegate_ptr_mgr_inst.contains(addressof(typeid(T)))) { return *static_cast<__object_type_managed_ptrs<T>*>(__delegate_ptr_mgr_inst.find(addressof(typeid(T)))->second.__addr()); }
                unordered_map<const type_info*, __generic_ptr_container>::iterator result = __delegate_ptr_mgr_inst.emplace(piecewise_construct, forward_as_tuple(addressof(typeid(T))), forward_as_tuple(addressof(__alloc_mgr<T>::__allocate), addressof(__alloc_mgr<T>::__deallocate))).first;
                return *static_cast<__object_type_managed_ptrs<T>*>(result->second.__addr());
            }
            template<typename T> void __register_acq_rel_fns(__action&& acq, __action&& rel)
            {
                unordered_map<const type_info*, __generic_ptr_container>::iterator i = __delegate_ptr_mgr_inst.find(addressof(typeid(T)));
                if(i != __delegate_ptr_mgr_inst.end())
                {
                    i->second.__acquire_fn = move(acq);
                    i->second.__release_fn = move(rel);
                    return;
                }
                __delegate_ptr_mgr_inst.emplace(piecewise_construct, forward_as_tuple(addressof(typeid(T))), forward_as_tuple(move(acq), move(rel), addressof(__alloc_mgr<T>::__allocate), addressof(__alloc_mgr<T>::__deallocate))).first;
            }
            template<typename T> __managed_object_node<T>& __get(size_t idx) { return __get_ptrs<T>()[idx]; }
            template<typename T> 
            class __delegate_ptr_impl
            {
                typedef __managed_object_node<T> __nt;
                size_t __idx;
                __managed_object_node<T>& __node;
            protected:
                __delegate_ptr_impl(size_t node_idx) : __idx{ node_idx }, __node{ __get<T>(node_idx).__acquire() } {}
            public:
                ~__delegate_ptr_impl() { __node.__release(); }
                typename __nt::reference operator*() & { return __node.__ref(); }
                typename __nt::const_reference operator*() const& { return __node.__ref(); }
                typename __nt::pointer operator->() & { return __node.__ptr(); }
                typename __nt::const_pointer operator->() const& { return __node.__ptr(); }
                size_t get_id() const { return __idx; }
            };
        }
        template<typename T>
        class delegate_ptr : public __impl::__delegate_ptr_impl<T>
        {
            using __base = __impl::__delegate_ptr_impl<T>;
        public:
            delegate_ptr(size_t id) : __base(id) {}
            delegate_ptr(T&& t) requires move_constructible<T> : __base(__impl::__get_ptrs<T>().add(move(t))) {}
            delegate_ptr(T const& t) requires copy_constructible<T> : __base(__impl::__get_ptrs<T>().add(t)) {}
            template<typename ... Args> requires constructible_from<T, Args...> delegate_ptr(Args&& ... args) : __base(__impl::__get_ptrs<T>().template add_new(type_token<T>(), forward<Args>(args)...)) {}
            static void on_acquire_release(__impl::__action&& acq, __impl::__action&& rel) { __impl::__register_acq_rel_fns<T>(move(acq), move(rel)); }
        };
    }
}
#endif