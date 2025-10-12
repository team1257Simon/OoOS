#ifndef __KAPI
#define __KAPI
#include <type_traits>
#include <typeinfo>
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
    template<typename T> concept no_args_invoke = requires(T t) { t(); };
    template<typename T> concept wrappable_actor = no_args_invoke<T> && !std::is_same_v<isr_actor, T>;
    template<typename T> concept boolable = requires(T t) { t ? true : false; };
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
}
#endif