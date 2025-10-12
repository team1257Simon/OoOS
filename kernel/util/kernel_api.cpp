#include "kernel_api.hpp"
#include "arch/pci_device_list.hpp"
#include "isr_table.hpp"
#include "unordered_map"
#include "stdexcept"
namespace ooos_kernel_module
{
    void isr_actor_base::actor_manager_wrapper::manager_action(functor_store& dst, functor_store const& src, mgr_op op) { if(*this) { (*manager)(dst, src, mm, op); } }
    void isr_actor_base::actor_manager_wrapper::invoke(functor_store& fn) { if(*this) { (*invoker)(fn); } }
    isr_actor_base::~isr_actor_base() noexcept { if(__my_wrapper) __my_wrapper.manager_action(__my_actor, __my_actor, destroy); }
    isr_actor::isr_actor() noexcept = default;
    isr_actor::~isr_actor() noexcept = default;
    isr_actor::isr_actor(isr_actor const& that) : isr_actor_base(that.__my_wrapper) { __my_wrapper.manager_action(this->__my_actor, that.__my_actor, clone); }
    isr_actor::isr_actor(isr_actor&& that) : isr_actor_base(std::move(that.__my_wrapper)) { this->__my_actor = that.__my_actor; }
    void isr_actor::operator()() { __my_wrapper.invoke(__my_actor); }
    typedef std::unordered_map<abstract_module_base*, isr_actor> actor_by_owner_map;
    typedef std::unordered_map<byte, actor_by_owner_map> actor_by_index_map;
    struct activator { actor_by_owner_map& actors; __isrcall void operator()() { for(actor_by_owner_map::value_type& pair : actors) { pair.second(); } } };
    isr_actor& isr_actor::operator=(isr_actor const& that)
    {
        this->__my_wrapper = that.__my_wrapper;
        this->__my_wrapper.manager_action(this->__my_actor, that.__my_actor, clone);
        return *this;
    }
    isr_actor& isr_actor::operator=(isr_actor&& that)
    {
        this->__my_wrapper          = that.__my_wrapper;
        this->__my_actor            = that.__my_actor;
        that.__my_wrapper.manager   = nullptr;
        that.__my_wrapper.invoker   = nullptr;
        that.__my_wrapper.mm        = nullptr;
        return *this;
    }
    actor_by_owner_map& at_index(uint8_t irq, actor_by_index_map& by_index)
    {
        if(by_index.contains(irq)) return by_index[irq];
        actor_by_owner_map& result = by_index.insert(std::make_pair(irq, std::forward<actor_by_owner_map>(16UL))).first->second;
        interrupt_table::add_irq_handler(irq, std::move(activator(result)));
        return result;
    }
    struct kmod_mm_impl : kmod_mm, kframe_tag
    {
        kernel_memory_mgr* mm;
        block_tag* first_managed_block;
        spinlock_t mod_mutex{};
        virtual void* mem_allocate(size_t size, size_t align) override;
        virtual void mem_release(void* block, size_t align) override;
        virtual kframe_tag* get_frame() override { return this; }
        virtual ~kmod_mm_impl();
        kmod_mm_impl();
    };
    static struct : kernel_api
    {
        kernel_memory_mgr* mm;
        pci_device_list* pci;
        actor_by_index_map* actors_by_index;
        char actors_by_index_data[sizeof(actor_by_index_map)];
        virtual void* allocate_dma(size_t size, bool prefetchable) override { return mm->allocate_dma(size, prefetchable); }
        virtual void release_dma(void* ptr, size_t size) override { mm->deallocate_dma(ptr, size); }
        virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) override { return pci->find(device_class, subclass); }
        virtual void* acpi_get_table(const char* label) override { return find_system_table(label); }
        virtual kmod_mm* create_mm() override { return new kmod_mm_impl(); }
        virtual void destroy_mm(kmod_mm* mm) override { if(mm) delete mm; }
        virtual void log(std::type_info const& from, const char* message) override { xklog("[" + std::ext::demangle(from) + "]: " + message); }
        virtual void remove_actors(abstract_module_base* owner) override { for(actor_by_index_map::value_type& pair : *actors_by_index) { pair.second.erase(owner); } }
        virtual void on_irq(uint8_t irq, isr_actor&& handler, abstract_module_base* owner) override { at_index(irq, *actors_by_index).insert_or_assign(owner, std::move(handler)); }
        virtual void init_memory_fns(kframe_exports* ptrs) override
        {
            new(ptrs) kframe_exports
            {
                .allocate       = &kframe_tag::allocate,
                .array_allocate = &kframe_tag::array_allocate,
                .deallocate     = &kframe_tag::deallocate,
                .reallocate     = &kframe_tag::reallocate
            };
        }
    } __api_impl{};
    kmod_mm_impl::kmod_mm_impl() : kmod_mm(), kframe_tag(), mm(__api_impl.mm), first_managed_block(nullptr) {}
    void* kmod_mm_impl::mem_allocate(size_t size, size_t align)
    {
        block_tag* tag = get_for_allocation(size ? size : 1UL, align);
        if(!tag) throw std::bad_alloc();
        lock(std::addressof(mod_mutex));
        if(!first_managed_block) first_managed_block = tag;
        else
        {
            tag->next                       = first_managed_block;
            first_managed_block->previous   = tag;
            first_managed_block             = tag;
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
            if(tag->previous) tag->previous->next               = tag->next;
            if(tag->next) tag->next->previous                   = tag->previous;
            if(first_managed_block == tag) first_managed_block  = tag->next;
            tag->previous   = nullptr;
            tag->next       = nullptr;
            release_block(tag);
            release(std::addressof(mod_mutex));
        }
    }
    kmod_mm_impl::~kmod_mm_impl()
    {
        block_tag* tag = first_managed_block;
        while(tag)
        {
            block_tag* next     = tag->next;
            release_block(tag);
            tag                 = next;
        }
    }
    void init_api() { __api_impl.mm = std::addressof(kmm); __api_impl.pci = pci_device_list::get_instance(); __api_impl.actors_by_index = new(__api_impl.actors_by_index_data) ooos_kernel_module::actor_by_index_map(16UL); }
    kernel_api* get_api_instance() { if(__unlikely(!__api_impl.pci || !__api_impl.mm)) return nullptr; else return std::addressof(__api_impl); }
    
}