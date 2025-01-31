#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
fat32_allocation_table::fat32_allocation_table(size_t num_sectors, uint64_t start_sector) : __base{ num_sectors * physical_block_size / sizeof(uint32_t) }, __num_sectors{ num_sectors }, __start_sector{ start_sector } { get_from_disk();}
bool fat32_allocation_table::sync_to_disk() const { return ahci_hda::is_initialized() && ahci_hda::write(__start_sector, reinterpret_cast<const char*>(this->__beg()), this->__num_sectors); }
bool fat32_allocation_table::get_from_disk() { if(ahci_hda::is_initialized()) { this->__advance(ahci_hda::read(reinterpret_cast<char*>(this->__beg()), __start_sector, __num_sectors) / sizeof(uint32_t)); return true; } return false; }
uint64_t fat32::__cluster_to_sector(uint32_t cl) const noexcept { return (cl - 2) * __sectors_per_cluster + __sector_base; }
dev_t fat32::xgdevid() const noexcept { return __dev_serial; }