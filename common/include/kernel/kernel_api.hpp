#ifndef __KAPI
#define __KAPI
#include <stdarg.h>
#include <fs/dev_stream.hpp>
#include <arch/pci.hpp>
#include <typeinfo>
#include <net/protocol/arp.hpp>
#include <bits/stl_queue.hpp>
struct kframe_tag;
struct kframe_exports;
#ifndef __HAVE_ALIGNED_REALLOCATE
namespace std::__detail { [[nodiscard]] [[gnu::externally_visible]] void* __aligned_reallocate(void* ptr, size_t n, size_t align); }
#define __HAVE_ALIGNED_REALLOCATE
#endif
namespace ooos
{
	class abstract_module_base;
	class isr_actor;
	struct module_eh_ctx;
	template<typename T> concept no_args_invoke					= requires(T t) { t(); };
	template<typename T, typename R> concept no_args_supplier	= no_args_invoke<T> && requires(T t) { { t() } -> std::convertible_to<R>; };
	namespace __internal
	{
		class __anything;
		template<typename> struct __use_value_type {};
		template<typename> struct __use_difference_type {};
		template<typename> struct __basic_char_type_helper : std::false_type {};
		template<> struct __basic_char_type_helper<char> : std::true_type {};
		template<> struct __basic_char_type_helper<unsigned char> : std::true_type {};
		template<> struct __basic_char_type_helper<signed char> : std::true_type {};
		template<> struct __basic_char_type_helper<wchar_t> : std::true_type {};
		template<> struct __basic_char_type_helper<char8_t> : std::true_type {};
		template<> struct __basic_char_type_helper<char16_t> : std::true_type {};
		template<> struct __basic_char_type_helper<char32_t> : std::true_type {};
		template<typename T> using __ref 									= T&;
		template<typename T> concept __can_reference 						= requires { typename __ref<T>; };
		template<typename T> concept __object 								= std::is_object_v<T>;
		template<typename T> concept __can_dereference 						= requires(std::add_lvalue_reference_t<T> __t) { { *__t } -> __can_reference; };
		template<typename T> concept __can_reference_and_dereference 		= __can_reference<T> && __can_dereference<T>;
		template<typename PT, typename VT> concept __points_to 				= std::is_convertible_v<decltype(*std::declval<PT>()), std::add_lvalue_reference_t<VT>>;
		template<typename PT, typename VT> concept __points_to_maybe_const 	= __points_to<PT, std::add_const_t<VT>>;
		template<typename PT, typename VT> concept __points_to_const 		= __points_to_maybe_const<PT, VT> && !__points_to<PT, VT>;
		template<typename IT> concept __has_defined_value_type 				= requires { typename IT::value_type; };
		template<typename IT> concept __has_implicit_value_type 			= !__has_defined_value_type<IT> && __can_reference_and_dereference<IT>;
		template<typename IT> concept __has_defined_difference_type 		= requires { typename IT::difference_type; };
		template<typename IT> concept __has_implicit_difference_type 		= !__has_defined_difference_type<IT> && requires(std::add_lvalue_reference_t<std::add_const_t<IT>> a, std::add_lvalue_reference_t<std::add_const_t<IT>> b) { { b - a } -> std::integral; };
		template<typename T> concept __has_difference_type 					= requires { typename __use_difference_type<T>::type; };
		template<typename T> concept __has_value_type 						= requires { typename __use_value_type<T>::type; };
		template<typename T> concept __convertible_to_weak 					= std::is_convertible_v<T, std::weak_ordering>;
		template<typename T> concept __weakly_ordered 						= requires(T&& t1, T&& t2) { { static_cast<T&&>(t1) <=> static_cast<T&&>(t2) } -> __convertible_to_weak; };
		template<typename T> concept __non_register 						= sizeof(T) > sizeof(long long);
		template<typename T> concept __can_be_parameter_type 				= std::is_standard_layout_v<T> && (std::is_copy_constructible_v<T> || std::is_move_constructible_v<T> || std::is_default_constructible_v<T>) && (std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>);
		template<typename T> concept __simple_swappable 					= std::is_move_assignable_v<T> && std::is_move_constructible_v<T>;
		template<typename T, typename ... Args> concept __callable			= std::is_invocable_v<T, Args...>;
		template<typename FT, typename T> concept __transfer_fn				= std::assignable_from<std::remove_cvref_t<T>&, decltype(std::declval<FT>()(std::declval<T>()))>;
		template<typename T, typename U> concept __explicitly_convertible	= requires(T t) { static_cast<U>(t); };
		template<typename T> concept __basic_char_type						= __basic_char_type_helper<T>::value;
		template<typename T> concept __can_be_parameter						= __can_be_parameter_type<T> || __basic_char_type<std::remove_cvref_t<decltype(*std::declval<T>())>> || __can_be_parameter_type<std::remove_cvref_t<decltype(std::declval<T>()[0])>>;
		template<__has_defined_difference_type IT> struct __use_difference_type<IT> { typedef typename IT::difference_type type; };
		template<__has_implicit_difference_type IT> struct __use_difference_type<IT> { typedef decltype(std::declval<IT>() - std::declval<IT>()) type; };
		template<typename T> struct __use_difference_type<T const> : __use_difference_type<T> {};
		template<__has_defined_value_type IT> struct __use_value_type<IT> { typedef typename IT::value_type type; };
		template<__has_implicit_value_type IT> struct __use_value_type<IT> { typedef std::remove_reference_t<decltype(*std::declval<IT>())> type; };
		template<typename T> struct __use_value_type<T const> : __use_value_type<T> {};
		template<typename T> struct __single_buffer_value { typedef std::remove_cvref_t<T> type; typedef std::remove_reference_t<T> const_type; };
		template<__non_register T> struct __single_buffer_value<T> { typedef std::add_lvalue_reference_t<T> type; typedef std::add_lvalue_reference_t<std::add_const_t<T>> const_type; };
		template<typename T> struct __pack_type_select { typedef T type; };
		template<typename T> struct __pack_type_select<T[]> { typedef T* type; };
		template<typename T, size_t N> struct __pack_type_select<T[N]> { typedef T* const type; };
		template<size_t N, typename ... Ts> struct __nth_pack_param;
		template<template<typename ...> class C, typename H, typename ... Ts> struct __nth_pack_param<0, C<H, Ts...>> { typedef typename __pack_type_select<H>::type type; };
		template<size_t N, template<typename ...> class C, typename H, typename ... Ts> struct __nth_pack_param<N, C<H, Ts...>> : __nth_pack_param<N - 1, C<Ts...>> {};
		template<no_args_invoke FT, typename RT = decltype((std::declval<FT&&>())())> constexpr RT __invoke_f(FT&& f) { if constexpr(!std::is_void_v<RT>) { return (std::forward<FT>(f))(); } else { (std::forward<FT>(f))(); } }
		template<typename VT, __callable<VT> FT> constexpr void __invoke_v(FT&& f, VT&& v) { std::forward<FT>(f)(std::forward<VT>(v)); }
		template<__simple_swappable T> constexpr void __swap(T& a, T& b) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) { T tmp = std::move(a); a = std::move(b); b = std::move(tmp); }
	}
	template<typename T> concept wrappable_actor 								= no_args_invoke<T> && !std::is_same_v<isr_actor, T>;
	template<typename T> concept boolable 										= requires(T t) { t ? true : false; };
	template<typename T> concept io_buffer_ok 									= std::is_default_constructible_v<T> && !std::is_volatile_v<T> && std::is_copy_assignable_v<T>;
	template<typename IT, typename VT> concept value_iterator 					= __internal::__can_reference_and_dereference<IT> && __internal::__points_to<IT, VT>;
	template<typename IT, typename VT> concept maybe_const_value_iterator 		= __internal::__can_reference_and_dereference<IT> && __internal::__points_to_maybe_const<IT, std::remove_const_t<VT>>;
	template<typename IT, typename VT> concept const_value_iterator 			= __internal::__can_reference_and_dereference<IT> && __internal::__points_to_const<IT, std::remove_const_t<VT>>;
	template<__internal::__has_difference_type IT> using iterator_difference_t 	= typename __internal::__use_difference_type<IT>::type;
	template<__internal::__has_value_type IT> using dereference_value_t 		= typename __internal::__use_value_type<IT>::type;
	template<typename T> using in_value 										= typename __internal::__single_buffer_value<T>::const_type;
	template<typename T> using out_value 										= typename __internal::__single_buffer_value<T>::type;
	template<typename IT> concept incrementable_iterator 						= __internal::__has_value_type<IT> && __internal::__has_difference_type<IT>;
	template<incrementable_iterator IT>
	struct simple_iterator
	{
	protected:
		IT current;
	public:
		typedef dereference_value_t<IT> value_type;
		typedef iterator_difference_t<IT> difference_type;
		typedef std::add_lvalue_reference_t<value_type> reference;
		typedef std::add_pointer_t<value_type> pointer;
	protected:
		typedef decltype(std::declval<difference_type>() <=> std::declval<difference_type>()) order_type;
	public:
		constexpr IT const& base() const noexcept { return current; }
		constexpr simple_iterator() noexcept : current() {}
		constexpr explicit simple_iterator(IT const& i) noexcept : current(i) {}
		template<typename JT> requires(maybe_const_value_iterator<IT, dereference_value_t<JT>>) constexpr simple_iterator(simple_iterator<JT> const& that) noexcept : current{ that.base() } {}
		constexpr reference operator*() const noexcept { return *current; }
		constexpr pointer operator->() const noexcept { return current; }
		constexpr reference operator[](difference_type n) const noexcept { return *(current + n); }
		constexpr simple_iterator& operator++() noexcept { ++current; return *this; }
		constexpr simple_iterator operator++(int) noexcept { return simple_iterator(current++); }
		constexpr simple_iterator& operator--() noexcept { --current; return *this; }
		constexpr simple_iterator operator--(int) noexcept { return simple_iterator(current--); }
		constexpr simple_iterator& operator+=(difference_type n) noexcept { current += n; return *this; }
		constexpr simple_iterator& operator-=(difference_type n) noexcept { current -= n; return *this; }
		constexpr simple_iterator operator+(difference_type n) const noexcept { return simple_iterator(current + n); }
		constexpr simple_iterator operator-(difference_type n) const noexcept { return simple_iterator(current - n); }
		friend constexpr order_type operator<=>(simple_iterator const& __this, simple_iterator const& that) noexcept { return (__this.current - that.current) <=> static_cast<difference_type>(0); }
		friend constexpr bool operator==(simple_iterator const& __this, simple_iterator const& that) noexcept { return __this.current == that.current; }
	};
	struct kmod_mm
	{
		virtual void* mem_allocate(size_t size, size_t align) 								= 0;
		virtual void mem_release(void* block, size_t align) 								= 0;
		virtual void* mem_resize(void* old, size_t old_size, size_t target, size_t align) 	= 0;
		virtual kframe_tag* get_frame() 													= 0;
		virtual ~kmod_mm() 																	= default;
	};
	struct register_data
	{
		register_t data;
		constexpr register_data() noexcept = default;
		template<typename T> requires(sizeof(T) <= sizeof(register_t)) constexpr register_data(T t) noexcept : data(__builtin_bit_cast(register_t, t)) {}
	};
	union local_functor_types
	{
		void* object;
		const void* const_object;
		void (*function_ptr)();
		void (abstract_module_base::*member_function_ptr)();
	};
	union [[gnu::may_alias]] functor_store
	{
		local_functor_types ignored;
		char actual[sizeof(local_functor_types)];
		constexpr void* access() noexcept { return std::addressof(actual[0]); }
		constexpr const void* access() const noexcept { return std::addressof(actual[0]); }
		template<typename T> constexpr T& cast() noexcept { return *static_cast<T*>(access()); }
		template<typename T> constexpr T const& cast() const noexcept { return *static_cast<T const*>(access()); }
		template<typename T> constexpr void set_ptr(T* ptr) noexcept { *static_cast<T**>(access()) = ptr; }
	};
	enum mgr_op
	{
		get_pointer,
		clone,
		destroy,
		get_type_info,
		move
	};
	struct isr_actor_base
	{
		template<typename R, typename C, typename ... Args> using member_fn = R (C::*)(Args...);
		template<typename GT> constexpr static bool not_empty(GT* gt) noexcept { return gt != nullptr; }
		template<typename R, typename C, typename ... Args> constexpr static bool not_empty(member_fn<R, C, Args...> mf) noexcept { return mf != nullptr; }
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Waddress"
		template<boolable T> constexpr static bool not_empty(T t) noexcept { return t ? true : false; }
		#pragma GCC diagnostic pop
		template<typename T> constexpr static bool not_empty(T t) noexcept { return true; }
		template<no_args_invoke FT>
		class actor_manager
		{
			constexpr static size_t __max_size  	= sizeof(local_functor_types);
			constexpr static size_t __max_align 	= alignof(local_functor_types);
			template<typename GT> static inline void __create_wrapper(functor_store& dst, GT&& ftor, kmod_mm*, std::true_type) noexcept(std::is_nothrow_constructible<FT, GT>::value) { new(dst.access()) FT(std::forward<GT>(ftor)); }
			template<typename GT> static inline void __create_wrapper(functor_store& dst, GT&& ftor, kmod_mm* alloc, std::false_type) { dst.set_ptr(new(alloc->mem_allocate(sizeof(FT), alignof(FT))) FT(std::forward<GT>(ftor))); }
			static inline void __delete_wrapper(functor_store& target, kmod_mm*, std::true_type) { target.template cast<FT>().~FT(); }
			static inline void __delete_wrapper(functor_store& target, kmod_mm* alloc, std::false_type) { FT* ptr = target.template cast<FT*>(); ptr->~FT(); alloc->mem_release(ptr, alignof(FT)); }
			using __is_locally_storable = std::bool_constant<std::is_trivially_copyable_v<FT> && (sizeof(FT) <= __max_size && alignof(FT) <= __max_align && __max_align % alignof(FT) == 0)>;
			constexpr static bool __is_local_store	= __is_locally_storable::value;
		public:
			constexpr static FT* get_ptr(functor_store const& src) { if constexpr(__is_local_store) { return std::addressof(const_cast<FT&>(src.template cast<FT>())); } else { return src.template cast<FT*>(); } }
			template<typename GT> static inline void init_actor(functor_store& dst, GT&& src, kmod_mm* alloc) noexcept(std::is_nothrow_constructible<FT, GT>::value && __is_local_store) { __create_wrapper(dst, std::forward<GT>(src), alloc, __is_locally_storable()); }
			static inline void destroy_actor(functor_store& dst, kmod_mm* alloc) { __delete_wrapper(dst, alloc, __is_locally_storable()); }
			static void invoke_fn(functor_store& fn) { __internal::__invoke_f(std::forward<FT>(*get_ptr(fn))); }
			static void action(functor_store& dst, functor_store const& src, kmod_mm* alloc, mgr_op op)
			{
				switch(op)
				{
				case get_pointer:
					dst.template cast<FT*>() = get_ptr(src);
					break;
				case clone:
				case move:
					init_actor(dst, *const_cast<FT const*>(get_ptr(src)), alloc);
					break;
				case destroy:
					destroy_actor(dst, alloc);
					break;
				case get_type_info:
					dst.set_ptr(std::addressof(typeid(FT)));
					break;
				}
			}
		};
		using manager_type = void(*)(functor_store&, functor_store const&, kmod_mm*, mgr_op);
		using invoker_type = void(*)(functor_store&);
		functor_store __my_actor;
		manager_type __my_manager;
		invoker_type __my_invoke;
		kmod_mm* __my_alloc;
		constexpr bool __empty() const noexcept { return !__my_manager || !__my_alloc; }
		constexpr ~isr_actor_base() noexcept { if(__my_manager && __my_alloc) { __my_manager(__my_actor, __my_actor, __my_alloc, destroy); } }
	};
	struct isr_actor : private isr_actor_base
	{
		constexpr isr_actor() noexcept = default;
		constexpr ~isr_actor() noexcept = default;
		constexpr operator bool() const noexcept { return !__empty(); }
		template<wrappable_actor FT> constexpr isr_actor(FT&& ft, kmod_mm* alloc)
		{
			typedef actor_manager<std::decay_t<FT>> __mgr;
			if(not_empty(ft))
			{
				__mgr::init_actor(__my_actor, std::forward<FT>(ft), alloc);
				__my_manager    = std::addressof(__mgr::action);
				__my_invoke     = std::addressof(__mgr::invoke_fn);
				__my_alloc      = alloc;
			}
		}
		constexpr isr_actor(isr_actor const& that) : isr_actor_base()
		{
			if(static_cast<bool>(that))
			{
				that.__my_manager(this->__my_actor, that.__my_actor, that.__my_alloc, clone);
				this->__my_manager  = that.__my_manager;
				this->__my_invoke   = that.__my_invoke;
				this->__my_alloc    = that.__my_alloc;
			}
		}
		constexpr isr_actor(isr_actor&& that) : isr_actor_base()
		{
			if(static_cast<bool>(that))
			{
				this->__my_actor    = that.__my_actor;
				this->__my_manager  = that.__my_manager;
				this->__my_invoke   = that.__my_invoke;
				this->__my_alloc    = that.__my_alloc;
				that.__my_manager   = nullptr;
				that.__my_invoke    = nullptr;
				that.__my_alloc     = nullptr;
			}
		}
		constexpr void swap(isr_actor& that) noexcept
		{
			using __internal::__swap;
			__swap(this->__my_actor, that.__my_actor);
			__swap(this->__my_invoke, that.__my_invoke);
			__swap(this->__my_manager, that.__my_manager);
			__swap(this->__my_alloc, that.__my_alloc);
		}
		isr_actor& operator=(isr_actor const& that) { isr_actor(that).swap(*this); return *this; }
		isr_actor& operator=(isr_actor&& that) { isr_actor(std::move(that)).swap(*this); return *this; }
		constexpr void operator()() { if(!__empty()) __my_invoke(__my_actor); }
		constexpr std::type_info const& target_type() const noexcept { if(__my_manager) { functor_store tmp_store; __my_manager(tmp_store, __my_actor, __my_alloc, get_type_info); if(std::type_info const* result = tmp_store.cast<std::type_info const*>()) return *result; } return typeid(nullptr); }
	};
	struct netdev_api_helper;
	class abstract_netdev_module;
	struct kernel_api
	{
		friend class abstract_module_base;
		virtual void* allocate_dma(size_t size, bool prefetchable) 																		= 0;
		virtual void release_dma(void* ptr, size_t size) 																				= 0;
		virtual void* map_dma(uintptr_t addr, size_t sz, bool prefetchable)																= 0;
		virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) 												= 0;
		virtual void* acpi_get_table(const char* label) 																				= 0;
		virtual uintptr_t vtranslate(void* addr) noexcept																				= 0;
		virtual void on_irq(uint8_t irq, isr_actor&& handler, abstract_module_base* owner) 												= 0;
		virtual void remove_actors(abstract_module_base* owner) 																		= 0;
		virtual kmod_mm* create_mm() 																									= 0;
		virtual void destroy_mm(kmod_mm* mm) 																							= 0;
		virtual void log(std::type_info const& from, const char* message) 																= 0;
		virtual uint32_t register_device(dev_stream<char>* stream, device_type type) 													= 0;
		virtual bool deregister_device(dev_stream<char>* stream) 																		= 0;
		virtual void init_memory_fns(kframe_exports* ptrs) 																				= 0;
		[[noreturn]] virtual void ctx_raise(module_eh_ctx& ctx, const char* msg, int status) 											= 0;
		[[nodiscard]] virtual netdev_api_helper* create_net_helper(abstract_netdev_module& mod)											= 0;
		virtual size_t vformat(kmod_mm* mm, const char* fmt, const char*& out, va_list args) 											= 0;
		virtual size_t vlogf(std::type_info const& from, const char* fmt, va_list args) 												= 0;
	protected:
		virtual void register_type_info(std::type_info const* ti) 																		= 0;
		virtual void relocate_type_info(abstract_module_base* mod, std::type_info const* local_si, std::type_info const* local_vmi) 	= 0;
	};
	kernel_api* get_api_instance();
	void init_api();
	pid_t active_pid() noexcept;
	struct generic_config_parameter
	{
		const char* parameter_name;
		size_t value_size;
		char value_data[];
		constexpr void* get_value() noexcept { return value_data; }
		constexpr const void* get_value() const noexcept { return value_data; }
		constexpr generic_config_parameter* next_in_array() noexcept { void* pos = this; void* next = static_cast<char*>(pos) + value_size + sizeof(generic_config_parameter); return static_cast<generic_config_parameter*>(next); }
		constexpr generic_config_parameter const* next_in_array() const noexcept { const void* pos = this; const void* next = static_cast<const char*>(pos) + value_size + sizeof(generic_config_parameter); return static_cast<generic_config_parameter const*>(next); }
	};
	template<typename T> struct parameter_type_t { constexpr explicit parameter_type_t() noexcept = default; };
	template<__internal::__can_be_parameter_type T> constexpr inline parameter_type_t<T> parameter_type{};
	template<__internal::__can_be_parameter_type T> constexpr inline parameter_type_t<T[]> parameter_array{};
	template<__internal::__basic_char_type CT> constexpr inline parameter_type_t<CT const*> parameter_string{};
	template<typename T> struct config_parameter;
	template<__internal::__can_be_parameter_type T>
	struct config_parameter<T>
	{
		const char* name;
		size_t value_size;
		T value;
	};
	template<__internal::__can_be_parameter_type T, size_t N> 
	struct config_parameter<T[N]>
	{
		const char* name;
		size_t value_size;
		T value[N];
	};
	template<__internal::__basic_char_type CT>
	struct config_parameter<CT const*>
	{
		const char* name;
		size_t value_size;
		CT const* value;
	};
	template<__internal::__can_be_parameter_type T>
	constexpr config_parameter<T> parameter(const char* name, T const& value)
	{
		return config_parameter<T>
		{
			.name		{ name },
			.value_size	{ sizeof(T) },
			.value		{ value }
		};
	}
	template<__internal::__can_be_parameter_type T>
	constexpr config_parameter<T> parameter(const char* name, T&& value)
	{
		return config_parameter<T>
		{
			.name		{ name },
			.value_size	{ sizeof(T) },
			.value		{ std::forward<T>(value) }
		};
	}
	template<__internal::__can_be_parameter T, typename ... Args>
	requires(std::constructible_from<T, Args...>)
	constexpr config_parameter<T> parameter(const char* name, parameter_type_t<T>, Args&& ... args)
	{
		return config_parameter<T>
		{
			.name		{ name },
			.value_size	{ sizeof(T) },
			.value		{ std::forward<Args>(args)... }
		};
	}
	template<__internal::__can_be_parameter_type T, std::convertible_to<T> ... Us>
	constexpr config_parameter<T[sizeof...(Us)]> parameter(const char* name, parameter_type_t<T[]>, Us&& ... values)
	{
		typedef T array_type[sizeof...(Us)];
		return config_parameter<array_type>
		{
			.name		{ name },
			.value_size	{ sizeof(T) * sizeof...(Us) },
			.value		{ std::forward<Us>(values)... }
		};
	}
	struct config_iterator
	{
		typedef generic_config_parameter value_type;
		typedef decltype(std::declval<value_type*>() - std::declval<value_type*>()) difference_type;
		typedef std::add_pointer_t<value_type> pointer;
		typedef decltype(*std::declval<pointer>()) reference;
	private:
		pointer __cur;
	public:
		constexpr config_iterator() noexcept = default;
		constexpr explicit config_iterator(pointer p) noexcept : __cur(p) {}
		constexpr pointer base() const noexcept { return __cur; }
		constexpr pointer operator->() const noexcept { return __cur; }
		constexpr reference operator*() const noexcept { return *__cur; }
		constexpr config_iterator& operator++() noexcept { __cur = __cur->next_in_array(); return *this; }
		constexpr config_iterator operator++(int) noexcept { config_iterator that(__cur); __cur = __cur->next_in_array(); return that; }
		friend constexpr bool operator==(config_iterator const& __this, config_iterator const& that) noexcept { return __this.__cur == that.__cur; }
		friend constexpr std::strong_ordering operator<=>(config_iterator const& __this, config_iterator const& that) noexcept { return __this.__cur <=> that.__cur; }
	};
	struct config_const_iterator
	{
		typedef generic_config_parameter const value_type;
		typedef decltype(std::declval<value_type*>() - std::declval<value_type*>()) difference_type;
		typedef std::add_pointer_t<value_type> pointer;
		typedef decltype(*std::declval<pointer>()) reference;
	private:
		pointer __cur;
	public:
		constexpr config_const_iterator() noexcept = default;
		constexpr config_const_iterator(config_iterator const& that) noexcept : __cur(that.base()) {}
		constexpr explicit config_const_iterator(pointer p) noexcept : __cur(p) {}
		constexpr pointer base() const noexcept { return __cur; }
		constexpr pointer operator->() const noexcept { return __cur; }
		constexpr reference operator*() const noexcept { return *__cur; }
		constexpr config_const_iterator& operator++() noexcept { __cur = __cur->next_in_array(); return *this; }
		constexpr config_const_iterator operator++(int) noexcept { config_const_iterator that(__cur); __cur = __cur->next_in_array(); return that; }
		friend constexpr bool operator==(config_const_iterator const& __this, config_const_iterator const& that) noexcept { return __this.__cur == that.__cur; }
		friend constexpr std::strong_ordering operator<=>(config_const_iterator const& __this, config_const_iterator const& that) noexcept { return __this.__cur <=> that.__cur; }
	};
	struct [[gnu::may_alias]] generic_config_table
	{
		typedef config_iterator iterator;
		typedef config_const_iterator const_iterator;
		size_t size_bytes;
		generic_config_parameter params[];
		constexpr iterator begin() noexcept { return iterator(params); }
		constexpr const_iterator cbegin() const noexcept { return const_iterator(params); }
		constexpr const_iterator begin() const noexcept { return cbegin(); }
		constexpr iterator end() noexcept { return iterator(static_cast<generic_config_parameter*>(static_cast<void*>(static_cast<char*>(static_cast<void*>(params)) + size_bytes))); }
		constexpr const_iterator cend() const noexcept { return const_iterator(static_cast<generic_config_parameter const*>(static_cast<const void*>(static_cast<const char*>(static_cast<const void*>(params)) + size_bytes))); }
		constexpr const_iterator end() const noexcept { return cend(); }
	};
	inline generic_config_table empty_config{ .size_bytes{}, .params{} };
	namespace __internal
	{
		template<typename T> constexpr inline bool __is_empty_and_non_final = __is_empty(T) && !__is_final(T);
		template<size_t I, __can_be_parameter_type ... Ts> struct __config_table_impl;
		template<size_t I, __can_be_parameter_type T, bool = __is_empty_and_non_final<T>> struct __config_entry_base;
		template<size_t I, __can_be_parameter_type T>
		struct __config_entry_base<I, T, false>
		{
			config_parameter<T> __param;
			constexpr __config_entry_base(config_parameter<T>&& p) : __param(std::move(p)) {}
			constexpr static T& __get(__config_entry_base& t) { return t.__param.value; }
			constexpr static T const& __get(__config_entry_base const& t) noexcept { return t.__param.value; }
			constexpr static size_t __size(__config_entry_base const& t) noexcept { return t.__param.value_size; }
		};
		template<size_t I, __can_be_parameter_type T>
		struct __config_entry_base<I, T, true> : T
		{
			const char* name;
			size_t value_size;
			constexpr static size_t __size_value = sizeof(config_parameter<T>);
			constexpr __config_entry_base(config_parameter<T>&& p) : T(std::move(p.value)), name(p.name), value_size(p.value_size) {}
			constexpr static T& __get(__config_entry_base& t) noexcept { return t; }
			constexpr static T const& __get(__config_entry_base const& t) noexcept { return t; }
			constexpr static size_t __size(__config_entry_base const& t) noexcept { return t.value_size; }
		};
		template<size_t I, __can_be_parameter_type T>
		struct __config_table_impl<I, T> : private __config_entry_base<I, T>
		{
			typedef __config_entry_base<I, T> __base;
			template<size_t, __internal::__can_be_parameter_type ...> friend struct config_table_impl;
			constexpr static size_t __size_value = sizeof(config_parameter<T>);
			constexpr static T& __get(__config_table_impl& t) noexcept { return __base::__get(t); }
			constexpr static T const& __get(__config_table_impl const& t) noexcept { return __base::__get(t); }
			constexpr static size_t __size(__config_table_impl const& t) noexcept { return __base::__size(t); }
			constexpr __config_table_impl(config_parameter<T>&& p) : __base(std::move(p)) {}
		};
		template<size_t I, __can_be_parameter_type T, __can_be_parameter_type ... Us>
		struct __config_table_impl<I, T, Us...> : private __config_entry_base<I, T>, public __config_table_impl<I + 1, Us...>
		{
			template<size_t, __internal::__can_be_parameter_type ...> friend struct config_table_impl;
			constexpr static size_t __size_value = sizeof(config_parameter<T>) + __config_table_impl<I + 1, Us ...>::__size_value;
			typedef T __type;
			typedef __config_table_impl<I + 1, Us...> __next;
			typedef __config_entry_base<I, T> __base;
			constexpr static T& __get(__config_table_impl& t) noexcept { return __base::__get(t); }
			constexpr static T const& __get(__config_table_impl const& t) noexcept { return __base::__get(t); }
			constexpr static size_t __size(__config_table_impl const& t) noexcept { return __base::__size(t) + __next::__size(t); }
			constexpr __config_table_impl(config_parameter<T>&& tparam, config_parameter<Us>&&... uparams) : __base(std::move(tparam)), __next(std::forward<config_parameter<Us>>(uparams)...) {}
		};
		template<size_t I, __can_be_parameter_type T, __can_be_parameter_type ... Us> constexpr T& __get(__config_table_impl<I, T, Us...>& t) noexcept { return __config_table_impl<I, T, Us...>::__get(t); }
		template<size_t I, __can_be_parameter_type T, __can_be_parameter_type ... Us> constexpr T const& __get(__config_table_impl<I, T, Us...> const& t) noexcept { return __config_table_impl<I, T, Us...>::__get(t); }
		template<__internal::__can_be_parameter_type ... Ts> constexpr size_t __size(__config_table_impl<0UZ, Ts...> const& t) { return __config_table_impl<0UZ, Ts...>::__size(t); }
	}
	template<__internal::__can_be_parameter_type ... Ts>
	struct [[gnu::may_alias]] module_config_table
	{
		typedef __internal::__config_table_impl<0UZ, Ts...> parameter_types;
		size_t size_bytes;
		parameter_types parameters;
		constexpr void compute_size() noexcept { size_bytes = __internal::__size(parameters); }
	};
	template<__internal::__can_be_parameter_type ... Ts>
	union [[gnu::may_alias]] module_config
	{
		generic_config_table generic;
		module_config_table<Ts...> actual;
		constexpr module_config& compute_size_value() noexcept { actual.compute_size(); return *this; }
	};
	template<__internal::__can_be_parameter_type ... Ts> constexpr module_config<Ts...> create_config(config_parameter<Ts>&& ... params) { return module_config<Ts...>{ .actual{ .parameters{ std::forward<config_parameter<Ts>>(params)... } } }.compute_size_value(); }
	template<size_t I, __internal::__can_be_parameter_type ... Ts> using element_type_t = typename __internal::__nth_pack_param<I, module_config_table<Ts...>>::type;
	template<size_t I, __internal::__can_be_parameter_type ... Ts> constexpr element_type_t<I, Ts...>& get_element(module_config<Ts...>& conf) noexcept { return __internal::__get<I>(conf.actual.parameters); }
	template<size_t I, __internal::__can_be_parameter_type ... Ts> constexpr element_type_t<I, Ts...> const& get_element(module_config<Ts...> const& conf) noexcept { return __internal::__get<I>(conf.actual.parameters); }
	template<typename FT> concept eh_functor			= requires(FT ft, const char* c, int i) { ft(c, i); };
	template<typename MT> concept module_type			= std::derived_from<MT, abstract_module_base>;
	template<typename FT> concept condition_callable	= boolable<decltype(std::declval<FT>()())>;
	struct module_eh_ctx
	{
		jmp_buf handler_ctx;
		int status;
		const char* msg;
	};
	struct cxxabi_abort
	{
		module_eh_ctx* eh_ctx;
		kernel_api* api;
		const char* abort_msg;
		const char* pv_msg;
		[[noreturn]] inline void terminate() { api->ctx_raise(*eh_ctx, abort_msg, -1); __builtin_unreachable(); }
		[[noreturn]] inline void pure_virt() { api->ctx_raise(*eh_ctx, pv_msg, -1); __builtin_unreachable(); }
	};
	template<typename T> constexpr T* pclamp(T* p, T* __min, T* __max) noexcept { return p < __min ? __min : p > __max ? __max : p; }
	extension template<typename T, std::allocator_object<T> AT = std::allocator<T>>
	[[nodiscard]] constexpr T* resize(T* array, size_t ocount, size_t ncount, AT const& alloc)
	{
		if(__unlikely(!array)) return alloc.allocate(ncount);
		if constexpr(requires { { alloc.resize(array, ocount, ncount) } -> std::same_as<T*>; }) return alloc.resize(array, ocount, ncount);
		if constexpr(!std::is_trivially_destructible_v<T>)
		{
			T* result 		= alloc.allocate(ncount);
			size_t ccount 	= ncount < ocount ? ncount : ocount;
			copy_or_move(result, array, ccount);
			if(ccount < ocount) for(size_t i = ccount; i < ocount; i++) array[i].~T();
			alloc.deallocate(array, ocount);
			return result;
		}
		else return static_cast<T*>(std::__detail::__aligned_reallocate(array, ncount, alignof(T)));
	}
	template<typename IT, __internal::__transfer_fn<decltype(*std::declval<IT>())> FT>
	constexpr IT transform(IT start, IT end, FT&& fn) noexcept(noexcept(fn(*start)))
	{
		IT i;
		for(i = start; i != end; i++) *i = fn(*i);
		return i;
	}
	enum class blockdev_type : uint8_t
	{
		BDT_NONE	= 0UC,  // Unknown or no device
		BDT_HDD		= 1UC,  // Hard disk drive
		BDT_CDD		= 2UC,  // CD/DVD drive
		BDT_SSD		= 3UC,  // Solid state drive
	};
	using enum blockdev_type;
	struct abstract_block_device
	{
		typedef unsigned int utticket;
		typedef signed int stticket;
		virtual stticket read(void* dest, uintptr_t src_start, size_t sector_count) 		= 0;
		virtual stticket write(uintptr_t dest_start, const void* src, size_t sector_count) 	= 0;
		virtual size_t block_size() const noexcept 											= 0;
		virtual bool io_busy() const 														= 0;
		virtual bool io_complete(utticket task_ticket) const 								= 0;
		virtual size_t io_count(utticket task_ticket) const 								= 0;
		virtual blockdev_type device_type() const noexcept 									= 0;
		virtual size_t max_operation_blocks() const noexcept 								= 0;
		inline bool io_ready() const { return !io_busy(); }
		struct provider
		{
			typedef unsigned int uidx_t;
			typedef signed int sidx_t;
			virtual uidx_t count() const noexcept 											= 0;
			virtual sidx_t index_of(blockdev_type dev_type) const noexcept 					= 0;
			virtual blockdev_type type_at_index(uidx_t idx) const noexcept 					= 0;
			virtual abstract_block_device& operator[](uidx_t idx) 							= 0;
			virtual abstract_block_device const& operator[](uidx_t idx) const 				= 0;
		};
	};
	struct empty_blockdev_slot : abstract_block_device
	{
		inline virtual stticket read(void* dest, uintptr_t src_start, size_t sector_count) override { return -1; }
		inline virtual stticket write(uintptr_t dest_start, const void* src, size_t sector_count) override { return -1; }
		inline virtual size_t block_size() const noexcept override { return 0UZ; }
		inline virtual bool io_busy() const override { return true; }
		inline virtual bool io_complete(utticket task_ticket) const override { return false; }
		inline virtual size_t io_count(utticket task_ticket) const override { return 0UZ; }
		inline virtual blockdev_type device_type() const noexcept override { return BDT_NONE; }
		inline virtual size_t max_operation_blocks() const noexcept override { return 0UZ; }
	};
	struct netdev_api_helper : public netdev_helper
	{
		virtual bool construct_transfer_buffers() = 0;
		virtual protocol_arp& get_arp() noexcept = 0;
		virtual protocol_ethernet& get_ethernet() noexcept = 0;
		virtual std::ext::resettable_queue<netstack_buffer>& get_transfer_buffers() noexcept = 0;
		virtual ~netdev_api_helper() = default;
		inline netdev_api_helper(mac_t const& mac) : netdev_helper(mac) {}
	};
	template<typename T, std::convertible_to<T> ... Us> constexpr std::array<T, sizeof...(Us)> make_array(Us ... vals) { return std::array<T, sizeof...(Us)>{ static_cast<T>(vals)... }; }
}
namespace std
{
	namespace __detail
	{
		template<typename T> concept __not_void	= !is_same_v<T, void>;
		template<typename T>
		concept __element_gettable	= requires(T& t, T const& ct) {
			{ ooos::get_element<std::declval<size_t>()>(t) } -> __not_void;
			{ ooos::get_element<std::declval<size_t>()>(ct) } -> __not_void;
		};
	}
	template<size_t I, __detail::__element_gettable T> struct tuple_element<I, T> { typedef decltype(ooos::get_element<I>(std::declval<T&>())) type; };
	template<size_t I, __detail::__element_gettable T> struct tuple_element<I, T const> { typedef decltype(ooos::get_element<I>(std::declval<T const&>())) type; };
}
#endif