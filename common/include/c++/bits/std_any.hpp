#ifndef __STD_ANY
#define __STD_ANY
#include <ext/type_erasure.hpp>
#include <bits/aligned_buffer.hpp>
#include <ext/delegate_ptr.hpp>	// ext::__impl:__alloc_mgr
#include <bits/in_place_t.hpp>
namespace std
{
	template<typename T> concept __decay_copy_constructible = copy_constructible<decay_t<T>>;
	template<typename T> concept __non_void = !is_void_v<T>;
	struct any;
	template<typename T> T* any_cast(any* a) noexcept;
	template<typename T> T const* any_cast(any const* a) noexcept;
	struct any
	{
	protected:
		template<typename T> using __alloc = ext::__impl::__alloc_mgr<T>;
		void* __ptr;
		void* (*__copy_fn)(const void*);
		void (*__dealloc_fn)(void*);
		ext::type_erasure __object_type;
		constexpr void __destroy_if_present() { if(__ptr && __dealloc_fn) (*__dealloc_fn)(__ptr); }
		template<__non_void T> T const* __cast_to_type() const
		{
			if(!__ptr) return nullptr;
			ext::type_erasure e = ext::get_erasure<T>();
			return static_cast<T const*>(e.cast_from(__ptr, __object_type));
		}
		template<__non_void T> T* __cast_to_type()
		{
			if(!__ptr) return nullptr;
			ext::type_erasure e = ext::get_erasure<T>();
			return static_cast<T*>(e.cast_from(__ptr, __object_type));
		}
	public:
		constexpr any() noexcept = default;
		constexpr ~any() { __destroy_if_present(); }
		constexpr bool has_value() const { return __ptr != nullptr; }
		constexpr void swap(any& that)
		{
			void* tptr					= this->__ptr;
			void (*tdealloc)(void*)		= this->__dealloc_fn;
			void* (*tcopy)(const void*)	= this->__copy_fn;
			this->__ptr					= that.__ptr;
			this->__dealloc_fn			= that.__dealloc_fn;
			this->__copy_fn				= that.__copy_fn;
			this->__object_type.swap(that.__object_type);
			that.__copy_fn				= tcopy;
			that.__dealloc_fn			= tdealloc;
			that.__ptr					= tptr;
		}
		constexpr void reset()
		{
			__ptr			= nullptr;
			__copy_fn		= nullptr;
			__dealloc_fn	= nullptr;
			__object_type	= ext::get_erasure<void>();
		}
		template<__decay_copy_constructible T> any(T&& t) :
			__ptr			{ __alloc<T>::__create(move(t)) },
			__copy_fn		{ __alloc<T>::__copy_construct },
			__dealloc_fn	{ __alloc<T>::__deallocate },
			__object_type	{ typeid(T) }
							{}
		template<__decay_copy_constructible T, typename ... Args> requires constructible_from<T, Args...> any(in_place_type_t<T>, Args&& ... args) :
			__ptr			{ __alloc<T>::__create(forward<Args>(args)...) },
			__copy_fn		{ __alloc<T>::__copy_construct },
			__dealloc_fn	{ __alloc<T>::__deallocate },
			__object_type	{ typeid(T) }
							{}
		constexpr any(any const& that) :
			__ptr			{ that.__copy_fn && that.__ptr ? (*that.__copy_fn)(that.__ptr) : nullptr },
			__copy_fn		{ that.__copy_fn },
			__dealloc_fn	{ that.__dealloc_fn },
			__object_type	{ that.__object_type }
							{}
		constexpr any(any&& that) :
			__ptr			{ that.__ptr },
			__copy_fn		{ that.__copy_fn },
			__dealloc_fn	{ that.__dealloc_fn },
			__object_type	{ move(that.__object_type) }
							{ that.reset(); }
		constexpr any& operator=(any const& that)
		{
			__destroy_if_present();
			this->__ptr			= that.__copy_fn && that.__ptr ? (*that.__copy_fn)(that.__ptr) : nullptr;
			this->__copy_fn		= that.__copy_fn;
			this->__dealloc_fn	= that.__dealloc_fn;
			this->__object_type	= that.__object_type;
			return *this;
		}
		constexpr any& operator=(any&& that)
		{
			__destroy_if_present();
			this->__ptr			= that.__ptr;
			this->__copy_fn		= that.__copy_fn;
			this->__dealloc_fn	= that.__dealloc_fn;
			this->__object_type	= that.__object_type;
			that.reset();
			return *this;
		}
		template<__decay_copy_constructible T> any& operator=(T&& t)
		{
			__destroy_if_present();
			__ptr			= __alloc<T>::__create(move(t));
			__copy_fn		= __alloc<T>::__copy_construct;
			__dealloc_fn	= __alloc<T>::__deallocate;
			__object_type	= ext::get_erasure<T>();
		}
		template<__decay_copy_constructible T, typename ... Args> requires constructible_from<T, Args...> decay_t<T>& emplace(Args&& ... args)
		{
			__destroy_if_present();
			__ptr			= __alloc<T>::__create(forward<Args>(args)...);
			__copy_fn		= __alloc<T>::__copy_construct;
			__dealloc_fn	= __alloc<T>::__deallocate;
			__object_type	= ext::get_erasure<T>();
			return *static_cast<T*>(__ptr);
		}
		template<typename T> friend T* std::any_cast(any* a) noexcept;
		template<typename T> friend T const* std::any_cast(any const* a) noexcept;
		constexpr type_info const& type() const noexcept { return __object_type.get_type_info(); }
		extension constexpr ext::type_erasure const& erasure() const noexcept { return __object_type; }
	};
	template<typename T> T* any_cast(any* a) noexcept { return a->__cast_to_type<T>(); }
	template<typename T> T const* any_cast(any const* a) noexcept  { return a->__cast_to_type<T>(); }
}
#endif