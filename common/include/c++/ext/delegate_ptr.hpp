#ifndef __DELEGATE_PTR
#define __DELEGATE_PTR
#include <bits/stl_allocator.h>
#include <bits/typeinfo.h>
#include <vector>
#include <unordered_map>
#include <functional>
#include <stdexcept>
/**
 * A delegate pointer is a non-owning pointer (as a shared pointer) to a shared object, the owner of which need not be known to the requestor.
 * Essentially, it's like a shared pointer, but the reference-counting is globally implementation-backed.
 * When an object is created as a shared object via this interface, the owner "delegates" to the system the responsibility of tracking shared references.
 * Each such object is itself managed via an owning pointer structure, which is tied to the typeid of the object's type.
 * In order to acquire a delegate pointer, the caller only needs the index (effectively the index into a vector) of the desired object, or can create a new one.
 * Otherwise, the copy- and move-construction semantics are the same as for a shared pointer.
 * Since kernel-level code is less safe against dangling references, this system allows the use of exceptions to handle a dangling reference to such a pointer.
 * The code that uses them can then handle the exception appropriately (returning EPIPE for a dangling reference to a pipe, for example).
 * The actual implementation is probably a sufficient abuse of type punning to warrant a protection order...but thankfully those only apply to people, not abstract concepts.
 */
