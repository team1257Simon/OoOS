#ifndef __KMOD
#define __KMOD
#include "kernel_api.hpp"
#include <new>
namespace ooos_kernel_module
{
    class abstract_module_base
    {
        kernel_api* __api_hooks;
        kmod_mm* __allocated_mm;
        void (*__fini_fn)();
    public:    
        virtual void initialize() = 0;
        virtual void finalize() = 0;
        inline abstract_module_base* tie_api_mm(kernel_api* api, kmod_mm* mm) { if(!__api_hooks && !__allocated_mm && api && mm) { __api_hooks = api; __allocated_mm = mm; } return this; }
        inline void* allocate_dma(size_t size, bool prefetchable) { return __api_hooks->allocate_dma(size, prefetchable); }
        inline pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) { return __api_hooks->find_pci_device(device_class, subclass); }
        inline void* acpi_get_table(const char* label) { return __api_hooks->acpi_get_table(label); }
        inline void log(const char* msg) { __api_hooks->log(typeid(*this), msg); }
        template<wrappable_actor FT> inline void on_irq(uint8_t irq, FT&& handler) { __api_hooks->on_irq(irq, static_cast<isr_actor&&>(isr_actor(static_cast<FT&&>(handler), __allocated_mm)), this); }
        inline void setup(kernel_api* api, kmod_mm* mm, void (*fini)()) { if(api && mm && fini && !(__api_hooks || __allocated_mm || __fini_fn)) { __api_hooks = api; __allocated_mm = mm; __fini_fn = fini; } }
        friend void module_takedown(abstract_module_base* mod);
    };
    void module_takedown(abstract_module_base* mod);
    template<typename T, typename ... Args>
    abstract_module_base* setup_instance(void* addr, kernel_api* api, kframe_tag** frame_tag, kframe_exports* kframe_fns, void (*init)(), void (*fini)(), Args&& ... args)
    {
        if(addr && api && frame_tag && kframe_fns && fini)
        {
            if(kmod_mm* mm = api->create_mm())
            {
                *frame_tag = mm->get_frame();
                api->init_memory_fns(kframe_fns);
                (*init)();
                T* result = new (addr) T(static_cast<Args&&>(args)...);
                result->setup(api, mm, fini);
                return result;
            }
        }
        return nullptr;
    }
    template<io_buffer_ok T>
    class io_module_base : public abstract_module_base
    {
    public:
        typedef T value_type;
        typedef value_type* pointer;
        typedef value_type const* const_pointer;
        typedef value_type& reference;
        typedef value_type const& const_reference;
        typedef simple_iterator<pointer> iterator;
        typedef simple_iterator<const_pointer> const_iterator;
        typedef typename iterator::difference_type difference_type;
    protected:
        struct io_buffer
        {
            pointer beg;
            pointer cur;
            pointer fin;
            constexpr size_t size() const noexcept { return cur > beg ? static_cast<size_t>(cur - beg) : 0UZ; }
            constexpr size_t capacity() const noexcept { return fin > beg ? static_cast<size_t>(fin - beg) : 0UZ; }
            constexpr size_t remaining() const noexcept { return (beg && fin > cur) ? static_cast<size_t>(fin - cur) : 0UZ; }
            constexpr void set(pointer b, pointer c, pointer e) noexcept { beg = b; cur = c; fin = e; }
            constexpr void reset() noexcept { beg = cur = fin = pointer(); }
            constexpr io_buffer() noexcept = default;
            constexpr ~io_buffer() noexcept = default;
            constexpr io_buffer(pointer b, pointer c, pointer e) noexcept : beg(b), cur(c), fin(e) {}
            constexpr io_buffer(pointer b, pointer e) noexcept : beg(b), cur(b), fin(e) {}
            constexpr io_buffer(pointer b, size_t n) noexcept : beg(b), cur(b), fin(b + n) {}
            constexpr io_buffer(io_buffer&& that) noexcept : beg(that.beg), cur(that.cur), fin(that.fin) { that.reset(); }
            constexpr void swap(io_buffer& that) noexcept { io_buffer tmp(this->beg, this->cur, this->fin); this->set(that.beg, that.cur, that.fin); that.set(tmp.beg, tmp,cur, tmp.fin); }
            constexpr io_buffer& operator=(io_buffer&& that) noexcept { io_buffer(static_cast<io_buffer&&>(that)).swap(*this); return *this; }
            constexpr io_buffer& operator=(nullptr_t) noexcept { reset(); return *this; }
            constexpr iterator begin() noexcept { return iterator(beg); }
            constexpr const_iterator cbegin() const noexcept { return const_iterator(beg); }
            constexpr const_iterator begin() const noexcept { return const_iterator(beg); }
            constexpr iterator end() noexcept { return iterator(cur); }
            constexpr const_iterator end() const noexcept { return const_iterator(cur); }
            constexpr const_iterator cend() const noexcept { return const_iterator(cur); }
            constexpr void bump(difference_type n) noexcept { cur = clamp(beg, fin, cur + n); }
            constexpr void set(pointer b, pointer e) noexcept { set(b, b + static_cast<difference_type>(cur - beg), e); }
            // constexpr void spike(volleyball_type vb) noexcept { lol jk }
        } in, out;
    public:
        virtual bool put(T const& t) { if(out.remaining()) { *(out.cur++) = t; return true; } return false; }
        virtual size_t put(T const* ts, size_t num) { size_t n = clamp(0UZ, out.remaining(), num); if(n) { __builtin_memcpy(out.cur, ts, n * sizeof(T)); out.bump(n); } return n; }
        virtual bool get(T& t) { if(in.remaining()) { t = *(in.cur++); return true; } return false; }
        virtual size_t get(T* ts, size_t num) { size_t n = clamp(0UZ, in.remaining(), num); if(n) { __builtin_memcpy(ts, in.cur, n * sizeof(T)); in.bump(n); } return n; }
        virtual bool poll() const { return out.remaining() > 0UZ; }
        virtual size_t poll(size_t num) const { return clamp(0UZ, out.remaining(), num); }
        virtual bool peek() const { return in.remaining() > 0UZ; }
        virtual bool peek(T& t) const { if(in.remaining()) { t = *(in.cur); return true; } return false; }
        virtual size_t peek(size_t num) const { return clamp(0UZ, in.remaining(), num); }
        virtual size_t peek(T* ts, size_t num) const { size_t n = clamp(0UZ, in.remaining(), num); if(n) __builtin_memcpy(ts, in.cur, n * sizeof(T)); return n; }
        virtual size_t pbump(difference_type n) { out.bump(n); return out.size(); }
        virtual size_t gbump(difference_type n) { in.bump(n); return in.size(); }
    };
}
/**
 * EXPORT_MODULE(T, Args...)
 * All modules must invoke this macro exactly once in order to build properly.
 * When invoked, it defines a function that the kernel will call when the module loads.
 * That function allocates a separate page frame structure which is used to implement most of the underlying "glue" needed by C++ to function fully.
 * It then also sets up some pointers to vtables which tie in kernel functionality modules might need to use.
 */
#define EXPORT_MODULE(module_class, ...) static char __instance[sizeof(module_class)]; extern "C" { ooos_kernel_module::abstract_module_base* module_init(ooos_kernel_module::kernel_api* api, kframe_tag** frame_tag, kframe_exports* kframe_fns, void (*init)(), void (*fini)()) { return ooos_kernel_module::setup_instance<module_class>(&__instance, api, frame_tag, kframe_fns, init, fini, __VA_ARGS__); } }
#endif