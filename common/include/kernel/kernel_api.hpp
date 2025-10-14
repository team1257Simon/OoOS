#ifndef __KAPI
#define __KAPI
#include <type_traits>
#include <typeinfo>
#include <compare>
#include <arch/pci.hpp>
#ifdef __KERNEL__
#include "kernel_mm.hpp"
#else
struct kframe_tag;
struct kframe_exports;
#endif
namespace ooos_kernel_module
{
    class abstract_module_base;
    class isr_actor;
    namespace __internal
    {
        class __anything;
        template<typename T> using __ref = T&;
        template<typename T> concept __can_reference = requires { typename __ref<T>; };
        template<typename T> concept __object = std::is_object_v<T>;
        template<typename T> concept __can_dereference = requires(std::add_lvalue_reference_t<T> __t) { { *__t } -> __can_reference; };
        template<typename T> concept __can_reference_and_dereference = __can_reference<T> && __can_dereference<T>;
        template<typename PT, typename VT> concept __points_to = std::is_convertible_v<decltype(*std::declval<PT>()), std::add_lvalue_reference_t<VT>>;
        template<typename PT, typename VT> concept __points_to_maybe_const = __points_to<PT, std::add_const_t<VT>>;
        template<typename PT, typename VT> concept __points_to_const = __points_to_maybe_const<PT, VT> && !__points_to<PT, VT>;
        template<typename IT> concept __has_defined_value_type = requires { typename IT::value_type; };
        template<typename IT> concept __has_implicit_value_type = !__has_defined_value_type<IT> && __can_reference_and_dereference<IT>;
        template<typename IT> concept __has_defined_difference_type = requires { typename IT::difference_type; };
        template<typename IT> concept __has_implicit_difference_type = !__has_defined_difference_type<IT> && requires(std::add_lvalue_reference_t<std::add_const_t<IT>> a, std::add_lvalue_reference_t<std::add_const_t<IT>> b) { { b - a } -> std::integral; };
        template<typename> struct __use_difference_type {};
        template<__has_defined_difference_type IT> struct __use_difference_type<IT> { typedef typename IT::difference_type type; };
        template<__has_implicit_difference_type IT> struct __use_difference_type<IT> { typedef decltype(std::declval<IT>() - std::declval<IT>()) type; };
        template<typename T> struct __use_difference_type<T const> : __use_difference_type<T> {};
        template<typename T> concept __has_difference_type = requires { typename __use_difference_type<T>::type; };
        template<typename> struct __use_value_type {};
        template<__has_defined_value_type IT> struct __use_value_type<IT> { typedef typename IT::value_type type; };
        template<__has_implicit_value_type IT> struct __use_value_type<IT> { typedef std::remove_reference_t<decltype(*std::declval<IT>())> type; };
        template<typename T> struct __use_value_type<T const> : __use_value_type<T> {};
        template<typename T> concept __has_value_type = requires { typename __use_value_type<T>::type; };
        template<typename T> concept __convertible_to_weak = std::is_convertible_v<T, std::weak_ordering>;
        template<typename T> concept __weakly_ordered = requires(T&& t1, T&& t2) { { static_cast<T&&>(t1) <=> static_cast<T&&>(t2) } -> __convertible_to_weak; };
    }
    template<typename T> concept no_args_invoke = requires(T t) { t(); };
    template<typename T> concept wrappable_actor = no_args_invoke<T> && !std::is_same_v<isr_actor, T>;
    template<typename T> concept boolable = requires(T t) { t ? true : false; };
    template<typename T> concept io_buffer_ok = std::is_trivially_copyable_v<T> && !std::is_volatile_v<T>;
    template<typename IT, typename VT> concept value_iterator = __internal::__can_reference_and_dereference<IT> && __internal::__points_to<IT, VT>;
    template<typename IT, typename VT> concept maybe_const_value_iterator = __internal::__can_reference_and_dereference<IT> && __internal::__points_to_maybe_const<IT, std::remove_const_t<VT>>;
    template<typename IT, typename VT> concept const_value_iterator = __internal::__can_reference_and_dereference<IT> && __internal::__points_to_const<IT, std::remove_const_t<VT>>;
    template<__internal::__has_difference_type IT> using iterator_difference_t = typename __internal::__use_difference_type<IT>::type;
    template<__internal::__has_value_type IT> using dereference_value_t = typename __internal::__use_value_type<IT>::type;
    template<typename IT> concept incrementable_iterator = __internal::__has_value_type<IT> && __internal::__has_difference_type<IT>;
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
        constexpr simple_iterator() noexcept : current{} {}
        constexpr explicit simple_iterator(IT const& i) noexcept : current{ i } {}
        template<typename JT> requires maybe_const_value_iterator<IT, typename dereference_value<JT>::type> constexpr simple_iterator(simple_iterator<JT> const& that) noexcept : current{ that.base() } {}
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
        virtual void* mem_allocate(size_t size, size_t align) = 0;
        virtual void mem_release(void* block, size_t align) = 0;
        virtual kframe_tag* get_frame() = 0;
        virtual ~kmod_mm() = default;
    };
    union actors
    {
        void* object;
        const void* const_object;
        void (*function_ptr)();
        void (abstract_module_base::*member_function_ptr)();
    };
    union [[gnu::may_alias]] functor_store
    {
        actors ignored;
        char actual[sizeof(actors)];
        constexpr void* access() noexcept { return &actual[0]; }
        constexpr const void* access() const noexcept { return &actual[0]; }
        template<typename T> constexpr T& cast() noexcept { return *static_cast<T*>(access()); }
        template<typename T> constexpr T const& cast() const noexcept { return *static_cast<T const*>(access()); }
    };
    enum mgr_op
    {
        get_pointer,
        clone,
        destroy
    };
    struct isr_actor_base
    {
        template<typename R, typename C, typename ... Args> using member_fn = R (C::*)(Args...);
        template<typename GT> constexpr static bool not_empty(GT* gt) noexcept { return gt != nullptr; }
        template<typename R, typename C, typename ... Args> constexpr static bool not_empty(member_fn<R, C, Args...> mf) noexcept { return mf != nullptr; }
        template<boolable T> constexpr static bool not_empty(T t) noexcept { return t ? true : false; }
        template<typename T> constexpr static bool not_empty(T t) noexcept { return true; }
        template<no_args_invoke FT>
        class actor_manager
        {
            constexpr static size_t __max_size  = sizeof(actors);
            constexpr static size_t __max_align = alignof(actors);
            template<typename GT> static inline void __create_wrapper(functor_store& dst, GT&& ftor, kmod_mm*, std::true_type) noexcept(std::is_nothrow_constructible<FT, GT>::value) { new(dst.access()) FT(static_cast<GT&&>(ftor)); }
            template<typename GT> static inline void __create_wrapper(functor_store& dst, GT&& ftor, kmod_mm* mm, std::false_type) { dst.template cast<FT*>() = new(mm->mem_allocate(sizeof(FT), alignof(FT))) FT(static_cast<GT&&>(ftor)); }
            static inline void __delete_wrapper(functor_store& target, kmod_mm*, std::true_type) { target.template cast<FT>().~FT(); }
            static inline void __delete_wrapper(functor_store& target, kmod_mm* mm, std::false_type) { mm->mem_release(target.access(), alignof(FT)); }
            using __is_locally_storable = std::bool_constant<std::is_trivially_copyable_v<FT> && (sizeof(FT) <= __max_size && alignof(FT) <= __max_align && __max_align % alignof(FT) == 0)>;
            constexpr static bool __is_local_store = __is_locally_storable::value;
        public:
            constexpr static FT* get_ptr(functor_store const& src) { if constexpr(__is_local_store) { return &const_cast<FT&>(src.template cast<FT>()); } else { return src.template cast<FT*>(); } }
            template<typename GT> static inline void init_actor(functor_store& dst, GT&& src, kmod_mm* mm) noexcept(std::is_nothrow_constructible<FT, GT>::value && __is_local_store) { __create_wrapper(dst, static_cast<GT&&>(src), mm, __is_locally_storable()); }
            static inline void destroy_actor(functor_store& dst, kmod_mm* mm) { __delete_wrapper(dst, mm, __is_locally_storable()); }
            static void invoke_fn(functor_store& fn) { FT* ptr = get_ptr(fn); if(ptr && not_empty(*ptr)) { (*ptr)(); } }
            static void action(functor_store& dst, functor_store const& src, kmod_mm* mm, mgr_op op)
            {
                switch(op)
                {
                case get_pointer:
                    dst.template cast<FT*>() = get_ptr(src);
                    break;
                case clone:
                    init_actor(dst, *const_cast<FT const*>(get_ptr(src)), mm);
                    break;
                case destroy:
                    destroy_actor(dst, mm);
                    break;
                }
            }
        };
        using manager_type = void(*)(functor_store&, functor_store const&, kmod_mm*, mgr_op);
        using invoker_type = void(*)(functor_store&);
        template<no_args_invoke FT> constexpr static manager_type get_manager(FT ft) { return not_empty(ft) ? &actor_manager<FT>::action : nullptr; }
        template<no_args_invoke FT> constexpr static invoker_type get_invoker(FT ft) { return not_empty(ft) ? &actor_manager<FT>::invoke_fn : nullptr; }
        struct actor_manager_wrapper
        {
            kmod_mm* mm;
            manager_type manager;
            invoker_type invoker;
            void manager_action(functor_store& dst, functor_store const& src, mgr_op op);
            void invoke(functor_store& fn);
            constexpr operator bool() const noexcept { return mm && manager && invoker; }
        };
        actor_manager_wrapper __my_wrapper;
        functor_store __my_actor;
        constexpr isr_actor_base() noexcept = default;
        constexpr isr_actor_base(manager_type mgr, invoker_type inv, kmod_mm* mm) noexcept : __my_wrapper(mm, mgr, inv), __my_actor() {}
        constexpr isr_actor_base(actor_manager_wrapper const& wrapper) noexcept : __my_wrapper(wrapper) {}
        constexpr isr_actor_base(actor_manager_wrapper&& wrapper) noexcept : __my_wrapper(wrapper) { wrapper.invoker = nullptr; wrapper.manager = nullptr; wrapper.mm = nullptr; }
        ~isr_actor_base() noexcept;
    };
    struct isr_actor : private isr_actor_base
    {
        isr_actor() noexcept;
        ~isr_actor() noexcept;
        template<wrappable_actor FT> inline isr_actor(FT&& ft, kmod_mm* mm) : isr_actor_base(get_manager(ft), get_invoker(ft), mm) { if(not_empty(ft)) actor_manager<FT>::init_actor(__my_actor, static_cast<FT&&>(ft), mm); }
        constexpr operator bool() const noexcept { return __my_wrapper; }
        isr_actor(isr_actor const& that);
        isr_actor(isr_actor&& that);
        isr_actor& operator=(isr_actor const& that);
        isr_actor& operator=(isr_actor&& that);
        void operator()();
    };
    struct kernel_api
    {
        friend class abstract_module_base;
        virtual void* allocate_dma(size_t size, bool prefetchable) = 0;
        virtual void release_dma(void* ptr, size_t size) = 0;
        virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) = 0;
        virtual void* acpi_get_table(const char* label) = 0;
        virtual void on_irq(uint8_t irq, isr_actor&& handler, abstract_module_base* owner) = 0;
        virtual void remove_actors(abstract_module_base* owner) = 0;
        virtual kmod_mm* create_mm() = 0;
        virtual void destroy_mm(kmod_mm* mm) = 0;
        virtual void log(std::type_info const& from, const char* message) = 0;
        virtual void init_memory_fns(kframe_exports* ptrs) = 0;
    };
    kernel_api* get_api_instance();
    void init_api();
    template<__internal::__weakly_ordered T> [[nodiscard]] constexpr T&& clamp(T&& mini, T&& maxi, T&& t) noexcept { return static_cast<T&&>(t > mini ? (t < maxi ? t : maxi) : mini); }
}
#endif