namespace std
{
	namespace ext
	{
		// Any action that must be performed on acquiring or releasing a delegate pointer, such as modifying a semaphore
		typedef function<void(void*)> delegate_callback;
		namespace __impl
		{
			typedef function<void(size_t)> __indexed_function;
			struct [[gnu::may_alias]] __managed_object_node_base
			{
				size_t __idx;
				size_t __ref_count;
				__indexed_function const& __dealloc_fn;
				delegate_callback const& __acquire_fn;
				delegate_callback const& __release_fn;
				constexpr __managed_object_node_base(size_t idx, __indexed_function const& dealloc_fn, delegate_callback const& acq, delegate_callback const& rel) :
					__idx			{ idx },
					__ref_count		{ 0UZ },
					__dealloc_fn	{ dealloc_fn },
					__acquire_fn	{ acq },
					__release_fn	{ rel }
									{}
			};
			template<typename T, typename I> concept __indexable				= requires(std::remove_cvref_t<T>& t) { t[std::declval<I>()]; };
			template<typename T, typename I> concept __const_indexable			= requires(std::remove_cvref_t<T> const& t) { t[std::declval<I>()]; };
			template<typename T, typename I>
			class __index_result_helper
			{
				static auto __test(std::remove_cvref_t<T>& t) noexcept -> decltype(t[std::declval<I>()]);
				static auto __ctest(std::remove_cvref_t<T> const& t) noexcept -> decltype(t[std::declval<I>()]);
			public:
				typedef decltype(__index_result_helper<T, I>::__test(*std::declval<T*>())) __result;
				typedef decltype(__index_result_helper<T, I>::__ctest(*std::declval<T const*>())) __const_result;
			};
			template<typename I, __indexable<I> T> struct __index_result { typedef typename __index_result_helper<T, I>::__result type; };
			template<typename I, __indexable<I> T> using __index_result_t = typename __index_result<I, T>::type;
			template<typename I, __const_indexable<I> T> struct __const_index_result { typedef typename __index_result_helper<T, I>::__const_result type; };
			template<typename I, __const_indexable<I> T> using __const_index_result_t = typename __const_index_result<I, T>::type;
			template<typename T>
			struct __managed_object_node : __managed_object_node_base
			{
				typedef remove_reference_t<T> value_type;
				typedef add_lvalue_reference_t<value_type> reference;
				typedef add_pointer_t<value_type> pointer;
				typedef add_pointer_t<add_const_t<value_type>> const_pointer;
				typedef add_lvalue_reference_t<add_const_t<value_type>> const_reference;
				typedef add_rvalue_reference_t<value_type> rv_reference;
				typedef add_rvalue_reference_t<add_const_t<value_type>> const_rv_reference;
			private:
				::__impl::__aligned_buffer<value_type> __data{};
			public:
				pointer __ptr() { return __data.__get_ptr(); }
				const_pointer __ptr() const { return __data.__get_ptr(); }
				reference __ref() { return *__ptr(); }
				const_reference __ref() const { return *__ptr(); }
				template<typename ... Args> requires(constructible_from<T, Args...>) __managed_object_node(size_t idx, __indexed_function const& dealloc, delegate_callback const& acq, delegate_callback const& rel, Args&& ... args) : __managed_object_node_base(idx, dealloc, acq, rel) { construct_at(__data.__get_ptr(), forward<Args>(args)...); }
				__managed_object_node& __acquire() { if(__acquire_fn) __acquire_fn(__data.__get_ptr()); ++__ref_count; return *this; }
				void __release() { if(__release_fn) __release_fn(__data.__get_ptr()); --__ref_count; if(!__ref_count && __dealloc_fn) __dealloc_fn(__idx); }
			};
			template<typename T> 
			struct __alloc_mgr
			{
				constexpr static allocator<__managed_object_node<T>> __alloc{};
				static void* __allocate() { return __alloc.allocate(1UZ); }
				template<typename ... Args> requires(constructible_from<T, Args...>) static T* __create(Args&& ... args) { return new(__allocate()) T(forward<Args>(args)...); }
				static void* __copy_construct(const void* ptr) requires(copy_constructible<decay_t<T>>) { return new(__allocate()) T(*static_cast<const T*>(ptr)); }
				static void __deallocate(void* p)
				{
					if(__unlikely(!p)) return;
					__managed_object_node<T>* node = static_cast<__managed_object_node<T>*>(p);
					if constexpr(!std::is_trivially_destructible_v<T>) { node->__ref().~T(); }
					__alloc.deallocate(node, 1UZ);
				}
			};
			struct __generic_ptr_container : vector<void*> 
			{
				using __node_alloc_ftor		= void* (*)();
				using __node_dealloc_ftor	= void (*)(void*);
				delegate_callback __acquire_fn;
				delegate_callback __release_fn;
				__node_alloc_ftor __alloc_node;
				__node_dealloc_ftor __dealloc_node;
				__indexed_function __destroy_node;
				size_t __last_idx{};
				void __erase_at_index(size_t idx);
				size_t __target_idx();
				__generic_ptr_container(delegate_callback acq, delegate_callback rel, __node_alloc_ftor alloc, __node_dealloc_ftor dealloc);
				__generic_ptr_container(__node_alloc_ftor alloc, __node_dealloc_ftor dealloc);
				inline void* __addr() { return this; }
			};
			template<typename T>
			struct __object_type_managed_ptrs : __generic_ptr_container
			{
				template<typename ... Args> requires(constructible_from<T, Args...>)
				size_t add_new(Args&& ... args)
				{
					size_t result	= __target_idx();
					at(result)		= new((*__alloc_node)()) __managed_object_node<T>(result, __destroy_node, __acquire_fn, __release_fn, forward<Args>(args)...);
					return result;
				}
				size_t add(T&& t)
				requires(move_constructible<T>)
				{
					size_t result	= __target_idx();
					at(result)		= new((*__alloc_node)()) __managed_object_node<T>(result, __destroy_node, __acquire_fn, __release_fn, move(t));
					return result;
				}
				size_t add(T const& t)
				requires(copy_constructible<T>)
				{
					size_t result	= __target_idx();
					at(result)		= new((*__alloc_node)()) __managed_object_node<T>(result, __destroy_node, __acquire_fn, __release_fn, t);
					return result;
				}
				__managed_object_node<T>& operator[](size_t idx) &
				{
					if(idx > size()) throw std::out_of_range("[UTIL/DPTR] no such index");
					if(void* ptr = at(idx)) return *static_cast<__managed_object_node<T>*>(ptr);
					throw std::logic_error("[UTIL/DPTR] that index is no longer valid");
				}
			};
			typedef std::unordered_map<std::type_info const*, __generic_ptr_container>::iterator __container_iterator;
			extern std::unordered_map<std::type_info const*, __generic_ptr_container> __delegate_ptr_mgr_inst;
			template<typename T>
			__object_type_managed_ptrs<T>& __get_ptrs()
			{
				using __alloc_ftor				= typename __generic_ptr_container::__node_alloc_ftor;
				using __dealloc_ftor			= typename __generic_ptr_container::__node_dealloc_ftor;
				std::type_info const* ti_ptr	= std::addressof(typeid(T));
				if(__delegate_ptr_mgr_inst.contains(ti_ptr)) 
					return *static_cast<__object_type_managed_ptrs<T>*>(__delegate_ptr_mgr_inst.find(ti_ptr)->second.__addr());
				__alloc_ftor a					= std::addressof(__alloc_mgr<T>::__allocate);
				__dealloc_ftor d				= std::addressof(__alloc_mgr<T>::__deallocate);
				__container_iterator result		= __delegate_ptr_mgr_inst.emplace(piecewise_construct, std::forward_as_tuple(ti_ptr), std::forward_as_tuple(a, d)).first;
				return *static_cast<__object_type_managed_ptrs<T>*>(result->second.__addr());
			}
			template<typename T>
			void __register_acq_rel_fns(delegate_callback&& acq, delegate_callback&& rel)
			{
				using __alloc_ftor				= typename __generic_ptr_container::__node_alloc_ftor;
				using __dealloc_ftor			= typename __generic_ptr_container::__node_dealloc_ftor;
				std::type_info const* ti_ptr	= std::addressof(typeid(T));
				if(__container_iterator i		= __delegate_ptr_mgr_inst.find(ti_ptr); i != __delegate_ptr_mgr_inst.end())
				{
					i->second.__acquire_fn		= std::move(acq);
					i->second.__release_fn		= std::move(rel);
					return;
				}
				__alloc_ftor a					= std::addressof(__alloc_mgr<T>::__allocate);
				__dealloc_ftor d				= std::addressof(__alloc_mgr<T>::__deallocate);
				__delegate_ptr_mgr_inst.emplace(piecewise_construct, std::forward_as_tuple(ti_ptr), std::forward_as_tuple(std::move(acq), std::move(rel), a, d));
			}
			template<typename T> __managed_object_node<T>& __get(size_t idx) { return __get_ptrs<T>()[idx]; }
			template<typename T> 
			class __delegate_ptr_impl
			{
				typedef __managed_object_node<T> __nt;
				size_t __idx;
				__managed_object_node<T>* __node;
			protected:
				__delegate_ptr_impl(size_t node_idx) : __idx(node_idx), __node(std::addressof(__get<T>(node_idx).__acquire())) {}
				__delegate_ptr_impl(__delegate_ptr_impl&& that) : __idx(that.__idx), __node(that.__node) { that.__node = nullptr; }
				void __destroy() { if(__node) __node->__release(); }
				void __assign(__delegate_ptr_impl const& that)
				{
					this->__destroy();
					this->__idx		= that.__idx;
					this->__node	= that.__node;
					this->__node->__acquire();
				}
				void __assign(__delegate_ptr_impl&& that)
				{
					this->__destroy();
					this->__idx		= that.__idx;
					this->__node	= that.__node;
					that.__node		= nullptr; 
				}
			public:
				~__delegate_ptr_impl() { __destroy(); }
				size_t get_id() const { return __idx; }
				typename __nt::reference operator*() & { return __node->__ref(); }
				typename __nt::const_reference operator*() const& { return __node->__ref(); }
				typename __nt::pointer operator->() & { return __node->__ptr(); }
				typename __nt::const_pointer operator->() const& { return __node->__ptr(); }
				template<typename I> requires(__indexable<T, I>) __index_result_t<I, T> operator[](I&& i) & { return __node->__ref()[std::forward<I>(i)]; }
				template<typename I> requires(__const_indexable<T, I>) __const_index_result_t<I, T> operator[](I&& i) const& { return __node->__ref()[std::forward<I>(i)]; }
			};
		}
		template<typename T>
		class delegate_ptr : public __impl::__delegate_ptr_impl<T>
		{
			using __base = __impl::__delegate_ptr_impl<T>;
		public:
			delegate_ptr(size_t id) : __base(id) {}
			delegate_ptr(T&& t) requires(move_constructible<T>) : __base(__impl::__get_ptrs<T>().add(std::move(t))) {}
			delegate_ptr(T const& t) requires(copy_constructible<T>) : __base(__impl::__get_ptrs<T>().add(t)) {}
			delegate_ptr(delegate_ptr const& that) : __base(that.get_id()) {}
			delegate_ptr(delegate_ptr&& that) : __base(std::move(that)) {}
			template<typename ... Args> requires(constructible_from<T, Args...>) delegate_ptr(Args&& ... args) : __base(__impl::__get_ptrs<T>().add_new(std::forward<Args>(args)...)) {}
			delegate_ptr& operator=(delegate_ptr const& that) { __base::__assign(that); return *this; }
			delegate_ptr& operator=(delegate_ptr&& that) { __base::__assign(std::move(that)); return *this; }
			void release() { __base::__destroy(); this->__node = nullptr; }
			// Registers any action that must be performed on a per-reference basis with regard to a given object when acquiring or releasing a delegate pointer.
			static void on_acquire_release(delegate_callback&& acq, delegate_callback&& rel) { __impl::__register_acq_rel_fns<T>(std::move(acq), std::move(rel)); }
			friend constexpr strong_ordering operator<=>(delegate_ptr const& __this, delegate_ptr const& __that) noexcept { return __this.__idx <=> __that.__idx; }
			friend constexpr bool operator==(delegate_ptr const& __this, delegate_ptr const& __that) noexcept { return __this.__idx == __that.__idx; }
		};
	}
}
#endif