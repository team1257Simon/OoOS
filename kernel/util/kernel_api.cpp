#include "module.hpp"
#include "kernel_mm.hpp"
#include "stdlib.h"
#include "arch/pci_device_list.hpp"
#include "device_registry.hpp"
#include "isr_table.hpp"
#include "bits/hash_set.hpp"
#include "stdexcept"
extern "C" size_t kvasprintf(char** restrict strp, const char* restrict fmt, va_list args);
using namespace ABI_NAMESPACE;
struct vtable_header
{
	/** Offset of the leaf object. */
	ptrdiff_t			leaf_offset;
	/** Type of the object. */
	__class_type_info*	type;
};
static __si_class_type_info* meta_dyncast_si(std::type_info* ti, __class_type_info const* local_si)
{
	vtable_header* vt			= addr_t(ti).deref<addr_t>().minus(sizeof(vtable_header));
	__class_type_info* ti_meta	= vt->type;
	return static_cast<__si_class_type_info*>(ti_meta->cast_to(addr_t(ti).plus(vt->leaf_offset), local_si));
}
static __vmi_class_type_info* meta_dyncast_vmi(std::type_info* ti, __class_type_info const* local_vmi)
{
	vtable_header* vt			= addr_t(ti).deref<addr_t>().minus(sizeof(vtable_header));
	__class_type_info* ti_meta	= vt->type;
	return static_cast<__vmi_class_type_info*>(ti_meta->cast_to(addr_t(ti).plus(vt->leaf_offset), local_vmi));
}
namespace ooos
{
	template<typename T> struct type_tag { constexpr explicit type_tag() = default; };
	template<typename T> constexpr inline type_tag<T> tag{};
	struct get_name { constexpr const char* const& operator()(std::type_info const* const& ti) const noexcept { return ti->__type_name; } };
	class type_info_map : public std::hash_set<std::type_info const*, const char*, std::elf64_gnu_hash, std::ext::lexcial_equals<char>, std::allocator<std::type_info const*>, get_name> 
	{
		typedef std::hash_set<std::type_info const*, const char*, std::elf64_gnu_hash, std::ext::lexcial_equals<char>, std::allocator<std::type_info const*>, get_name> __base;
	public:
		type_info_map() : __base(64UZ) {}
		__class_type_info const* operator[](const char* key) const
		{
			const_iterator result = find(key);
			if(result != end()) return dynamic_cast<__class_type_info const*>(*result);
			return nullptr;
		}
	};
	struct kmod_mm_impl : kmod_mm, kframe_tag
	{
		kernel_memory_mgr* mm;
		block_tag* first_managed_block;
		spinlock_t mod_mutex{};
		virtual void* mem_allocate(size_t size, size_t align) override;
		virtual void mem_release(void* block, size_t align) override;
		virtual void* mem_resize(void* old, size_t old_size, size_t target, size_t align) override;
		virtual kframe_tag* get_frame() override { return this; }
		virtual ~kmod_mm_impl();
		kmod_mm_impl();
	};
	static struct : kernel_api
	{
		kernel_memory_mgr* mm = std::addressof(kmm);
		type_info_map kernel_type_info;
		pci_device_list* pci;
		virtual void* allocate_dma(size_t size, bool prefetchable) override { return mm->allocate_dma(size, prefetchable); }
		virtual void release_dma(void* ptr, size_t size) override { mm->deallocate_dma(ptr, size); }
		virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) override { return pci->find(device_class, subclass); }
		virtual void* acpi_get_table(const char* label) override { return find_system_table(label); }
		virtual kmod_mm* create_mm() override { return new kmod_mm_impl(); }
		virtual void destroy_mm(kmod_mm* mod_mm) override { if(mod_mm) delete mod_mm; }
		virtual void log(std::type_info const& from, const char* message) override { xklog("[" + std::ext::demangle(from) + "]: " + message); }
		virtual void remove_actors(abstract_module_base* owner) override { interrupt_table::deregister_owner(owner); }
		virtual void on_irq(uint8_t irq, isr_actor&& handler, abstract_module_base* owner) override { interrupt_table::add_irq_handler(owner, irq, std::forward<isr_actor>(handler)); }
		virtual uint32_t register_device(dev_stream<char>* stream, device_type type) override { return dreg.add(stream, type); }
		virtual bool deregister_device(dev_stream<char>* stream) override { return dreg.remove(stream); }
		virtual void register_type_info(std::type_info const* ti) override { kernel_type_info.insert(ti); }
		virtual void relocate_type_info(abstract_module_base* mod, std::type_info const* local_si, std::type_info const* local_vmi) override { this->__relocate_ti_r(const_cast<std::type_info*>(std::addressof(typeid(*mod))), local_si, local_vmi); }
		virtual size_t vformat(kmod_mm* mm, const char* fmt, const char*& out, va_list args) override
		{
			char* result	= nullptr;
			size_t count	= kvasprintf(std::addressof(result), fmt, args);
			if(!count) return 0UZ;
			char* str		= static_cast<char*>(mm->mem_allocate(count, alignof(char)));
			array_copy(str, result, count);
			free(result);
			out			 	= str;
			return count;
		}
		virtual size_t vlogf(std::type_info const& from, const char* fmt, va_list args)
		{
			char* result	= nullptr;
			size_t count	= kvasprintf(std::addressof(result), fmt, args);
			if(!count) return 0UZ;
			xklog("[" + std::ext::demangle(from) + "]: " + result);
			free(result);
			return count;
		}
		[[noreturn]] virtual void ctx_raise(module_eh_ctx& ctx, const char* msg, int status) override
		{
			ctx.msg			= msg;
			ctx.status		 = status ? status : -1;
			longjmp(ctx.handler_ctx, ctx.status);
		}
		virtual void init_memory_fns(kframe_exports* ptrs) override
		{
			new(ptrs) kframe_exports
			{
				.allocate		= &kframe_tag::allocate,
				.array_allocate	= &kframe_tag::array_allocate,
				.deallocate	 	= &kframe_tag::deallocate,
				.reallocate		= &kframe_tag::reallocate
			};
		}
		void __relocate_si_r(__si_class_type_info* ti, std::type_info const* local_si, std::type_info const* local_vmi)
		{
			__class_type_info* base			= const_cast<__class_type_info*>(ti->__base_type);
			__class_type_info const* equiv	= kernel_type_info[base->__type_name];
			if(!equiv) this->__relocate_ti_r(base, local_si, local_vmi);
			else ti->__base_type			= equiv;
		}
		void __relocate_vmi_r(__base_class_type_info* bases, size_t num_bases, std::type_info const* local_si, std::type_info const* local_vmi)
		{
			for(size_t i = 0; i < num_bases; i++)
			{
				__class_type_info* base			= const_cast<__class_type_info*>(bases[i].__base_type);
				__class_type_info const* equiv	= kernel_type_info[base->__type_name];
				if(!equiv) this->__relocate_ti_r(base, local_si, local_vmi);
				else bases[i].__base_type		= equiv;
			}
		}
		void __relocate_ti_r(std::type_info* ti, std::type_info const* local_si, std::type_info const* local_vmi)
		{
			if(__si_class_type_info* si			= meta_dyncast_si(ti, addr_t(local_si)))
				__relocate_si_r(si, local_si, local_vmi);
			else if(__vmi_class_type_info* vmi	= meta_dyncast_vmi(ti, addr_t(local_vmi)))
				__relocate_vmi_r(vmi->__base_info, vmi->__base_count, local_si, local_vmi);
		}
	} __api_impl{};
	void register_type(std::type_info const& ti) { __api_impl.register_type_info(std::addressof(ti)); }
	kmod_mm_impl::kmod_mm_impl() : kmod_mm(), kframe_tag(), mm(__api_impl.mm), first_managed_block(nullptr) {}
	void* kmod_mm_impl::mem_allocate(size_t size, size_t align)
	{
		block_tag* tag = get_for_allocation(size ? size : 1UL, align);
		if(!tag) throw std::bad_alloc();
		lock(std::addressof(mod_mutex));
		if(!first_managed_block) first_managed_block = tag;
		else
		{
			tag->next						= first_managed_block;
			first_managed_block->previous	= tag;
			first_managed_block				= tag;
		}
		release(std::addressof(mod_mutex));
		return tag->actual_start();
	}
	void kmod_mm_impl::mem_release(void* block, size_t align)
	{
		if(__unlikely(!block)) return;
		if(block_tag* tag = find_tag(block, align))
		{
			lock(std::addressof(mod_mutex));
			if(tag->previous) tag->previous->next				= tag->next;
			if(tag->next) tag->next->previous					= tag->previous;
			if(first_managed_block == tag) first_managed_block	= tag->next;
			tag->previous										= nullptr;
			tag->next											= nullptr;
			release_block(tag);
			release(std::addressof(mod_mutex));
		}
	}
	void* kmod_mm_impl::mem_resize(void* old, size_t old_size, size_t target, size_t align)
	{
		void* result		= mem_allocate(target, align);
		if(!old) return result;
		array_copy<char>(static_cast<char*>(result), static_cast<char*>(old), old_size < target ? old_size : target);
		mem_release(old, align);
		return result;
	}
	kmod_mm_impl::~kmod_mm_impl()
	{
		block_tag* tag		= first_managed_block;
		while(tag)
		{
			block_tag* next	 = tag->next;
			release_block(tag);
			tag				= next;
		}
	}
	void init_api()
	{
		__api_impl.pci		= pci_device_list::get_instance();
		register_type(typeid(abstract_module_base));
		register_type(typeid(device_stream));
		register_type(typeid(io_module_base<char>));
		register_type(typeid(abstract_block_device));
		register_type(typeid(abstract_block_device::provider));
		register_type(typeid(block_io_provider_module));
	}
	kernel_api* get_api_instance() { if(__unlikely(!__api_impl.pci || !__api_impl.mm)) return nullptr; else return std::addressof(__api_impl); }
}