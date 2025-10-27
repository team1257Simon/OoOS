#ifndef __MODULE_HDA
#define __MODULE_HDA
#include <fs/block_device.hpp>
#include <module.hpp>
namespace ooos
{
    class delegate_hda : public partitioned_block_device
    {
        abstract_module_base* __provider_module;
        abstract_block_device* __block_device;
        partition_table __part_table;
        void __read_pt();
        bool __await_disk(unsigned int ticket);
    public:
        void initialize(block_io_provider_module& provider);
        virtual bool read(void* out, uint64_t start_sector, uint32_t count) override;
        virtual bool write(uint64_t start_sector, const void* in, uint32_t count) override;
        virtual size_t sector_size() const override;
        virtual partition_table& get_partition_table() override;
        delegate_hda() noexcept;
        delegate_hda(block_io_provider_module& provider);
    };
}
#endif