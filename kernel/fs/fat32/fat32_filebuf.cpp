#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
std::streamsize fat32_filebuf::unread_size() { if(__next_cluster_idx < __my_clusters.size()) { return (static_cast<size_t>(__my_clusters.size() - (__next_cluster_idx + 1)) * __parent->parent_fs->block_size()) + 1UL; /* The minimum nuber of bytes remaining, if there are any unread clusters, is 1 (for a cluster with only 1 written byte) */ } else return 0UL; }
int fat32_filebuf::write_dev() { size_t bs = __parent->parent_fs->block_size(); size_t n = 0; for(size_t i = 0; i < __my_clusters.size() && n < __size(); i++, n += bs) { if(!__parent->parent_fs->write_clusters(__my_clusters[i], this->__get_ptr(n))) { return -1; } } return 0; }
int fat32_filebuf::sync() { if(is_dirty) { int result = write_dev(); is_dirty = (result >= 0); return result; } return 0; }
fat32_filebuf::fat32_filebuf(std::vector<uint32_t>&& covered_clusters, fat32_file_node* pn) : 
    __base                  {}, 
    __my_clusters           { covered_clusters }, 
    __next_cluster_idx      { 0UL },
    __parent                { pn }
                            {}
std::streamsize fat32_filebuf::read_dev(std::streamsize n)
{
    size_t bs = __parent->parent_fs->block_size();
    size_t s = div_round_up(n, bs);
    size_t k = 0;
    if(!__grow_buffer(s * bs)) return 0UZ;
    if(!gptr()) { setg(__beg(), __cur(), __max()); }
    for(size_t i = 0; i < s && __next_cluster_idx < __my_clusters.size(); i++, ++__next_cluster_idx) { if(__parent->parent_fs->read_clusters(__get_ptr(k), __my_clusters[i])) { k += bs; } else break; }
    setg(__beg(), __cur(), __max());
    return std::min(k, n);
}
std::streamsize fat32_filebuf::on_overflow(std::streamsize n)
{
    size_t bs = __parent->parent_fs->block_size();
    size_t s = div_round_up(n, bs);
    size_t k = 0;
    for(size_t i = 0; i < s; i++, k += bs) { if(uint32_t cl = __parent->claim_next(__my_clusters.back())) { __my_clusters.push_back(cl); } else break; }
    if(!__grow_buffer(k)) return 0UZ;
    return k;
}