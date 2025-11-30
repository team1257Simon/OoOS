#ifndef __DYN_PTR
#define __DYN_PTR
#include <ext/type_erasure.hpp>
#include <bits/stdexcept.h>
#include <bits/in_place_t.hpp>
#include <functional>
namespace std
{
	extension namespace ext
	{
		template<typename T> concept polymorphic = is_polymorphic_v<T>;
		namespace __impl
		{
			template<polymorphic T, allocator_object<T> A>
			struct __dynamic_ptr
			{
				template<polymorphic U, allocator_object<U> B> friend struct __dynamic_ptr;
			protected:
				template<derived_from<T> U> using __allocator = conditional_t<is_same_v<T, U>, A, typename A::template rebind<U>>;
				using __base_alloc = A;
				template<derived_from<T> U>
				struct __alloc_helper
				{
					constexpr static __allocator<U> __ualloc{};
					constexpr static T* __allocate() { return __ualloc.allocate(1UZ); }
					constexpr static void __deallocate(void* ptr)
					{
						U* u	= static_cast<U*>(ptr);
						if constexpr(!std::is_trivially_destructible_v<U>) u->~U();
						__ualloc.deallocate(u, 1UZ);
					}
				};
				typedef void (*__dealloc_fn)(void*);
				constexpr static __base_alloc __base{};
				type_erasure __obj_erasure;
				T* __ptr;
				__dealloc_fn __dfn;
				static void __base_dealloc(void* ptr) { __base.deallocate(static_cast<T*>(ptr), 1UZ); }
				~__dynamic_ptr() { if(__ptr && __dfn) (*__dfn)(__ptr); }
				void __clear()
				{
					__ptr			= nullptr;
					__dfn			= nullptr;
					__obj_erasure	= typeid(T);
				}
				template<derived_from<T> U, typename ... Args> requires(constructible_from<U, Args...>)
				__dynamic_ptr(in_place_type_t<U>, Args&& ... args) :
					__obj_erasure	{ typeid(U) },
					__ptr			{ construct_at(__alloc_helper<U>::__allocate(), forward<Args>(args)...) },
					__dfn			{ __alloc_helper<U>::__deallocate }
									{}
				__dynamic_ptr() requires (!is_default_constructible_v<T>) :
					__obj_erasure   { typeid(T) },
					__ptr			{ nullptr },
					__dfn			{ nullptr }
									{}
				template<derived_from<T> U> __dynamic_ptr(U* ptr) :
					__obj_erasure	{ typeid(*ptr) },
					__ptr			{ ptr },
					__dfn			{ __alloc_helper<U>::__deallocate }
									{}
				__dynamic_ptr(__dynamic_ptr&& that) :
					__obj_erasure	{ move(that.__obj_erasure) },
					__ptr			{ that.__ptr },
					__dfn			{ that.__dfn }
									{ that.__clear(); }
				template<derived_from<T> U, allocator_object<U> B>
				__dynamic_ptr(__dynamic_ptr<U, B>&& that) :
					__obj_erasure   { move(that.__obj_erasure) },
					__ptr           { static_cast<T*>(that.__ptr) },
					__dfn           { that.__dfn }
									{ that.__clear(); }
			public:
				void reset()
				{
					if(__ptr && __dfn) (*__dfn)(__ptr);
					__ptr			= nullptr;
					__obj_erasure	= typeid(T);
					if constexpr(is_default_constructible_v<T>) { __ptr = construct_at(__base.allocate(1UZ)); __dfn = __base_dealloc; }
					else __dfn		= nullptr;
				}
				template<derived_from<T> U, typename ... Args> requires(constructible_from<U, Args...>)
				decay_t<U>& emplace(Args&& ... args)
				{
					if(__ptr) reset();
					__obj_erasure	= typeid(U);
					__ptr			= construct_at(__alloc_helper<U>::__allocate(), forward<Args>(args)...);
					__dfn			= __alloc_helper<U>::__deallocate;
				}
			protected:
				void __move_assign(__dynamic_ptr&& that)
				{
					if(__ptr) reset();
					__ptr			= that.__ptr;
					__obj_erasure	= move(that.__obj_erasure);
					__dfn			= that.__dfn;
					that.__clear();
				}
				template<derived_from<T> U, allocator_object<void> B>
				void __move_assign(__dynamic_ptr<U, B>&& that)
				{
					if(__ptr) reset();
					__ptr			= static_cast<T*>(that.__ptr);
					__obj_erasure	= move(that.__obj_erasure);
					__dfn			= that.__dfn;
					that.__clear();
				}
			};
		}
		template<polymorphic T, allocator_object<T> A = allocator<T>>
		class dynamic_ptr : public __impl::__dynamic_ptr<T, A>
		{
			typedef __impl::__dynamic_ptr<T, A> __base;
			template<derived_from<T> U, allocator_object<U> B> using __alt_base = __impl::__dynamic_ptr<U, B>;
		public:
			type_erasure const& erasure() const& { return this->__obj_erasure; }
			T* operator->() & { return this->__ptr; }
			T const* operator->() const& { return this->__ptr; }
			T& operator*() & { return *this->__ptr; }
			T const& operator*() const& { return *this->__ptr; }
			template<typename U> U& cast() & { return dynamic_cast<U&>(*this->__ptr); }
			template<typename U> U const& cast() const& { return dynamic_cast<U const&>(*this->__ptr); }
			template<typename U> U* weak_cast() & { return dynamic_cast<U*>(this->__ptr); }
			template<typename U> U const* weak_cast() const& { return dynamic_cast<U*>(this->__ptr); }
			template<derived_from<T> U, typename ... Args> requires(constructible_from<U, Args...>) dynamic_ptr(in_place_type_t<U>, Args&& ... args) : __base(in_place_type<U>, forward<Args>(args)...) {}
			template<derived_from<T> U> dynamic_ptr(U* ptr) : __base(ptr) {}
			template<derived_from<T> U, allocator_object<U> B> dynamic_ptr(dynamic_ptr<U, B>&& that) : __base(forward<__alt_base<U, B>>(that)) {}
			dynamic_ptr(dynamic_ptr&& that) : __base(forward<__base>(that)) {}
			dynamic_ptr() : __base() {}
			dynamic_ptr& operator=(dynamic_ptr&& that) { this->__move_assign(move(that)); return *this; }
		};
		template<polymorphic T, typename ... Args>
		requires(constructible_from<T, Args...>)
		dynamic_ptr<T> make_dynamic(Args&& ... args) { return dynamic_ptr<T>(in_place_type<T>, forward<Args>(args)...); }
	}
}
#endif