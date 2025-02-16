#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
bool fat32::__has_init{ false };
fat32* fat32::__instance;
static std::aligned_allocator<fat32, filesystem> fat_alloc{};
uint64_t figure_start_sector() { if(ahci_hda::is_initialized() && !ahci_hda::get_partition_table().empty()) { return ahci_hda::get_partition_table().front().start_lba; } return 2048UL; /* default value assumed for now */ }
fat32_allocation_table::fat32_allocation_table(size_t num_sectors, size_t bytes_per_sector, uint64_t start_sector) : __base{ num_sectors * bytes_per_sector / sizeof(uint32_t) }, __num_sectors{ num_sectors }, __start_sector{ start_sector } { get_from_disk(); }
bool fat32_allocation_table::sync_to_disk() const { return ahci_hda::is_initialized() && ahci_hda::write(__start_sector, reinterpret_cast<const char*>(this->__beg()), this->__num_sectors); }
bool fat32_allocation_table::get_from_disk() { if(ahci_hda::is_initialized() && ahci_hda::read(reinterpret_cast<char*>(this->__beg()), __start_sector, __num_sectors)) { this->__setc(this->__max()); return true; } return false; }
void fat32::__release_clusters_from(uint32_t start) { for(fat32_allocation_table::iterator i = __the_table.from(start), j = i.next(); i != __the_table.end(); i = j++) *i &= fat32_cluster_pres; }
uint64_t fat32::cluster_to_sector(uint32_t cl) const noexcept { return (cl - 2) * __sectors_per_cluster + __sector_base; }
dev_t fat32::xgdevid() const noexcept { return __dev_serial; }
void fat32::add_start_cluster_ref(uint64_t cl) { std::map<uint64_t, size_t>::iterator i = __st_cluster_ref_counts.find(cl); if(i != __st_cluster_ref_counts.end()) { i->second++;  } else { __st_cluster_ref_counts.insert(std::make_pair(cl, 1UL)); }}
void fat32::rm_start_cluster_ref(uint64_t cl) { std::map<uint64_t, size_t>::iterator i = __st_cluster_ref_counts.find(cl); if(i != __st_cluster_ref_counts.end()) { i->second--; } }
fat32::fat32(uint32_t root_cl, uint8_t sectors_per_cl, uint16_t bps, uint64_t first_sect, uint64_t fat_sectors, dev_t drive_serial) : filesystem{}, __root_cl_num{ root_cl }, __sectors_per_cluster{ sectors_per_cl }, __sector_base{ first_sect + fat_sectors }, __dev_serial{ drive_serial }, __the_table{ fat_sectors, bps, first_sect } {}
void fat32::syncdirs() { if(__root_directory && __root_directory->fsync()) { __the_table.sync_to_disk(); } /* Directory fsync is recursive for fat32 implementation */ }
folder_inode* fat32::get_root_directory() { return __root_directory; }
file_inode* fat32::open_fd(tnode* n) { if(fat32_file_inode* fn = dynamic_cast<fat32_file_inode*>(n->as_file())) {fn->set_fd(this->next_fd++); fn->on_open(); return fn; } return nullptr; }
void fat32::dlfilenode(file_inode* fd)
{
    fd->prune_refs();
    std::map<uint64_t, size_t>::iterator i = __st_cluster_ref_counts.find(fd->cid());
    if(!i->second) { __release_clusters_from(static_cast<uint32_t>(i->first)); __st_cluster_ref_counts.erase(i); }
    __file_nodes.erase(*fd);
    this->syncdirs();
}
void fat32::dldirnode(folder_inode* dd)
{    
    if(!dd->is_empty()) { throw std::logic_error{ std::string{ "cannot delete folder " } + dd->name() + " because it is not empty" }; }
    dd->prune_refs();
    std::map<uint64_t, size_t>::iterator i = __st_cluster_ref_counts.find(dd->cid());
    if(!i->second) { __release_clusters_from(static_cast<uint32_t>(i->first)); __st_cluster_ref_counts.erase(i); }
    __folder_nodes.erase(*dd);
    this->syncdirs();
}
file_inode* fat32::mkfilenode(folder_inode* parent, std::string const& name)
{
    dword cl = claim_cluster(__the_table);
    if(!cl) return nullptr;
    std::vector<fat32_directory_entry>::iterator avail = dynamic_cast<fat32_folder_inode&>(*parent).first_unused_entry();
    avail->regular_entry.first_cluster_hi = cl.hi;
    avail->regular_entry.first_cluster_lo = cl.lo;
    std::set<fat32_file_inode>::iterator i = __file_nodes.emplace(this, name, std::addressof(avail->regular_entry)).first;
    add_start_cluster_ref(i->start_cluster());
    return i.base();
}
folder_inode* fat32::mkdirnode(folder_inode* parent, std::string const& name)
{
    dword cl = claim_cluster(__the_table);
    if(!cl) return nullptr;
    std::vector<fat32_directory_entry>::iterator avail = dynamic_cast<fat32_folder_inode&>(*parent).first_unused_entry();
    avail->regular_entry.first_cluster_hi = cl.hi;
    avail->regular_entry.first_cluster_lo = cl.lo;
    std::set<fat32_folder_inode>::iterator i = __folder_nodes.emplace(this, name, std::addressof(avail->regular_entry)).first;
    add_start_cluster_ref(i->start_cluster());
    return i.base();
}
bool fat32::init() { __root_directory = __folder_nodes.emplace(this, std::string(), __root_cl_num).first.base(); __root_directory->parse_dir_data(); return __root_directory->valid(); }
fat32_file_inode *fat32::put_file_node(std::string const& name, fat32_regular_entry* e) { std::pair<std::set<fat32_file_inode>::iterator, bool> result = __file_nodes.emplace(this, name, e); if(!result.second) { return nullptr; } return result.first.base(); }
fat32_folder_inode *fat32::put_folder_node(std::string const& name, fat32_regular_entry* e) { std::pair<std::set<fat32_folder_inode>::iterator, bool> result = this->__folder_nodes.emplace(this, name, e); if(!result.second) { return nullptr; } return result.first.base(); }
bool fat32::has_init() { return __has_init; }
fat32 *fat32::get_instance() { return __instance; }
bool fat32::init_instance()
{
    if(__has_init) return true;
    fat32_bootsect bootsect{};
    uint64_t ss = figure_start_sector();
    if(!ahci_hda::read_object(bootsect, ss)) return false;
    __instance = new fat32(bootsect.root_cluster_num, bootsect.sectors_per_cluster, bootsect.bytes_per_sector, ss + bootsect.num_reserved_sectors, bootsect.num_fats * bootsect.fat_size, bootsect.volume_serial);
    __has_init = __instance->init();
    return __has_init;
}