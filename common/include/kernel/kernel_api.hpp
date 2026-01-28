#ifndef __KAPI
#define __KAPI
#include <stdarg.h>
#include <fs/dev_stream.hpp>
#include <arch/pci.hpp>
#include <typeinfo>
#include <optional>
#include <net/protocol/arp.hpp>
#include <bits/unique_ptr.hpp>
#include <bits/stl_queue.hpp>
struct kframe_tag;
struct kframe_exports;
struct sysfs_vnode;
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
		template<typename T> concept __can_inherit = std::is_class_v<T> && !std::is_final_v<T>;
		template<typename T, typename ... Us> concept __not_in = !(sizeof...(Us)) || !(std::is_same_v<T, Us> || ...);
		template<typename...> struct __no_repeats_helper;
		template<> struct __no_repeats_helper<> : std::true_type {};
		template<typename T> struct __no_repeats_helper<T> : std::true_type {};
		template<typename T, typename ... Us>
		struct __no_repeats_helper<T, Us ...> : std::__and_<std::bool_constant<__not_in<T, Us...>>, __no_repeats_helper<Us...>> {};
		template<typename ... Ts> concept __non_repeating = __no_repeats_helper<Ts...>::value;
		template<typename ... Ts> concept __can_inherit_all = __non_repeating<Ts...> && (__can_inherit<Ts> && ...);
		template<typename, typename U> struct __qualify_like { typedef std::remove_cvref_t<U> type; };
		template<typename T, typename U> struct __qualify_like<T const, U> { typedef std::remove_cvref_t<U> const type; };
		template<typename T, typename U> struct __qualify_like<T volatile, U> { typedef std::remove_cvref_t<U> volatile type; };
		template<typename T, typename U> struct __qualify_like<T const volatile, U> { typedef volatile std::remove_cvref_t<T> const type; };
	}
	template<typename T, typename U> using like_qualified_pointer				= std::add_pointer_t<typename __internal::__qualify_like<T, U>::type>;
	template<template<typename> class C, typename ... Ts> requires(__internal::__can_inherit_all<C<Ts>...>) struct repeated_template : C<Ts> ... {};
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
	template<typename IT> concept incrementable_iterator 						= std::input_or_output_iterator<IT> && std::incrementable<IT>;
	template<incrementable_iterator IT>
	struct simple_iterator : public std::iterator_traits<IT>
	{
		using typename std::iterator_traits<IT>::value_type;
		using typename std::iterator_traits<IT>::difference_type;
		using typename std::iterator_traits<IT>::reference;
		using typename std::iterator_traits<IT>::pointer;
		using typename std::iterator_traits<IT>::iterator_concept;
		using typename std::iterator_traits<IT>::iterator_category;
	protected:
		IT current;
		typedef decltype(std::declval<difference_type>() <=> std::declval<difference_type>()) order_type;
	public:
		constexpr IT const& base() const noexcept { return current; }
		simple_iterator() noexcept requires(std::default_initializable<IT>) = default;
		constexpr explicit simple_iterator(IT i) noexcept(std::is_nothrow_constructible_v<IT, IT>) : current(i) {}
		template<std::not_self<IT> JT> requires(std::is_constructible_v<IT, JT const&>)
		constexpr simple_iterator(simple_iterator<JT> const& that) noexcept : current(that) {}
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
		isr_actor& operator=(nullptr_t) noexcept { isr_actor().swap(*this); return *this; }
		constexpr void operator()() { if(!__empty()) __my_invoke(__my_actor); }
		constexpr std::type_info const& target_type() const noexcept { if(__my_manager) { functor_store tmp_store; __my_manager(tmp_store, __my_actor, __my_alloc, get_type_info); if(std::type_info const* result = tmp_store.cast<std::type_info const*>()) return *result; } return typeid(nullptr); }
	};
	struct sysfstream
	{
		sysfs_vnode& node;
		virtual int write(const void* src, size_t n);
		virtual int read(void* dst, size_t n);
		virtual int seekpos(std::streampos where);
		virtual int seekoff(std::streamoff off, std::ios_base::seekdir way);
		virtual ~sysfstream();
		constexpr sysfstream(sysfs_vnode& n) noexcept : node(n) {}
	private:
		bool dirty	= false;
	};
	struct netdev_api_helper;
	class abstract_netdev_module;
	struct kernel_api
	{
		friend class abstract_module_base;
		virtual void* allocate_dma(size_t size, bool prefetchable) 																			= 0;
		virtual void release_dma(void* ptr, size_t size) 																					= 0;
		virtual void* map_dma(uintptr_t addr, size_t sz, bool prefetchable)																	= 0;
		virtual size_t dma_size(size_t requested)																							= 0;
		virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) 													= 0;
		virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass, uint8_t prog_if)									= 0;
		virtual void* acpi_get_table(const char* label) 																					= 0;
		virtual uintptr_t vtranslate(void* addr) noexcept																					= 0;
		virtual std::pair<std::optional<sysfstream>, int> sysfs_open(const char* name, bool create) noexcept								= 0;
		virtual void on_irq(uint8_t irq, isr_actor&& handler, abstract_module_base* owner) 													= 0;
		virtual void remove_actors(abstract_module_base* owner) 																			= 0;
		virtual kmod_mm* create_mm() 																										= 0;
		virtual void destroy_mm(kmod_mm* mm) 																								= 0;
		virtual void log(std::type_info const& from, const char* message) 																	= 0;
		virtual uint32_t register_device(dev_stream<char>* stream, device_type type)	 													= 0;
		virtual bool deregister_device(dev_stream<char>* stream) 																			= 0;
		virtual void init_memory_fns(kframe_exports* ptrs) 																					= 0;
		[[noreturn]] virtual void ctx_raise(module_eh_ctx& ctx, const char* msg, int status) 												= 0;
		[[nodiscard]] virtual netdev_api_helper* create_net_helper(abstract_netdev_module& mod)												= 0;
		virtual size_t vformat(kmod_mm* mm, const char* fmt, const char*& out, va_list args) 												= 0;
		virtual size_t vlogf(std::type_info const& from, const char* fmt, va_list args) 													= 0;
		virtual bool export_type_info(std::type_info const& ti)																				= 0;
		virtual bool import_type_info(std::type_info const& ti)																				= 0;
		virtual bool msi(abstract_module_base* owner, std::array<isr_actor, 7>&& handlers, msi32_t volatile& reg, msi_trigger_mode mode)	= 0;
		virtual bool msi(abstract_module_base* owner, std::array<isr_actor, 7>&& handlers, msi64_t volatile& reg, msi_trigger_mode mode)	= 0;
		virtual bool extended_msi(abstract_module_base* owner, isr_actor&& handler, msix_t volatile& reg, msi_trigger_mode mode)			= 0;
		virtual int save_config(abstract_module_base* mod)																					= 0;
		virtual int load_config(abstract_module_base* mod)																					= 0;
	protected:
		virtual void register_type_info(std::type_info const* ti) 																			= 0;
		virtual void relocate_type_info(abstract_module_base* mod, std::type_info const* local_si, std::type_info const* local_vmi) 		= 0;
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
		typedef std::true_type is_open_coded;
		typedef std::true_type can_memcpy;
		size_t size_bytes;
		generic_config_parameter params[];
		constexpr iterator begin() noexcept { return iterator(params); }
		constexpr const_iterator cbegin() const noexcept { return const_iterator(params); }
		constexpr const_iterator begin() const noexcept { return cbegin(); }
		constexpr iterator end() noexcept { return iterator(static_cast<generic_config_parameter*>(static_cast<void*>(static_cast<char*>(static_cast<void*>(params)) + size_bytes))); }
		constexpr const_iterator cend() const noexcept { return const_iterator(static_cast<generic_config_parameter const*>(static_cast<const void*>(static_cast<const char*>(static_cast<const void*>(params)) + size_bytes))); }
		constexpr const_iterator end() const noexcept { return cend(); }
		constexpr size_t size() const noexcept { return size_bytes; }
		constexpr bool empty() const noexcept { return !size_bytes; }
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
		virtual bool construct_transfer_buffers()												= 0;
		virtual protocol_arp& get_arp() noexcept												= 0;
		virtual protocol_ethernet& get_ethernet() noexcept										= 0;
		virtual std::ext::resettable_queue<netstack_buffer>& get_transfer_buffers() noexcept	= 0;
		virtual ~netdev_api_helper()															= default;
		inline netdev_api_helper(mac_t const& mac) : netdev_helper(mac) {}
	};
	template<trivial_copy T, std::allocator_object<T> A = std::allocator<T>>
	class simple_buffer : protected std::__impl::__dynamic_buffer<T, A, false>
	{
		typedef std::__impl::__dynamic_buffer<T, A, false> __base;
		friend class std::vector<T, A>;
	public:
		typedef typename __base::__value_type value_type;
		typedef typename __base::__allocator_type allocator_type;
		typedef typename __base::__size_type size_type;
		typedef typename __base::__difference_type difference_type;
		typedef typename __base::__reference reference;
		typedef typename __base::__const_reference const_reference;
		typedef typename __base::__pointer pointer;
		typedef typename __base::__const_pointer const_pointer;
		typedef simple_iterator<pointer> iterator;
		typedef std::const_iterator<iterator> const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	private:
		consteval static bool __nt_move_assign() noexcept { return std::is_nothrow_move_assignable_v<allocator_type> || !std::__has_move_propagate<allocator_type>; }
	public:
		constexpr explicit simple_buffer() noexcept(noexcept(allocator_type())) : __base() {}
		constexpr explicit simple_buffer(allocator_type const& alloc) noexcept(noexcept(allocator_type(alloc))) : __base(alloc) {}
		constexpr simple_buffer(size_type count, allocator_type const& alloc = allocator_type()) : __base(count, alloc) {}
		constexpr simple_buffer(size_type count, const_reference val, allocator_type const& alloc = allocator_type()) : __base(count, val, alloc) {}
		constexpr simple_buffer(simple_buffer const& that) : __base(that) {}
		constexpr simple_buffer(simple_buffer&& that) noexcept(std::is_nothrow_move_constructible_v<allocator_type>) : __base(std::forward<__base>(that)) {}
		template<trivial_copy U, typename ... Args> requires(std::is_integral_v<T> && std::constructible_from<U, Args...>)
		constexpr simple_buffer(allocator_type const& alloc, std::in_place_type_t<U>, Args&& ... args);
		template<trivial_copy U, typename ... Args> requires(std::is_integral_v<T> && std::constructible_from<U, Args...>)
		constexpr simple_buffer(std::in_place_type_t<U> tag, Args&& ... args) : simple_buffer(allocator_type(), tag, std::forward<Args>(args)...) {}
		constexpr simple_buffer& operator=(simple_buffer const& that) { this->__copy_assign(that); return *this; }
		constexpr simple_buffer& operator=(simple_buffer&& that) noexcept(__nt_move_assign()) { this->__move_assign(std::forward<__base>(that)); return *this; }
		constexpr void swap(simple_buffer& that) noexcept { this->__swap(that); }
		constexpr void swap(std::vector<T, A>& that) noexcept { that.swap_like(*this); }
		constexpr pointer data() noexcept { return this->__beg(); }
		constexpr const_pointer data() const noexcept { return this->__beg(); }
		constexpr allocator_type get_allocator() const noexcept { return this->__get_alloc(); }
		constexpr iterator begin() noexcept { return iterator(this->__beg()); }
		constexpr const_iterator cbegin() const noexcept { return const_iterator(this->__beg()); }
		constexpr const_iterator begin() const noexcept { return cbegin(); }
		constexpr iterator end() noexcept { return iterator(this->__cur()); }
		constexpr const_iterator cend() const noexcept { return const_iterator(this->__cur()); }
		constexpr const_iterator end() const noexcept { return cend(); }
		constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
		constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
		constexpr const_reverse_iterator rbegin() const noexcept { return crbegin(); }
		constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
		constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
		constexpr const_reverse_iterator rend() const noexcept { return crend(); }
		constexpr size_type size() const noexcept { return this->__size(); }
		constexpr size_type capacity() const noexcept { return this->__capacity(); }
		constexpr bool empty() const noexcept { return !this->size(); }
		constexpr void clear() noexcept(noexcept(this->__destroy())) { this->__destroy(); }
		constexpr size_type write(const_pointer src, size_type n) { return static_cast<size_type>(std::max(this->__append_elements(src, src + n) - this->__beg(), 0Z)); }
		constexpr iterator push(const_reference val) { return iterator(this->__append_element(val)); }
		template<trivial_copy U, typename ... Args> requires(std::is_integral_v<T> && std::constructible_from<U, Args...>) constexpr U& emplace(Args&& ... args);
	};
	template<trivial_copy T, std::allocator_object<T> A>
	template<trivial_copy U, typename ... Args> requires(std::is_integral_v<T> && std::constructible_from<U, Args...>)
	constexpr simple_buffer<T, A>::simple_buffer(allocator_type const& alloc, std::in_place_type_t<U>, Args&& ... args) : __base(sizeof(U) / sizeof(T), alloc) {
		if(!empty()) {
			new(data()) U(std::forward<Args>(args)...);
			this->__setc(sizeof(U) / sizeof(T));
		}
	}
	template<trivial_copy T, std::allocator_object<T> A>
	template<trivial_copy U, typename ... Args> requires(std::is_integral_v<T> && std::constructible_from<U, Args...>)
	constexpr U& simple_buffer<T, A>::emplace(Args&& ... args)
	{
		this->__destroy();
		this->__grow_buffer_exact(sizeof(U) / sizeof(T));
		U* result	= new(this->data()) U(std::forward<Args>(args)...);
		this->__setc(sizeof(U) / sizeof(T));
		return *result;
	}
	template<condition_callable FT>
	constexpr bool await_completion(time_t max_spin, FT&& ft)
	{
		for(time_t spin = 0UZ; spin < max_spin; spin++)
			if(ft())
				return true;
		return ft();
	}
	template<typename T, std::convertible_to<T> ... Us> constexpr std::array<T, sizeof...(Us)> make_array(Us ... vals) { return std::array<T, sizeof...(Us)>{ static_cast<T>(vals)... }; }
	namespace __internal
	{
		template<trivial_copy T>
		struct __aligned_deleter
		{
			std::align_val_t align	= static_cast<std::align_val_t>(alignof(T));
			constexpr void operator()(T* ptr) const noexcept(std::is_nothrow_destructible_v<T>)
			{
				if constexpr(!std::is_trivially_destructible_v<T>)
					ptr->~T();
				operator delete(ptr, align);
			}
		};
		template<trivial_copy T>
		struct __aligned_deleter<T[]> {
			std::align_val_t align	= static_cast<std::align_val_t>(alignof(T));
			constexpr void operator()(T ptr[]) const noexcept {	operator delete[](ptr, align); }
		};
	}
	typedef std::unique_ptr<uint8_t[], __internal::__aligned_deleter<uint8_t[]>> aligned_buffer_ptr;
	template<typename T> constexpr std::align_val_t align_of() noexcept { return static_cast<std::align_val_t>(alignof(T)); }
	template<size_t N> constexpr std::align_val_t align_to() noexcept { return static_cast<std::align_val_t>(N); }
	template<trivial_copy T> class device_message;
	class generic_device_message
	{
		typedef __internal::__aligned_deleter<uint8_t[]> __deleter;
		aligned_buffer_ptr __data;
		size_t __size;
		std::ext::type_erasure __type;
		template<trivial_copy T> constexpr static T* __open_alloc(size_t size, std::align_val_t al) { return static_cast<T*>(operator new[](size, al)); }
		template<trivial_copy T, typename ... Args> requires(std::constructible_from<T, Args...>)
		constexpr static addr_t __create(size_t addsz, Args&& ... args) { return std::construct_at(__open_alloc<T>(sizeof(T) + addsz, align_of<T>()), std::forward<Args>(args)...); }
		constexpr static uint8_t* __clone(uint8_t* ptr, size_t size, std::align_val_t al) { return array_copy(__open_alloc<uint8_t>(size, al), ptr, size); }
	protected:
		template<trivial_copy T> requires(std::move_constructible<T>)
		constexpr generic_device_message(T&& t, size_t addsz = 0UZ) :
			__data(__create(addsz, std::move(t)), __deleter(align_of<T>())),
			__size(sizeof(T) + addsz),
			__type(typeid(T))
		{}
		template<trivial_copy T, typename ... Args> requires(std::constructible_from<T, Args...>)
		constexpr generic_device_message(std::in_place_type_t<T>, Args&& ... args) :
			__data(__create(0UZ, std::forward<Args>(args)...), __deleter(align_of<T>())),
			__size(sizeof(T)),
			__type(typeid(T))
		{}
		template<trivial_copy T, typename ... Args> requires(std::constructible_from<T, Args...>)
		constexpr generic_device_message(size_t addsz, std::in_place_type_t<T>, Args&& ... args) :
			__data(__create(addsz, std::forward<Args>(args)...), __deleter(align_of<T>())),
			__size(sizeof(T) + addsz),
			__type(typeid(T))
		{}
		void* get_ptr() const noexcept { return __data.get(); }
		__deleter const& get_deleter() const noexcept { return __data.get_deleter(); }
		std::align_val_t get_align() const noexcept { return __data.get_deleter().align; }
	public:
		template<trivial_copy T> device_message<T>* as() noexcept { return __type.is_derived_from(typeid(T)) ? addr_t(this) : nullptr; }
		template<trivial_copy T> device_message<T> const* as() const noexcept { return __type.is_derived_from(typeid(T)) ? addr_t(this) : nullptr; }
		constexpr size_t size() const noexcept { return __size; }
		constexpr std::ext::type_erasure const& type() const noexcept { return __type; }
		generic_device_message(generic_device_message&&) = default;
		generic_device_message& operator=(generic_device_message&&) = default;
		constexpr generic_device_message(generic_device_message const& that) :
			__data(__clone(that.__data.get(), that.__size, that.get_align()), that.get_deleter()),
			__size(that.__size),
			__type(that.__type)
		{}
		constexpr generic_device_message& operator=(generic_device_message const& that)
		{
			this->__data.reset(__clone(that.__data.get(), that.__size, that.get_align()));
			this->__data.get_deleter().align	= that.get_align();
			this->__size						= that.__size;
			this->__type						= that.__type;
			return *this;
		}
	};
	template<trivial_copy T>
	class device_message : public generic_device_message
	{
		template<trivial_copy U> constexpr static bool __can_attempt_cast() noexcept { return std::derived_from<T, U> || std::derived_from<U, T>; }
	public:
		constexpr device_message() requires(std::default_initializable<T>) : generic_device_message(std::in_place_type<T>) {}
		constexpr device_message(T&& t) requires(std::move_constructible<T>) : generic_device_message(std::move(t)) {}
		constexpr device_message(T&& t, size_t added_size) requires(std::move_constructible<T>) : generic_device_message(std::move(t), added_size) {}
		template<typename ... Args> requires(std::constructible_from<T, Args...>)
		constexpr device_message(std::in_place_t, Args&& ... args) :  generic_device_message(std::in_place_type<T>, std::forward<Args>(args)...) {}
		template<typename ... Args> requires(std::constructible_from<T, Args...>)
		constexpr device_message(size_t added_size, Args&& ... args) : generic_device_message(added_size, std::in_place_type<T>, std::forward<Args>(args)...) {}
		constexpr T* operator->() const noexcept { return static_cast<T*>(this->get_ptr()); }
		constexpr T& operator*() const noexcept { return *static_cast<T*>(this->get_ptr()); }
		constexpr T* base() const noexcept { return static_cast<T*>(this->get_ptr()); }
		template<trivial_copy U> requires(__can_attempt_cast<U>()) U* cast_to() const noexcept { return this->type().template cast_to<U>(this->get_ptr()); }
	};
	template<typename FT, trivial_copy T, typename ... BArgs> requires(std::default_initializable<T>)
	class bound_message_buffer_functor
	{
		device_message<T> __msg_buf;
		std::__bind_front_expr<FT, BArgs...> __fn;
	public:
		constexpr T* base() const noexcept { return __msg_buf.base(); }
		template<typename GT, typename ... Args> requires(sizeof...(Args) == sizeof...(BArgs) && std::convertible_to<GT, FT>)
		constexpr explicit bound_message_buffer_functor(GT&& fn, Args&& ... args) : __msg_buf(), __fn(std::bind_front(std::forward<GT>(fn), std::forward<Args>(args)...)) {}
		template<typename ... CArgs>
		constexpr decltype(auto) operator()(CArgs&& ... args) const { return __fn(*__msg_buf, std::forward<CArgs>(args)...); }
	};
	template<trivial_copy T, typename FT, typename ... Args>
	constexpr bound_message_buffer_functor<FT, T, Args...> bind_for_message(std::in_place_type_t<T>, FT&& fn, Args&& ... args) {}
	//	Abstract base for devices connected via protocols like USB.
	struct abstract_connectable_device
	{
		//	Interface for passing messages to and from the device. This will be provided by the hub driver.
		struct interface
		{
			//	These match the encodings for endpoint types used by XHCI.
			enum class type : uint8_t
			{
				NONE			= 0UC,
				H2D_SYNC		= 1UC,
				H2D_BULK		= 2UC,
				H2D_NOTIFY		= 3UC,
				CONTROL			= 4UC,
				D2H_SYNC		= 5UC,
				D2H_BULK		= 6UC,
				D2H_NOTIFY		= 7UC,
			};
			//	Hook for host-to-device transfers.
			virtual void put_msg(generic_device_message const& msg)			= 0;
			//	Hook for device-to-host transfers.
			virtual std::optional<generic_device_message> poll_msg()		= 0;
			//	Identify the direction and transfer type of the interface.
			virtual type interface_type() const								= 0;
		};
		//	Represents the device hub and/or host controller.
		struct provider
		{
			//	Gets a pointer to the device at the given slot ID. If the ID is out of range (i.e. no such slot exists), returns a null pointer.
			virtual abstract_connectable_device* operator[](size_t id)						= 0;
			//	If the device pointed to by dev is part of this hub/controller, returns its slot; otherwise returns std::nullopt.
			virtual std::optional<size_t> index_of(abstract_connectable_device* dev) const	= 0;
			//	Queries the number of slots on the hub/controller.
			virtual size_t size() const														= 0;
		};
		//	Counts the total number of interfaces to the device (e.g. USB endpoints).
		virtual size_t interface_count() const		= 0;
		//	Gets a pointer to the hub/controller to which the device is connected.
		virtual provider* parent()					= 0;
		//	Gets a pointer to the interface at the given index for the device. If the ID is out of range, returns a null pointer.
		virtual interface* operator[](size_t idx)	= 0;
		using enum interface::type;
	};
}
namespace std
{
	namespace __detail
	{
		template<typename T> concept __not_void	= !is_same_v<T, void>;
		template<typename T, size_t I>
		concept __element_gettable	= requires(T& t, T const& ct) {
			{ ooos::get_element<I>(t) } -> __not_void;
			{ ooos::get_element<I>(ct) } -> __not_void;
		};
	}
	template<size_t I, __detail::__element_gettable<I> T> struct tuple_element<I, T> { typedef decltype(ooos::get_element<I>(std::declval<T&>())) type; };
	template<size_t I, __detail::__element_gettable<I> T> struct tuple_element<I, T const> { typedef decltype(ooos::get_element<I>(std::declval<T const&>())) type; };
}
#endif