#ifndef __BLOCK_DEVICE
#define __BLOCK_DEVICE
#include "libk_decls.h"
#include "vector"
typedef struct __pt_header
{
	char sig[8];
	uint32_t revision;
	uint32_t header_size;
	char checksum[4];
	char rsv0[4];
	uint64_t lba_header;
	uint64_t lba_alternate;
	uint64_t lba_first_usable;
	uint64_t lba_last_usable;
	guid_t guid;
	uint64_t lba_partition_entry_array;
	uint32_t num_part_entries;
	uint32_t part_entry_size; // will be a multiple of 128
	uint32_t partition_array_crc32_checksum;
	char rsv1[physical_block_size - 0x5CU];
} __pack pt_header_t;
typedef struct __part_table_entry
{
	guid_t type_guid;
	guid_t part_guid;
	uint64_t start_lba;
	uint64_t end_lba;
	uint64_t attributes;
	char16_t part_name[36];
} __pack partition_entry_t;
typedef std::vector<partition_entry_t> partition_table;
#ifdef INST_PT
template class std::vector<partition_entry_t>;
#else
extern template class std::vector<partition_entry_t>;
#endif
// Interface for block device I/O handlers.
struct block_device
{
	virtual bool read(void* out, uint64_t start_sector, uint32_t count)			= 0;
	virtual bool write(uint64_t start_sector, const void* in, uint32_t count)	= 0;
	virtual size_t sector_size() const = 0;
};
struct partitioned_block_device : block_device { virtual partition_table& get_partition_table() = 0; };
#endif