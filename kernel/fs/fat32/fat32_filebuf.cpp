#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
fat32_filebuf::fat32_filebuf(std::vector<uint32_t> &&covered_clusters, __cl_conv_fn_t const& cluster_convert_fn, __cl_get_fn_t const& add_cluster_fn) : 
    __base                  { physical_block_size * 2 }, 
    __my_clusters           { std::move(covered_clusters) }, 
    __next_cluster_idx      { 0UL },
    __cluster_to_sector_fn  { cluster_convert_fn },
    __add_cluster_fn        { add_cluster_fn }
                            { if(ahci_hda::is_initialized() && !__my_clusters.empty() && bool(__cluster_to_sector_fn)) { size_t n = ahci_hda::read(this->__beg(), __cluster_to_sector_fn(__my_clusters.front()), 1); if(n) { this->__bumpc(int64_t(n)); this->__next_cluster_idx++; } } }
std::streamsize fat32_filebuf::__ddrem() { if(this->__next_cluster_idx < this->__my_clusters.size()) { return (static_cast<size_t>(__my_clusters.size() - (__next_cluster_idx + 1)) * physical_block_size) + 1UL; /* The minimum nuber of bytes remaining, if there are any unread clusters, is 1 (for a cluster with only 1 written byte) */ } else return 0UL; }
std::streamsize fat32_filebuf::__ddread(std::streamsize n)
{
    size_t s = div_roundup(n, physical_block_size);
    size_t k = 0;
    if(!this->__grow_buffer(s * physical_block_size)) return 0;
    for(size_t i = 0; i < s && __next_cluster_idx < __my_clusters.size(); i++, ++__next_cluster_idx) { if(size_t r = ahci_hda::read(this->gptr() + k, this->__cluster_to_sector_fn(__my_clusters[__next_cluster_idx]), 1)) { k += r; } else break; }
    return k;
}
int fat32_filebuf::__ddwrite()
{
    size_t n = 0;
    for(uint32_t cl : __my_clusters) { if(!ahci_hda::write(__cluster_to_sector_fn(cl), this->__get_ptr(n), 1)) { return -1; } n += physical_block_size; }
    return 0;
}
std::streamsize fat32_filebuf::__overflow(std::streamsize n)
{
    size_t s = div_roundup(n, physical_block_size);
    size_t k = 0;
    for(size_t i = 0; i < s; i++, k += physical_block_size) { if(uint32_t cl = __add_cluster_fn(__my_clusters.back())) { __my_clusters.push_back(cl); } else break; }
    if(!this->__grow_buffer(k)) return 0;
    return k;
}