#include "kernel_api.hpp"
#include "isr_table.hpp"
#include "kernel_mm.hpp"
struct kmod_mm_impl : kmod_mm, kframe_tag
{
    kernel_memory_mgr* mm;
    block_tag* first_managed_block;
    spinlock_t mod_mutex{};
    virtual void* mem_allocate(size_t size, size_t align) override;
    virtual void mem_release(void* block, size_t align) override;
    virtual ~kmod_mm_impl();
    kmod_mm_impl();
};
static struct : kernel_api
{
    kernel_memory_mgr* mm;
    pci_device_list* pci;
    virtual void* allocate_dma(size_t size, bool prefetchable) override { return mm->allocate_dma(size, prefetchable); }
    virtual pci_config_space* find_pci_device(uint8_t device_class, uint8_t subclass) override { return pci->find(device_class, subclass); }
    virtual void* acpi_get_table(const char* label) override { return find_system_table(label); }
    virtual kmod_mm* create_mm() override { return new kmod_mm_impl(); }
    virtual void destroy_mm(kmod_mm* mm) override { if(mm) delete mm; }
} __api_impl{};
void init_api() { __api_impl.mm = kernel_memory_mgr::__instance; __api_impl.pci = pci_device_list::get_instance(); }
kernel_api* get_api_instance() { if(__unlikely(!__api_impl.pci || !__api_impl.mm)) return nullptr; else return std::addressof(__api_impl); }
void* kmod_mm_impl::mem_allocate(size_t size, size_t align)
{
    if(__unlikely(!size)) return nullptr;
    block_tag* tag = get_for_allocation(size, align);
    if(__unlikely(!tag)) return nullptr;
    lock(std::addressof(mod_mutex));
    if(!first_managed_block) first_managed_block = tag;
    else
    {
        tag->next = first_managed_block;
        first_managed_block->previous = tag;
        first_managed_block = tag;
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
        if(tag->previous) tag->previous->next = tag->next;
        if(tag->next) tag->next->previous = tag->previous;
        if(first_managed_block == tag) first_managed_block = tag->next;
        tag->previous = nullptr;
        tag->next = nullptr;
        release_block(tag);
        release(std::addressof(mod_mutex));
    }
}
kmod_mm_impl::kmod_mm_impl() : kmod_mm(), kframe_tag(), mm(__api_impl.mm), first_managed_block(nullptr) {}
kmod_mm_impl::~kmod_mm_impl()
{
    block_tag* tag = first_managed_block;
    while(tag)
    {
        block_tag* next = tag->next;
        release_block(tag);
        tag = next;
    }
}