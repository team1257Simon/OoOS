#include <arch/pci_device_list.hpp>
#include <bits/hash_set.hpp>
#include <ext/delegate_ptr.hpp>
#include <fs/sysfs.hpp>
#include <net/netdev_module.hpp>
#include <sched/scheduler.hpp>
#include <device_registry.hpp>
#include <isr_table.hpp>
#include <kernel_mm.hpp>
#include <stdexcept>
#include <stdlib.h>
extern "C" size_t kvasprintf(char** restrict strp, const char* restrict fmt, va_list args);
using namespace ABI_NAMESPACE;
namespace ooos
{
	pid_t active_pid() noexcept { return scheduler::active_pid(); }
	template<typename T> struct type_tag { constexpr explicit type_tag() = default; };
	template<typename T> constexpr inline type_tag<T> tag{};
	struct get_name { constexpr const char* const& operator()(std::type_info const* const& ti) const noexcept { return ti->__type_name; } };
	class type_info_map : public std::hash_set<std::type_info const*, const char*, std::elf64_gnu_hash, std::ext::lexical_equals<char>, std::allocator<std::type_info const*>, get_name>
	{
		typedef std::hash_set<std::type_info const*, const char*, std::elf64_gnu_hash, std::ext::lexical_equals<char>, std::allocator<std::type_info const*>, get_name> __base;
	public:
		type_info_map() : __base(64UZ) {}
		std::type_info const* get_generic(const char* key) const
		{
			const_iterator result = find(key);
			if(result != end()) return *result;
			return nullptr;
		}
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
		std::vector<block_tag*> managed_blocks{};
		spinlock_t mod_mutex{};
		virtual void* mem_allocate(size_t size, size_t align) override;
		virtual void mem_release(void* block, size_t align) override;
		virtual void* mem_resize(void* old, size_t old_size, size_t target, size_t align) override;
		virtual kframe_tag* get_frame() override { return this; }
		virtual ~kmod_mm_impl();
		kmod_mm_impl();
	};
	struct netdev_api_helper_impl : public netdev_api_helper
	{
		abstract_netdev_module& mod;
		std::ext::resettable_queue<netstack_buffer> transfer_buffers;
		virtual int rx_transfer(netstack_buffer& b) noexcept override;
		virtual int transmit(abstract_packet_base& p) override;
		virtual bool construct_transfer_buffers() override;
		virtual protocol_arp& get_arp() noexcept override { return arp_handler; }
		virtual protocol_ethernet& get_ethernet() noexcept override { return ethernet_handler; }
		virtual std::ext::resettable_queue<netstack_buffer>& get_transfer_buffers() noexcept override { return transfer_buffers; }
		virtual protocol_handler& add_protocol(net16 id, protocol_handler&& ph) override { return ethernet_handler.handlers.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(std::move(ph))).first->second; }
		virtual ~netdev_api_helper_impl() = default;
		netdev_api_helper_impl(abstract_netdev_module& dev) : netdev_api_helper(dev.get_mac_addr()), mod(dev) {}
	};
	bool netdev_api_helper_impl::construct_transfer_buffers()
	{
		try
		{
			size_t count	= mod.buffer_count();
			size_t rx_sz	= mod.rx_limit();
			size_t tx_sz	= mod.tx_limit();
			for(size_t i	= 0UZ; i < count; i++) transfer_buffers.emplace(rx_sz, tx_sz, std::bind_front(&netdev_helper::rx_transfer, this), std::bind_front(&abstract_netdev_module::poll_tx, std::addressof(mod)), rx_sz, tx_sz);
			return true;
		}
		catch(...) { return false; }
	}
	int netdev_api_helper_impl::transmit(abstract_packet_base& p)
	{
		if(transfer_buffers.at_end()) transfer_buffers.restart();
		netstack_buffer& buffer	= transfer_buffers.pop();
		int err					= p.write_to(buffer);
		if(__unlikely(err != 0)) return err;
		return buffer.tx_flush();
	}
	int netdev_api_helper_impl::rx_transfer(netstack_buffer& b) noexcept
	{
		try
		{
			abstract_packet<ethernet_header> p(b);
			if(p->protocol_type == ethertype_arp)
			{
				p.packet_type	= arp_handler.packet_type();
				if(int err		= arp_handler.receive(p); __unlikely(err != 0)) return err;
				return 0;
			}
			int result			= ethernet_handler.receive(p);
			if(!result) return 0;
			if(result == -EPROTONOSUPPORT) { mod.logf("W: unrecognized protocol"); return 0; }
			return result;
		}
		catch(std::bad_alloc&)		{ return -ENOMEM; }
		catch(std::bad_cast&)		{ return -EPROTO; }
		catch(std::exception& e)	{ mod.logf(e.what()); return -EINVAL; }
	}
	static struct : kernel_api
	{
		kernel_memory_mgr* mm						= std::addressof(kmm);
		std::ext::delegate_ptr<sysfs> sysfs_impl	= nullptr;
		type_info_map kernel_type_info;
		pci_device_list* pci;
		virtual void* allocate_dma(size_t size, bool prefetchable) override { return mm->allocate_dma(size, prefetchable); }
		virtual void release_dma(void* ptr, size_t size) override { mm->deallocate_dma(ptr, size); }
		virtual void* map_dma(uintptr_t addr, size_t sz, bool prefetchable) override { return mm->map_dma(addr, sz, prefetchable); }
		virtual size_t dma_size(size_t requested) override { return kernel_memory_mgr::dma_size(requested); }
		virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) override { return pci->find(device_class, subclass); }
		virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass, uint8_t prog_if) override { return pci->find(device_class, subclass, prog_if); }
		virtual void* acpi_get_table(const char* label) override { return find_system_table(label); }
		virtual kmod_mm* create_mm() override { return new kmod_mm_impl(); }
		virtual void destroy_mm(kmod_mm* mod_mm) override { if(mod_mm) delete mod_mm; }
		virtual void log(std::type_info const& from, const char* message) override { xklog("[" + std::ext::demangle(from) + "]: " + message); }
		virtual void remove_actors(abstract_module_base* owner) override { interrupt_table::deregister_owner(dynamic_cast<void*>(owner)); }
		virtual uint32_t register_device(dev_stream<char>* stream, device_type type) override { return dreg.add(stream, type); }
		virtual bool deregister_device(dev_stream<char>* stream) override { return dreg.remove(stream); }
		virtual void register_type_info(std::type_info const* ti) override { kernel_type_info.insert(ti); }
		virtual uintptr_t vtranslate(void* addr) noexcept override { return translate_vaddr(addr); }
		virtual void on_irq(uint8_t irq, isr_actor&& handler, abstract_module_base* owner) override {
			try { interrupt_table::add_irq_handler(dynamic_cast<void*>(owner), irq, std::forward<isr_actor>(handler)); }
			catch(...) { owner->raise_error("out of memory", -ENOMEM); }
		}
		virtual std::pair<std::optional<sysfstream>, int> sysfs_open(const char* name, bool create) noexcept override
		{
			if(__unlikely(!this->__get_sysfs_delegate())) return std::pair<std::optional<sysfstream>, int>(std::nullopt, ENOSYS); 
			try
			{
				uint32_t ino	= sysfs_impl->find_node(name);
				if(!ino)
				{
					//	Note: the errno component of the pair is for unexpected errors rather than not-found results
					if(!create)
						return std::pair<std::optional<sysfstream>, int>(std::nullopt, 0);
					ino			= sysfs_impl->mknod(name, sysfs_object_type::GENERAL_CONFIG);
				}
				sysfs_vnode& n	= sysfs_impl->open(ino);
				n.pubseekpos(0Z);
				return std::make_pair(std::optional<sysfstream>(std::in_place, n), 0);
			}
			catch(std::bad_alloc&) { return std::pair<std::optional<sysfstream>, int>(std::nullopt, ENOMEM); }
			catch(std::exception& e) { return panic(e.what()), std::pair<std::optional<sysfstream>, int>(std::nullopt, EINVAL); }
		}
		virtual bool export_type_info(std::type_info const& ti) override
		{
			//	Only class types that aren't already registered can be exported to other modules.
			if(__unlikely(kernel_type_info.contains(ti.__type_name) || !dynamic_cast<__class_type_info const*>(std::addressof(ti)))) return false;
			register_type_info(std::addressof(ti));
			return true;
		}
		virtual bool import_type_info(std::type_info const& ti) override
		{
			std::type_info const* kernel_ti				= kernel_type_info.get_generic(ti.__type_name);
			if(__unlikely(!kernel_ti)) return false;
			const_cast<std::type_info&>(ti).__type_name	= kernel_ti->__type_name;
			return true;
		}
		virtual void relocate_type_info(abstract_module_base* mod, std::type_info const* local_si, std::type_info const* local_vmi) override
		{
			std::type_info const* kernel_si						= std::addressof(typeid(__si_class_type_info));
			std::type_info const* kernel_vmi					= std::addressof(typeid(__vmi_class_type_info));
			const_cast<std::type_info*>(local_si)->__type_name	= kernel_si->__type_name;
			const_cast<std::type_info*>(local_vmi)->__type_name	= kernel_vmi->__type_name;
			__class_type_info* local_class_meta					= const_cast<__class_type_info*>(dynamic_cast<__si_class_type_info const&>(*local_si).__base_type);
			local_class_meta->__type_name						= typeid(__class_type_info).__type_name;
			this->__relocate_ti_r(const_cast<std::type_info*>(std::addressof(typeid(*mod))));
		}
		[[nodiscard]] virtual netdev_api_helper* create_net_helper(abstract_netdev_module& mod) override
		{ 
			try { return new(mod.allocate_buffer(sizeof(netdev_api_helper_impl), alignof(netdev_api_helper_impl))) netdev_api_helper_impl(mod); }
			catch(...) {
				mod.raise_error("bad_alloc", -ENOMEM);
				__builtin_unreachable();
			}
		}
		virtual size_t vformat(kmod_mm* mm, const char* fmt, const char*& out, va_list args) override
		{
			try
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
			catch(...) { return panic("[KAPI] no memory"), 0UZ; }
		}
		virtual size_t vlogf(std::type_info const& from, const char* fmt, va_list args)
		{
			try
			{
				char* result	= nullptr;
				size_t count	= kvasprintf(std::addressof(result), fmt, args);
				if(!count) return 0UZ;
				xklog("[" + std::ext::demangle(from) + "]: " + result);
				free(result);
				return count;
			}
			catch(...) { return panic("[KAPI] no memory"), 0UZ; }
		}
		[[noreturn]] virtual void ctx_raise(module_eh_ctx& ctx, const char* msg, int status) override
		{
			ctx.msg			= msg;
			ctx.status		= status ? status : -1;
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
		virtual bool msi(abstract_module_base* owner, std::array<isr_actor, 7>&& handlers, msi32_t volatile& reg, msi_trigger_mode mode) override
		{
			using namespace interrupt_table;
			uint8_t count								= 0UC;
			while(count <= 7 && handlers[count]) count++;
			if(count > 1UC && !reg.message_control.multi_message_capable) return panic("[KAPI] cannot use multi-MSI on unsupported device"), false;
			msi_info* result							= allocate_msi_vectors(dynamic_cast<void*>(owner), count, reg.message_address, reg.message_data, mode);
			if(__unlikely(!result)) return panic("[KAPI] failed to allocate MSI vectors"), false;
			reg.message_control.multi_message_enable	= count;
			reg.message_address							= qword(result[0].msg_addr_value).lo;
			reg.message_data							= result[0].msg_data_value;
			for(uint8_t i = 0UC; i < count; i++)
			result[i].callback							= std::move(handlers[i]);
			return true;
		}
		virtual bool msi(abstract_module_base* owner, std::array<isr_actor, 7>&& handlers, msi64_t volatile& reg, msi_trigger_mode mode) override
		{
			using namespace interrupt_table;
			uint8_t count								= 0UC;
			while(count <= 7 && handlers[count]) count++;
			if(count > 1UC && !reg.message_control.multi_message_capable) return panic("[KAPI] cannot use multi-MSI on unsupported device"), false;
			msi_info* result							= allocate_msi_vectors(dynamic_cast<void*>(owner), count, reg.message_address, reg.message_data, mode);
			if(__unlikely(!result)) return panic("[KAPI] failed to allocate MSI vectors"), false;
			reg.message_control.multi_message_enable	= count;
			reg.message_address							= result[0].msg_addr_value;
			reg.message_data							= result[0].msg_data_value;
			for(uint8_t i = 0UC; i < count; i++)
			result[i].callback							= std::move(handlers[i]);
			return true;
		}
		virtual bool extended_msi(abstract_module_base* owner, isr_actor&& handler, msix_t volatile& reg, msi_trigger_mode mode) override
		{
			using namespace interrupt_table;
			msi_info* result	= allocate_msi_vector(dynamic_cast<void*>(owner), std::move(handler), reg.msg_addr, dword(reg.msg_data).lo, mode);
			if(__unlikely(!result)) return panic("[KAPI] failed to allocate MSIX vector"), false;
			reg.msg_addr		= result->msg_addr_value;
			reg.msg_data		= result->msg_data_value;
			return true;
		}
		virtual int save_config(abstract_module_base* mod) override
		{
			if(__unlikely(!this->__get_sysfs_delegate())) return ENOSYS;
			generic_config_table& cfg	= mod->get_config();
			if(__unlikely(!cfg.size())) return 0;	// vacuous success case
			typedef std::optional<sysfs_object_handle<generic_config_table>> opt_cfg_handle;
			try
			{
				std::string cfg_name		= std::string(typeid(*mod).name()) + "::config";
				opt_cfg_handle existing_cfg	= sysfs_impl->find<generic_config_table>(cfg_name);
				if(!existing_cfg) sysfs_impl->create(cfg_name, cfg);
				else if(__builtin_memcmp(existing_cfg->base(), std::addressof(cfg), cfg.size()))
					*existing_cfg			= cfg;
				else existing_cfg->release();	//	only write if the values are different; if they are the same, save time and don't write anything
				return 0;
			}
			catch(std::runtime_error& e) { return panic(e.what()), ENOSPC; }
			catch(std::bad_alloc&) { return ENOMEM; }
			catch(std::logic_error& e) { return panic(e.what()), EINVAL; }
		}
		virtual int load_config(abstract_module_base* mod) override
		{
			if(__unlikely(!this->__get_sysfs_delegate())) return ENOSYS;
			generic_config_table& cfg	= mod->get_config();
			if(__unlikely(!cfg.size())) return 0;	// vacuous success case
			typedef std::optional<sysfs_object_handle<const generic_config_table>> opt_cfg_handle;
			try
			{
				std::string cfg_name					= std::string(typeid(*mod).name()) + "::config";
				opt_cfg_handle existing_cfg				= sysfs_impl->find<const generic_config_table>(cfg_name);
				if(existing_cfg.has_value())
				{
					generic_config_table const* saved	= std::to_address(existing_cfg.value());
					if(__unlikely(saved->size() != cfg.size())) return panic("[KAPI] config table size mismatch"), E2BIG;
					array_copy<char>(addr_t(std::addressof(cfg.params)), addr_t(std::addressof(saved->params)), saved->size());
				}
				//	If there is no config to load then the default value is used; this is not an error
				return 0;
			}
			catch(std::bad_alloc&) { return ENOMEM; }
			catch(std::logic_error& e) { return panic(e.what()), EINVAL; }
		}
		void __relocate_si_r(__si_class_type_info* ti)
		{
			__class_type_info* base			= const_cast<__class_type_info*>(ti->__base_type);
			__class_type_info const* equiv	= kernel_type_info[base->__type_name];
			if(!equiv) this->__relocate_ti_r(base);
			else ti->__base_type			= equiv;
		}
		void __relocate_vmi_r(__base_class_type_info* bases, size_t num_bases)
		{
			for(size_t i = 0UZ; i < num_bases; i++)
			{
				__class_type_info* base			= const_cast<__class_type_info*>(bases[i].__base_type);
				__class_type_info const* equiv	= kernel_type_info[base->__type_name];
				if(!equiv) this->__relocate_ti_r(base);
				else bases[i].__base_type		= equiv;
			}
		}
		void __relocate_ti_r(std::type_info* ti)
		{
			if(__si_class_type_info* si			= dynamic_cast<__si_class_type_info*>(ti))
				__relocate_si_r(si);
			else if(__vmi_class_type_info* vmi	= dynamic_cast<__vmi_class_type_info*>(ti))
				__relocate_vmi_r(vmi->__base_info, vmi->__base_count);
		}
		bool __get_sysfs_delegate()
		{
			if(sysfs_impl) return true;
			sysfs_impl	= std::ext::latest;
			return static_cast<bool>(sysfs_impl);
		}
	} __api_impl{};
	void register_type(std::type_info const& ti) { __api_impl.register_type_info(std::addressof(ti)); }
	kmod_mm_impl::kmod_mm_impl() : kmod_mm(), kframe_tag(), mm(__api_impl.mm) {}
	void* kmod_mm_impl::mem_allocate(size_t size, size_t align)
	{
		block_tag* tag						= get_for_allocation(size ? size : 1UL, align);
		if(!tag) throw std::bad_alloc();
		lock(std::addressof(mod_mutex));
		managed_blocks.push_back(tag);
		release(std::addressof(mod_mutex));
		return tag->actual_start();
	}
	void kmod_mm_impl::mem_release(void* block, size_t align)
	{
		if(__unlikely(!block)) return;
		if(block_tag* tag = find_tag(block, align))
		{
			lock(std::addressof(mod_mutex));
			std::vector<block_tag*>::iterator i	= managed_blocks.find(tag);
			if(i != managed_blocks.end()) managed_blocks.erase(i);
			else xklog("[KMOD_MM] W: released a block that was not managed: " + std::to_string(tag));
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
		for(block_tag* tag : managed_blocks) release_block(tag);
		for(size_t i = 0UZ; i < block_index_range; i++)
		{
			for(block_tag* tag	= available_blocks[i]; tag; tag = tag->next)
			{
				while(tag->left_split)	tag	= melt_left(tag);
				while(tag->right_split)	tag	= melt_right(tag);
				kmm.deallocate_block(tag, tag->block_size, false);
			}
		}
	}
	int sysfstream::read(void* dst, size_t n)
	{
		size_t actual	= node.sgetn(static_cast<char*>(dst), n);
		if(__unlikely(!actual)) return ENOMEM;
		if(__unlikely(actual < n)) return -1;	// general code to indicate less data than expected
		return 0;
	}
	int sysfstream::write(const void* src, size_t n)
	{
		try
		{
			size_t actual	= node.sputn(static_cast<const char*>(src), n);
			if(__unlikely(actual < n)) return ENOSPC;
			dirty			= true;
			return 0;
		}
		catch(std::bad_alloc&) { return ENOMEM; }
	}
	int sysfstream::seekpos(std::streampos where)
	{
		std::streampos result	= node.pubseekpos(where);
		if(__unlikely(result < 0)) return ERANGE;
		return 0;
	}
	int sysfstream::seekoff(std::streamoff off, std::ios_base::seekdir way)
	{
		std::streampos result	= node.pubseekoff(off, way);
		if(__unlikely(result < 0)) return ERANGE;
		return 0;
	}
	sysfstream::~sysfstream()
	{
		if(!dirty) return;
		node.commit(node.count());
		node.pubsync();
		node.sync_parent();
	}
	kernel_api* get_api_instance()
	{
		if(__unlikely(!__api_impl.pci || !__api_impl.mm))
			return nullptr;
		else return std::addressof(__api_impl);
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
		register_type(typeid(abstract_netdev));
		register_type(typeid(abstract_netdev_module));
		register_type(typeid(abstract_connectable_device));
		register_type(typeid(abstract_connectable_device::interface));
		register_type(typeid(abstract_connectable_device::provider));
		register_type(typeid(abstract_hub_module_base));
	}
	void module_takedown(abstract_module_base* mod)
	{
		if(mod && mod->__api_hooks)
		{
			mod->finalize();
			if(mod->__fini_fn) (*mod->__fini_fn)();
			mod->__api_hooks->remove_actors(mod);
			if(mod->__allocated_mm) mod->__api_hooks->destroy_mm(mod->__allocated_mm);
		}
	}
}