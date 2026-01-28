#ifndef __KMOD
#define __KMOD
#include <kernel_api.hpp>
#include <span>
#include <optional>
#include <unordered_map>
#include <errno.h>
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
	static char __instance[sizeof(module_class)] __align(alignof(module_class));																														\
	static ooos::cxxabi_abort __abort_handler;																															\
	namespace ooos																																						\
	{																																									\
		kernel_api* api_global;																																			\
		static module_class* __local_inst_##module_class##_ptr;																											\
		template<> constexpr module_class*& local_instance_ptr<module_class>() { return __local_inst_##module_class##_ptr; }											\
		ooos::abstract_module_base* module_instance() noexcept { return __local_inst_##module_class##_ptr; }															\
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
#define always_provide(type, ...) inline virtual bool export_provided_types() final override { return provide_types<type __VA_OPT__(, __VA_ARGS__)>(); }
#define always_require(type, ...) inline virtual bool import_required_types() final override { return require_types<type __VA_OPT__(, __VA_ARGS__)>(); }
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
	 * Uncaught exceptions in module code will use this method as well, but such unexpected errors will lead to a kernel abort by default.
	 * Module objects are singletons and should be designed as such.
	 * If the module class constructor takes arguments, those should be passed to the EXPORT_MODULE macro above.
	 * Global constructors will be invoked before the module's constructor, but by necessity the typeinfo fix-ups will not have occurred by then.
	 * Certain abstract subclasses within the kernel extend this base, allowing specific types of modules to more easily export functionality to the kernel.
	 * Modules can export RTTI to other modules through the kernel API as well. Exported types must be class, struct, or union types.
	 * A module cannot export a type that is already registered, e.g. by another module or the kernel proper.
	 * Modules may also import RTTI via the kernel API; if a type is exported by another module, that other module must be loaded first.
	 * The RTTI import/export system allows modules to use polymorphic types, dynamic_cast, and type_erasure.
	 * The system is WIP, but eventually the plan is to add a dependency list so that modules can declare other modules that must be loaded before them.
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
	protected:
		inline bool export_type(std::type_info const& ti) { return __api_hooks->export_type_info(ti); }
		inline bool import_type(std::type_info const& ti) { return __api_hooks->import_type_info(ti); }
		template<typename ... Ts> requires((std::is_class_v<Ts> || std::is_union_v<Ts>) && ...)
		bool provide_types() { return (export_type(typeid(Ts)) && ...); }
		template<typename ... Ts> requires((std::is_class_v<Ts> || std::is_union_v<Ts>) && ...)
		bool require_types() { return (import_type(typeid(Ts)) && ...); }
	public:
		virtual bool initialize() 	= 0;
		virtual void finalize() 	= 0;
		inline virtual bool export_provided_types() { return true; }
		inline virtual bool import_required_types() { return true; }
		inline virtual generic_config_table& get_config() { return empty_config; }
		inline abstract_module_base* tie_api_mm(kernel_api* api, kmod_mm* mm) { if(!__api_hooks && !__allocated_mm && api && mm) { __api_hooks = api; __allocated_mm = mm; } return this; }
		inline void* allocate_dma(size_t size, bool prefetchable) { return __api_hooks->allocate_dma(size, prefetchable); }
		inline void* map_dma(uintptr_t addr, size_t size, bool prefetchable) { return __api_hooks->map_dma(addr, size, prefetchable); }
		inline void* allocate_buffer(size_t size, size_t align) try { return __allocated_mm->mem_allocate(size, align); } catch(...) { this->raise_error("bad_alloc", -ENOMEM); }
		inline void* resize_buffer(void* orig, size_t old_size, size_t target_size, size_t align) try { return __allocated_mm->mem_resize(orig, old_size, target_size, align); } catch(...) { this->raise_error("bad_alloc", -ENOMEM); }
		inline void release_buffer(void* ptr, size_t align) { __allocated_mm->mem_release(ptr, align); }
		inline void release_dma(void* ptr, size_t size) { __api_hooks->release_dma(ptr, __api_hooks->dma_size(size)); }
		inline pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) { return __api_hooks->find_pci_device(device_class, subclass); }
		inline pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass, uint8_t prog_if) { return __api_hooks->find_pci_device(device_class, subclass, prog_if); }
		inline void* acpi_get_table(const char* label) { return __api_hooks->acpi_get_table(label); }
		inline uintptr_t vtranslate(void* ptr) noexcept { return __api_hooks->vtranslate(ptr); }
		[[noreturn]] inline void raise_error(const char* msg, int code = -1) { __api_hooks->ctx_raise(__eh_ctx, msg, code); __builtin_unreachable(); }
		inline uint32_t register_device(dev_stream<char>* stream, device_type type) { return __api_hooks->register_device(stream, type); }
		inline bool deregister_device(dev_stream<char>* stream) { return __api_hooks->deregister_device(stream); }
		inline void log(const char* msg) { __api_hooks->log(typeid(*this), msg); }
		friend void module_takedown(abstract_module_base* mod);
		inline void put_ctx(cxxabi_abort& abort_handler) { abort_handler.eh_ctx = std::addressof(__eh_ctx); }
		inline jmp_buf& ctx_jmp() { return __eh_ctx.handler_ctx; }
		inline const char* ctx_msg() { return __eh_ctx.msg; }
		inline int ctx_status() { return __eh_ctx.status; }
		inline void ctx_end() { __eh_ctx.handler_ctx[0] = __saved_jb[0]; }
		inline int load_config() { return __api_hooks->load_config(this); }
		inline int save_config() { return __api_hooks->save_config(this); }
		inline std::pair<std::optional<sysfstream>, int> sysfs_open(const char* name, bool create = true) { return __api_hooks->sysfs_open(name, create); }
		inline size_t asprintf(const char** strp, const char* fmt, ...);
		inline size_t logf(const char* fmt, ...);
		inline block_io_provider_module* as_blockdev();
		template<io_buffer_ok T> inline dev_stream<T>* as_device() { return dynamic_cast<dev_stream<T>*>(this); }
		template<wrappable_actor FT> inline void on_irq(uint8_t irq, FT&& handler) { __api_hooks->on_irq(irq, std::move(isr_actor(std::forward<FT>(handler), this->__allocated_mm)), this); }
		template<typename T> inline void release_array(T* arr) { this->release_buffer(arr, alignof(T)); }
		template<typename T, size_t A = alignof(T)>
		inline T* allocate_array(size_t num) { return array_zero(static_cast<T*>(allocate_buffer(num * sizeof(T), A)), num); }
		template<wrappable_actor FT>
		inline bool register_msi(msix_t volatile& reg, FT&& handler, msi_trigger_mode mode = msi_trigger_mode::EDGE) { return __api_hooks->extended_msi(this, isr_actor(std::forward<FT>(handler), this->__allocated_mm), reg, mode); }
		template<trivial_copy T, size_t A = alignof(T)>
		inline T* resize_array(T* orig, size_t old_size, size_t target_size) { return static_cast<T*>(resize_buffer(orig, old_size * sizeof(T), target_size * sizeof(T), A)); }
		template<nontrivial_copy T, size_t A = alignof(T)>
		inline T* resize_array(T* orig, size_t old_size, size_t target_size)
		{
			T* result	= allocate_array<T>(target_size);
			copy_or_move(result, orig, std::min(old_size, target_size));
			release_buffer(orig, A);
			return result;
		}
		inline void setup(kernel_api* api, kmod_mm* mm, void (*fini)())
		{
			if(api && mm && fini && !(__api_hooks || __allocated_mm || __fini_fn))
			{
				__api_hooks		= api;
				__allocated_mm	= mm;
				__fini_fn		= fini;
				__relocate_type_info();
				__save_init_jb();
			}
		}
		template<wrappable_actor ... FTs> requires(sizeof...(FTs) < 8)
		inline bool register_msi(msi32_t volatile& reg, FTs&& ... handlers)
		{
			typedef std::array<isr_actor, 7> msi_array;
			auto ctor = [&]<wrappable_actor FT>(FT&& f) { return isr_actor(std::forward<FT>(f), this->__allocated_mm); };
			return __api_hooks->msi(this, msi_array{ ctor(std::forward<FTs>(handlers))... }, reg, msi_trigger_mode::EDGE);
		}
		template<wrappable_actor ... FTs> requires(sizeof...(FTs) < 8)
		inline bool register_msi(msi32_t volatile& reg, msi_trigger_mode mode, FTs&& ... handlers)
		{
			typedef std::array<isr_actor, 7> msi_array;
			auto ctor = [&]<wrappable_actor FT>(FT&& f) { return isr_actor(std::forward<FT>(f), this->__allocated_mm); };
			return __api_hooks->msi(this, msi_array{ ctor(std::forward<FTs>(handlers))... }, reg, mode);
		}
		template<wrappable_actor ... FTs> requires(sizeof...(FTs) < 8)
		inline bool register_msi(msi64_t volatile& reg, FTs&& ... handlers)
		{
			typedef std::array<isr_actor, 7> msi_array;
			auto ctor = [&]<wrappable_actor FT>(FT&& f) { return isr_actor(std::forward<FT>(f), this->__allocated_mm); };
			return __api_hooks->msi(this, msi_array{ ctor(std::forward<FTs>(handlers))... }, reg, msi_trigger_mode::EDGE);
		}
		template<wrappable_actor ... FTs> requires(sizeof...(FTs) < 8)
		inline bool register_msi(msi64_t volatile& reg, msi_trigger_mode mode, FTs&& ... handlers)
		{
			typedef std::array<isr_actor, 7> msi_array;
			auto ctor = [&]<wrappable_actor FT>(FT&& f) { return isr_actor(std::forward<FT>(f), this->__allocated_mm); };
			return __api_hooks->msi(this, msi_array{ ctor(std::forward<FTs>(handlers))... }, reg, mode);
		}
	};
	template<typename T>
	class managed_dma_span : public std::span<T>
	{
		typedef std::span<T> __base;
		std::reference_wrapper<abstract_module_base> __mod;
		__base& __upcast() { return *static_cast<__base*>(this); }
		void __reset() noexcept { __upcast() = __base(); }
		static T* __allocate(abstract_module_base& mod, size_t n, bool prefetch) { return array_zero<T>(addr_t(mod.allocate_dma(n * sizeof(T), prefetch)), n); }
		managed_dma_span& __set(T* ptr, size_t n) noexcept
		{
			destroy();
			__upcast()	= __base(ptr, n);
			return *this;
		}
	public:
		managed_dma_span(abstract_module_base& mod) noexcept : __base(), __mod(std::ref(mod)) {}
		managed_dma_span(abstract_module_base& mod, size_t n, bool prefetch) : __base(__allocate(mod, n, prefetch), n), __mod(std::ref(mod)) {}
		managed_dma_span(managed_dma_span&& that) noexcept : __base(std::backward<__base>(that)), __mod(that.__mod) { that.__reset(); }
		~managed_dma_span() { this->destroy(); }
		managed_dma_span& create(size_t n, bool prefetch) { return __set(__allocate(__mod, n, prefetch), n); }
		void destroy()
		{
			if(__base::data())
				__mod.get().release_dma(__base::data(), __base::size_bytes());
			__reset();
		}
		managed_dma_span& operator=(managed_dma_span&& that) noexcept
		{
			this->destroy();
			this->__upcast()	= that.__upcast();
			that.__reset();
			return *this;
		}
	};
	template<typename T>
	struct module_mm_allocator
	{
		typedef T value_type;
		typedef T* pointer;
		typedef typename std::pointer_traits<pointer>::rebind<const value_type> const_pointer;
		typedef typename std::pointer_traits<pointer>::rebind<void> void_pointer;
		typedef typename std::pointer_traits<pointer>::rebind<const void> const_void_pointer;
		typedef std::true_type propagate_on_container_move_assignment;
		typedef std::true_type propagate_on_container_copy_assignment;
		typedef std::true_type propagate_on_container_swap;
		typedef decltype(sizeof(value_type)) size_type;
		typedef decltype(alignof(value_type)) align_type;
		typedef decltype(std::declval<pointer>() - std::declval<pointer>()) difference_type;
		template<typename U> struct rebind { typedef module_mm_allocator<U> other; };
	private:
		constexpr static size_type __size_val				= sizeof(value_type);
		constexpr static align_type __align_val				= alignof(value_type);
		constexpr static std::align_val_t __std_align_val	= static_cast<std::align_val_t>(__align_val);
		abstract_module_base* __opt_module;
		[[gnu::always_inline]] inline void_pointer __allocate_n(size_type n) const
		{
			if(__unlikely(!n)) return nullptr;
			if(__opt_module) return __opt_module->allocate_array<value_type>(n);
			else return operator new(__size_val, __std_align_val);
		}
		[[gnu::always_inline]] inline void __deallocate(pointer p, size_type n) const
		{
			if(__unlikely(!n || !p)) return;
			if(__opt_module) __opt_module->release_array(p);
			else operator delete(p, n * __size_val, __std_align_val);
		}
		[[gnu::always_inline]] inline pointer __allocate(size_type n) const
		{
			pointer result	= static_cast<pointer>(this->__allocate_n(n));
			if(result) array_zero(result, n);
			return result;
		}
	public:
		constexpr module_mm_allocator() noexcept = default;
		constexpr ~module_mm_allocator() noexcept = default;
		constexpr module_mm_allocator(abstract_module_base* mod) noexcept : __opt_module(mod) {}
		constexpr module_mm_allocator(module_mm_allocator const&) noexcept = default;
		constexpr module_mm_allocator(module_mm_allocator&&) noexcept = default;
		constexpr module_mm_allocator& operator=(module_mm_allocator const&) = default;
		constexpr module_mm_allocator& operator=(module_mm_allocator&&) = default;
		template<typename U> constexpr module_mm_allocator(module_mm_allocator<U> const& that) noexcept : __opt_module(that.__opt_module) {}
		[[nodiscard]] [[gnu::always_inline]] inline pointer allocate(size_type n) const { return this->__allocate(n); }
		[[gnu::always_inline]] inline void deallocate(pointer p, size_type n) const { this->__deallocate(p, n); }
	};
	template<typename T> using mod_mm_vec = std::vector<T, module_mm_allocator<T>>;
	template<typename KT, typename MT, typename HT = std::hash<KT>, typename ET = std::equal_to<void>> using mod_mm_map = std::unordered_map<KT, MT, HT, ET, module_mm_allocator<std::pair<const KT, MT>>>;
	template<std::derived_from<abstract_module_base> MT> constexpr MT*& local_instance_ptr();
	template<std::derived_from<abstract_module_base> MT> constexpr MT& instance() { return *local_instance_ptr<MT>(); }
	struct block_io_provider_module : abstract_module_base, abstract_block_device::provider{};
	struct abstract_hub_module_base : abstract_module_base, abstract_connectable_device::provider{};
	template<trivial_copy BMT, std::derived_from<BMT> ... SMTs> requires(std::is_class_v<BMT> || std::is_union_v<BMT>)
	struct abstract_hub_module : abstract_hub_module_base { virtual bool export_provided_types() final override { return provide_types<BMT, SMTs...>(); } };
	inline block_io_provider_module* abstract_module_base::as_blockdev() { return dynamic_cast<block_io_provider_module*>(this); }
	inline size_t abstract_module_base::asprintf(const char** strp, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		size_t result	= __api_hooks->vformat(__allocated_mm, fmt, *strp, args);
		va_end(args);
		return result;
	}
	void module_takedown(abstract_module_base* mod);
	template<std::derived_from<abstract_module_base> T, typename ... Args> requires(std::constructible_from<T, Args...>)
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
				if(__unlikely(!result->import_required_types() || !result->export_provided_types())) return nullptr;
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
		va_start(args, fmt);
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
		virtual size_type tellg() const override { return out.size(); }
		virtual uint32_t get_device_id() const noexcept override { return device_id; }
		void create_buffer(io_buffer& b, size_type n) { b.set(this->allocate_array<value_type>(n), n); }
		void destroy_buffer(io_buffer& b) { this->release_array(b.beg); b.reset(); }
		void resize_buffer(io_buffer& b, size_type nsz) { b.set(this->resize_array(b.beg, b.capacity(), nsz), nsz); }
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
		if(__unlikely(!out.remaining() && !overflow(static_cast<size_type>(1UZ)))) return false;
		out.put(t);
		return true;
	}
	template<io_buffer_ok T>
	typename io_module_base<T>::size_type io_module_base<T>::seek(int direction, difference_type where, uint8_t ioflags)
	{
		if(ioflags & 0x04UC)
			out.seek(direction, where);
		if(ioflags & 0x08UC)
			in.seek(direction, where);
		return out.size() * (ioflags & 0x04UC ? 1UZ : 0UZ) + in.size() * (ioflags & 0x08UC ? 1UZ : 0UZ);
	}
	template<io_buffer_ok T>
	typename io_module_base<T>::size_type io_module_base<T>::seek(size_type where, uint8_t ioflags)
	{
		if(ioflags & 0x04UC)
			out.seek(where);
		if(ioflags & 0x08UC)
			in.seek(where);
		return out.size() * (ioflags & 0x04UC ? 1UZ : 0UZ) + in.size() * (ioflags & 0x08UC ? 1UZ : 0UZ);
	}
}
#endif