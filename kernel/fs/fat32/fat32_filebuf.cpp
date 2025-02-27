#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
fat32_filebuf::fat32_filebuf(std::vector<uint32_t> &&covered_clusters, fat32_file_node* pn) : 
    __base                  {}, 
    __my_clusters           { covered_clusters }, 
    __next_cluster_idx      { 0UL },
    __parent                { pn }
                            {}
std::streamsize fat32_filebuf::__ddrem() { if(this->__next_cluster_idx < this->__my_clusters.size()) { return (static_cast<size_t>(__my_clusters.size() - (__next_cluster_idx + 1)) * physical_block_size) + 1UL; /* The minimum nuber of bytes remaining, if there are any unread clusters, is 1 (for a cluster with only 1 written byte) */ } else return 0UL; }
int fat32_filebuf::__ddwrite() { size_t n = 0; for(size_t i = 0; i < __my_clusters.size() && n < this->__size(); i++, n += physical_block_size) { if(!__parent->parent_fs->write_clusters(__my_clusters[i], this->__get_ptr(n))) { return -1; } } return 0; }
std::streamsize fat32_filebuf::__ddread(std::streamsize n)
{
    size_t s = div_roundup(n, physical_block_size);
    size_t k = 0;
    if(!this->__grow_buffer(s * physical_block_size)) return 0;
    if(!gptr()) { this->setg(this->__beg(), this->__cur(), this->__max()); }
    for(size_t i = 0; i < s && __next_cluster_idx < __my_clusters.size(); i++, ++__next_cluster_idx) { if(__parent->parent_fs->read_clusters(this->__get_ptr(k), __my_clusters[i])) { k += physical_block_size * __parent->parent_fs->__sectors_per_cluster; } else break; }
    this->setg(this->__beg(), this->gptr(), this->__max());
    return std::min(k, n);
}
std::streamsize fat32_filebuf::__overflow(std::streamsize n)
{
    size_t s = div_roundup(n, physical_block_size);
    size_t k = 0;
    for(size_t i = 0; i < s; i++, k += physical_block_size) { if(uint32_t cl = __parent->claim_next(__my_clusters.back())) { __my_clusters.push_back(cl); } else break; }
    if(!this->__grow_buffer(k)) return 0;
    return k;
}