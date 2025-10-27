#include <fs/delegate_hda.hpp>
#include <ext/ptr_guard.hpp>
#include <functional>
static char __init_pt_hdr[sizeof(pt_header_t)];
constexpr static std::allocator<partition_entry_t> pt_alloc{};
namespace ooos
{
    delegate_hda::delegate_hda() noexcept = default;
    delegate_hda::delegate_hda(block_io_provider_module& provider) { initialize(provider); }
    partition_table& delegate_hda::get_partition_table() { return __part_table; }
    size_t delegate_hda::sector_size() const { return __block_device->block_size(); }
    bool ooos::delegate_hda::__await_disk(unsigned int ticket) { return await_result(std::bind(&abstract_block_device::io_complete, __block_device, ticket), max_wait * max_wait); }
    void delegate_hda::initialize(block_io_provider_module& provider)
    {
        int idx             = provider.index_of(BDT_HDD);
        if(idx < 0) throw std::runtime_error("[HDA] no HDD is present");
        __provider_module   = std::addressof(provider);
        __block_device      = std::addressof(provider[static_cast<unsigned>(idx)]);
        __read_pt();
    }
    void delegate_hda::__read_pt()
    {
        array_zero(__init_pt_hdr, sizeof(pt_header_t));
        if(!read(__init_pt_hdr, 1UL, 1U)) throw std::runtime_error("[HDA] failed to read GPT header");
        pt_header_t* hdr        = reinterpret_cast<pt_header_t*>(__init_pt_hdr);
        unsigned sz_multi       = hdr->part_entry_size / sizeof(partition_entry_t);
        if(!sz_multi) throw std::runtime_error("[HDA] invalid size for pt entries — GPT header is corrupted");
        size_t n                = hdr->num_part_entries * sz_multi;
        size_t block_size       = __block_device->block_size();
        size_t actual           = div_round_up(div_round_up(n * sizeof(partition_entry_t), block_size) * block_size, sizeof(partition_entry_t));
        partition_entry_t* arr  = pt_alloc.allocate(actual);
        std::ext::ptr_guard<partition_entry_t> guard(arr, actual, pt_alloc);
        array_zero<partition_entry_t>(arr, n);
        if(!read(arr, hdr->lba_partition_entry_array, div_round_up(n * sizeof(partition_entry_t), block_size))) throw std::runtime_error("[HDA] failed to read GPT");
        for(size_t i = 0; i < n; i += sz_multi) __part_table.push_back(arr[i]);
        pt_alloc.deallocate(arr, actual);
        guard.release();
    }
    bool delegate_hda::read(void* out, uint64_t start_sector, uint32_t count)
    {
        if(__unlikely(!__block_device)) { panic("[HDA] cannot access disk before initializing the delegate module"); return false; }
        std::function<bool()> check_io_avail(std::bind(&abstract_block_device::io_ready, __block_device));
        eh_exit_guard guard(__provider_module);
        module_eh_ctx& ctx  = __provider_module->get_eh_ctx();
        if(__unlikely(setjmp(ctx.handler_ctx)))
        {
            panic("[HDA] disk I/O raised an error status: ");
            panic(ctx.msg);
            return false;
        }
        size_t rem          = count;
        size_t t_read       = 0UZ;
        size_t s_read       = 0UZ;
        size_t s_per_op     = __block_device->max_operation_blocks();
        size_t b_per_s      = sector_size();
        if(__unlikely(!await_result(check_io_avail, max_wait))) { panic("[HDA] block device is unresponsive"); return false; }
        fence();
        while(rem)
        {
            size_t sct      = std::min(rem, s_per_op);
            int ticket      = __block_device->read(addr_t(out).plus(t_read), start_sector + s_read, sct);
            if(__unlikely(ticket < 0)) { panic("[HDA] block device has no available bandwidth"); return false; }
            if(__unlikely(!__await_disk(static_cast<unsigned>(ticket)))) { panic("[HDA] block device hung"); return false; }
            t_read          += b_per_s * sct;
            s_read          += sct;
            rem             -= sct;
        }
        fence();
        return true;
    }
    bool delegate_hda::write(uint64_t start_sector, const void* in, uint32_t count)
    {
        if(__unlikely(!__block_device)) { panic("[HDA] cannot access disk before initializing the delegate module"); return false; }
        std::function<bool()> check_io_avail(std::bind(&abstract_block_device::io_ready, __block_device));
        eh_exit_guard guard(__provider_module);
        module_eh_ctx& ctx  = __provider_module->get_eh_ctx();
        if(__unlikely(setjmp(ctx.handler_ctx)))
        {
            panic("[HDA] disk I/O raised an error status: ");
            panic(ctx.msg);
            return false;
        }
        size_t rem          = count;
        size_t t_write      = 0UZ;
        size_t s_write      = 0UZ;
        size_t s_per_op     = __block_device->max_operation_blocks();
        size_t b_per_s      = sector_size();
        if(__unlikely(!await_result(check_io_avail, max_wait))) { panic("[HDA] block device is unresponsive"); return false; }
        fence();
        while(rem)
        {
            size_t sct      = std::min(rem, s_per_op);
            int ticket      = __block_device->write(start_sector + s_write, addr_t(in).plus(t_write), sct);
            if(__unlikely(ticket < 0)) { panic("[HDA] block device has no available bandwidth"); return false; }
            if(__unlikely(!__await_disk(static_cast<unsigned>(ticket)))) { panic("[HDA] block device hung"); return false; }
            t_write         += b_per_s * sct;
            s_write         += sct;
            rem             -= sct;
        }
        fence();
        return true;
    }
}