#ifndef __KMOD
#define __KMOD
#include <kernel_api.hpp>
/**
 * EXPORT_MODULE(T, Args...)
 * All modules must invoke this macro exactly once in order to build properly.
 * When invoked, it defines a function that the kernel will call when the module loads.
 * That function allocates a separate page frame structure which is used to implement most of the underlying "glue" needed by C++ to function fully.
 * It then also sets up some pointers which tie in kernel functionality modules might need to use.
 * This includes a patch to the typeinfo struct representing the module's class, which points the base-type-info pointers at the kernel symbol representing each of its bases if one exists.
 * The process occurs recursively in case a module class inherits from a fully-inline base or one that is itself defined within the module, making the change to the first matching pointer it finds within each inheritance chain (but no others).
 * Doing so allows the kernel to use dynamic_cast to upcast the module class into something more specific, e.g. block_io_provider_module or io_module_base<char>.
 */
#define EXPORT_MODULE(module_class, ...)																																\
	static char __instance[sizeof(module_class)];																														\
	static ooos::cxxabi_abort __abort_handler;																															\
	namespace ooos																																						\
	{																																									\
		kernel_api* api_global;																																			\
		static module_class* __local_inst_##module_class##_ptr;																										\
		template<> constexpr module_class*& local_instance_ptr<module_class>() { return __local_inst_##module_class##_ptr; }											\
		ooos::abstract_module_base* module_instance() noexcept { return __local_inst_##module_class##_ptr; }																						\
	}																																									\
	extern "C"																																							\
	{																																									\
		ooos::abstract_module_base* module_init(ooos::kernel_api* api, kframe_tag** frame_tag, kframe_exports* kframe_fns, void (*init)(), void (*fini)())				\
		{																																								\
			__abort_handler.abort_msg 	= "abort() called in module " # module_class;																					\
			__abort_handler.abort_msg 	= "pure virtual call in module " # module_class;																				\
			__abort_handler.api 		= api;																															\
			ooos::api_global			= api;																															\
			return ooos::setup_instance<module_class>(std::addressof(__instance), api, frame_tag, kframe_fns, init, fini, __abort_handler __VA_OPT__(, __VA_ARGS__));	\
		}																																								\
		[[noreturn]] void abort() { __abort_handler.terminate(); }																										\
		[[noreturn]] void __cxa_pure_virtual() { __abort_handler.pure_virt(); }																							\
	}
#pragma endregion
class elf64_kernel_object;
namespace ooos
{
	struct block_io_provider_module;
	extern kernel_api* api_global;
	extern ooos::abstract_module_base* module_instance() noexcept;
	/**
	 * Base class for all module objects.
	 * This is an abstract class which hooks in various parts of the kernel that module code might need to use.
	 * The two pure virtual methods, initialize and finalize, are invoked by the kernel when the module is loaded and unloaded, respectively.
	 * The former will be called after the constructor for the module's class returns, and the latter will be called before any destructors in the module file.
	 * If a module encounters an error that needs to be handled by the kernel (e.g. by unloading the module), the raise_error method should be used.
	 * This can pass an error code and an error message to the kernel handlers.
	 * Uncaught exceptions in module code will use this method as well, but such unexpected errors will lead to a kernel abort.
	 * Module objects are singletons and should be designed as such.
	 * If the module class constructor takes arguments, those should be passed to the EXPORT_MODULE macro above.
	 * Global constructors will be invoked before the module's constructor, but by necessity the typeinfo fix-ups will not have occurred by then.
	 * Certain abstract subclasses within the kernel extend this base, allowing specific types of modules to more easily export functionality to the kernel.
	 */
	class abstract_module_base
	{
		kernel_api* __api_hooks;
		kmod_mm* __allocated_mm;
		void (*__fini_fn)();
		jmp_buf __saved_jb{};
		module_eh_ctx __eh_ctx{};
		friend class ::elf64_kernel_object;
		inline void __relocate_type_info() { __api_hooks->relocate_type_info(this, std::addressof(typeid(ABI_NAMESPACE::__si_class_type_info)), std::addressof(typeid(ABI_NAMESPACE::__vmi_class_type_info))); }
		inline void __save_init_jb() { __saved_jb[0] = __eh_ctx.handler_ctx[0]; }
	public:
		virtual bool initialize() 	= 0;
		virtual void finalize() 	= 0;
		inline virtual generic_config_table& get_config() { return empty_config; }
		inline abstract_module_base* tie_api_mm(kernel_api* api, kmod_mm* mm) { if(!__api_hooks && !__allocated_mm && api && mm) { __api_hooks = api; __allocated_mm = mm; } return this; }
		inline void* allocate_dma(size_t size, bool prefetchable) { return __api_hooks->allocate_dma(size, prefetchable); }
		inline void* allocate_buffer(size_t size, size_t align) { return __allocated_mm->mem_allocate(size, align); }
		inline void* resize_buffer(void* orig, size_t old_size, size_t target_size, size_t align) { return __allocated_mm->mem_resize(orig, old_size, target_size, align); }
		inline void release_buffer(void* ptr, size_t align) { __allocated_mm->mem_release(ptr, align); }
		inline void release_dma(void* ptr, size_t size) { __api_hooks->release_dma(ptr, size); }
		inline pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) { return __api_hooks->find_pci_device(device_class, subclass); }
		inline void* acpi_get_table(const char* label) { return __api_hooks->acpi_get_table(label); }
		inline uintptr_t vtranslate(void* ptr) noexcept { return __api_hooks->vtranslate(ptr); }
		[[noreturn]] inline void raise_error(const char* msg, int code = -1) { __api_hooks->ctx_raise(__eh_ctx, msg, code); __builtin_unreachable(); }
		inline uint32_t register_device(dev_stream<char>* stream, device_type type) { return __api_hooks->register_device(stream, type); }
		inline bool deregister_device(dev_stream<char>* stream) { return __api_hooks->deregister_device(stream); }
		inline void log(const char* msg) { __api_hooks->log(typeid(*this), msg); }
		template<io_buffer_ok T> inline dev_stream<T>* as_device() { return dynamic_cast<dev_stream<T>*>(this); }
		template<wrappable_actor FT> inline void on_irq(uint8_t irq, FT&& handler) { isr_actor actor(std::forward<FT>(handler), this->__allocated_mm); __api_hooks->on_irq(irq, std::forward<isr_actor>(actor), this); }
		inline void setup(kernel_api* api, kmod_mm* mm, void (*fini)()) { if(api && mm && fini && !(__api_hooks || __allocated_mm || __fini_fn)) { __api_hooks = api; __allocated_mm = mm; __fini_fn = fini; __relocate_type_info(); __save_init_jb(); } }
		friend void module_takedown(abstract_module_base* mod);
		inline void put_ctx(cxxabi_abort& abort_handler) { abort_handler.eh_ctx = std::addressof(__eh_ctx); }
		inline jmp_buf& ctx_jmp() { return __eh_ctx.handler_ctx; }
		inline const char* ctx_msg() { return __eh_ctx.msg; }
		inline int ctx_status() { return __eh_ctx.status; }
		inline void ctx_end() { __eh_ctx.handler_ctx[0] = __saved_jb[0]; }
		inline size_t asprintf(const char** strp, const char* fmt, ...);
		inline size_t logf(const char* fmt, ...);
		inline block_io_provider_module* as_blockdev();
	};
	template<std::derived_from<abstract_module_base> MT> constexpr MT*& local_instance_ptr();
	template<std::derived_from<abstract_module_base> MT> constexpr MT& instance() { return *local_instance_ptr<MT>(); }
	struct block_io_provider_module : abstract_module_base, abstract_block_device::provider{};
	inline block_io_provider_module* abstract_module_base::as_blockdev() { return dynamic_cast<block_io_provider_module*>(this); }
	inline size_t abstract_module_base::asprintf(const char** strp, const char* fmt, ...)
	{
		va_list args;
		va_start(args, 0);
		size_t result	= __api_hooks->vformat(__allocated_mm, fmt, *strp, args);
		va_end(args);
		return result;
	}
	void module_takedown(abstract_module_base* mod);
	template<typename T, typename ... Args>
	abstract_module_base* setup_instance(void* addr, kernel_api* api, kframe_tag** frame_tag, kframe_exports* kframe_fns, void (*init)(), void (*fini)(), cxxabi_abort& abort_handler, Args&& ... args)
	{
		if(addr && api && frame_tag && kframe_fns && fini)
		{
			if(kmod_mm* mm 				= api->create_mm())
			{
				*frame_tag				= mm->get_frame();
				api->init_memory_fns(kframe_fns);
				(*init)();
				T* result 				= new(addr) T(std::forward<Args>(args)...);
				result->setup(api, mm, fini);
				result->put_ctx(abort_handler);
				local_instance_ptr<T>()	= result;
				return result;
			}
		}
		return nullptr;
	}
	inline size_t abstract_module_base::logf(const char* fmt, ...)
	{
		va_list args;
		va_start(args, 0);
		size_t result = __api_hooks->vlogf(typeid(*this), fmt, args);
		va_end(args);
		return result;
	}
	struct eh_exit_guard
	{
		abstract_module_base* mod;
		constexpr void release() noexcept { mod = nullptr; }
		constexpr ~eh_exit_guard() noexcept { mod->ctx_end(); }
	};
	template<io_buffer_ok T>
	struct io_module_base : public abstract_module_base, public dev_stream<T>
	{
		typedef std::remove_cvref_t<T> value_type;
		typedef in_value<value_type> input_type;
		typedef out_value<value_type> output_type;
		typedef value_type* pointer;
		typedef value_type const* const_pointer;
		typedef value_type& reference;
		typedef value_type const& const_reference;
		typedef simple_iterator<pointer> iterator;
		typedef simple_iterator<const_pointer> const_iterator;
		typedef typename iterator::difference_type difference_type;
		typedef decltype(sizeof(value_type)) size_type;
		struct io_buffer
		{
			pointer beg;
			pointer cur;
			pointer fin;
			constexpr size_type size() const noexcept { return cur > beg ? static_cast<size_type>(cur - beg) : 0UZ; }
			constexpr size_type capacity() const noexcept { return fin > beg ? static_cast<size_type>(fin - beg) : 0UZ; }
			constexpr size_type remaining() const noexcept { return (beg && fin > cur) ? static_cast<size_type>(fin - cur) : 0UZ; }
			constexpr void set(pointer b, pointer c, pointer e) noexcept { beg = b; cur = c; fin = e; }
			constexpr void rewind() noexcept { cur = beg; }
			constexpr void reset() noexcept { beg = cur = fin = pointer(); }
			constexpr io_buffer() noexcept = default;
			constexpr ~io_buffer() noexcept = default;
			constexpr io_buffer(pointer b, pointer c, pointer e) noexcept : beg(b), cur(c), fin(e) {}
			constexpr io_buffer(pointer b, pointer e) noexcept : beg(b), cur(b), fin(e) {}
			constexpr io_buffer(pointer b, size_type n) noexcept : beg(b), cur(b), fin(b + n) {}
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
			constexpr void bump(difference_type n = static_cast<difference_type>(1)) noexcept { pointer target = cur + n; cur = target < fin ? (target > beg ? target : beg) : fin; }
			constexpr void set(pointer b, pointer e) noexcept { set(b, b + static_cast<difference_type>(size()), e); /* constexpr void spike(volleyball_t loljk); */ }
			constexpr void set(pointer b, size_type n) noexcept { set(b, b + n); }
			constexpr void seek(int direction, difference_type where) { pointer base = direction < 0 ? beg : direction > 0 ? fin : cur; pointer target = base + where; cur = target < fin ? (target > beg ? target : beg) : fin; }
			constexpr void seek(size_type where) { bump(where - size()); }
			constexpr output_type read() const noexcept { return *cur; }
			constexpr output_type get() noexcept { output_type result = read(); bump(); return result; }
			constexpr void write(input_type t) noexcept { *cur = t; }
			constexpr void put(input_type t) noexcept { write(t); bump(); }
		} in, out;
		uint32_t device_id;
		virtual bool overflow(size_type needed) { return out.remaining() >= needed; }
		virtual bool underflow(size_type needed) { return in.remaining() >= needed; }
		virtual int sync() override { return 0; }
		virtual size_type read(pointer dest, size_type n) override;
		virtual size_type write(size_type n, const_pointer src) override;
		virtual size_type seek(int direction, difference_type where, uint8_t ioflags) override;
		virtual size_type seek(size_type where, uint8_t ioflags) override;
		virtual size_type avail() const override { return in.remaining(); }
		virtual size_type out_avail() const override { return out.remaining(); }
		virtual uint32_t get_device_id() const noexcept override { return device_id; }
		void create_buffer(io_buffer& b, size_type n) { b.set(new (this->allocate_buffer(n * sizeof(value_type), alignof(value_type))) value_type[n], n); }
		void destroy_buffer(io_buffer& b) { this->release_buffer(b.beg, alignof(value_type)); b.reset(); }
		void resize_buffer(io_buffer& b, size_type nsz) { b.set(static_cast<pointer>(abstract_module_base::resize_buffer(b.beg, b.capacity(), nsz, alignof(value_type))), nsz); }
		value_type get();
		bool put(input_type t);
	};
	template<io_buffer_ok T>
	typename io_module_base<T>::size_type io_module_base<T>::read(pointer dest, size_type n)
	{
		size_type result{};
		for(size_type i = size_type{}; i < n; i++)
		{
			if(__unlikely(!in.remaining() && !underflow(n - i))) break;
			dest[i] = in.get();
			result++;
		}
		return result;
	}
	template<io_buffer_ok T>
	typename io_module_base<T>::size_type io_module_base<T>::write(size_type n, const_pointer src)
	{
		size_type result{};
		for(size_type i = size_type{}; i < n; i++)
		{
			if(__unlikely(!out.remaining() && !underflow(n - i))) break;
			out.put(src[i]);
			result++;
		}
		return result;
	}
	template<io_buffer_ok T>
	typename io_module_base<T>::value_type io_module_base<T>::get()
	{
		if(in.remaining()) return in.get();
		else if(underflow(static_cast<size_type>(1))) return in.get();
		else return value_type();
	}
	template<io_buffer_ok T>
	bool io_module_base<T>::put(input_type t)
	{
		if(__unlikely(!out.remaining() && !overflow(static_cast<size_type>(1)))) return false;
		out.put(t);
		return true;
	}
	template<io_buffer_ok T>
	typename io_module_base<T>::size_type io_module_base<T>::seek(int direction, difference_type where, uint8_t ioflags)
	{
		if(ioflags & 0x04)
			out.seek(direction, where);
		if(ioflags & 0x08)
			in.seek(direction, where);
		return out.size() * (ioflags & 0x04 ? 1 : 0) + in.size() * (ioflags & 0x08 ? 1 : 0);
	}
	template<io_buffer_ok T>
	typename io_module_base<T>::size_type io_module_base<T>::seek(size_type where, uint8_t ioflags)
	{
		if(ioflags & 0x04)
			out.seek(where);
		if(ioflags & 0x08)
			in.seek(where);
		return out.size() * (ioflags & 0x04 ? 1 : 0) + in.size() * (ioflags & 0x08 ? 1 : 0);
	}
}
#endif