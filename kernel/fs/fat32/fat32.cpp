#include "fs/fat32.hpp"
#include "rtc.h"
#include "stdexcept"
bool fat32::__has_init = false;
fat32* fat32::__instance;
static fat32_bootsect bootsect{};
static char driver_space[sizeof(fat32)]{};
static void set_filename(char* fname, std::string const& sname) { size_t pos_dot = sname.find('.'), l = std::min(8UL, sname.size()); if(pos_dot != std::string::npos && pos_dot < l) l = pos_dot; std::string::const_iterator i = sname.begin(); for(size_t j = 0; j < 8; j++) { if(j < l) { fname[j] = *i; i++; } else { fname[j] = ' '; } } ++i; for(size_t j = 8; j < 11; j++, i++) { fname[j] = (i < sname.end()) ? *i : ' '; } }
uint32_t claim_cluster(fat32_allocation_table& tb, uint32_t last_sect) { for(uint32_t i = 3U; i < tb.size(); i++) { if((tb[i] & fat32_cluster_mask) == 0) { if(last_sect > 2) { tb[last_sect] |= (i & fat32_cluster_mask); } tb[i] |= fat32_cluster_eof; tb.mark_dirty(); return i; } } return 0; }
fat32_allocation_table::fat32_allocation_table(size_t num_sectors, size_t bytes_per_sector, uint64_t start_sector, fat32* parent) : __base{ num_sectors * bytes_per_sector / sizeof(uint32_t) }, __num_sectors{ num_sectors }, __start_sector{ start_sector }, __parent{ parent } {}
bool fat32_allocation_table::sync_to_disk() const { if(__dirty) { if(__parent->write_blockdev(__start_sector, reinterpret_cast<char const*>(__beg()), __num_sectors)) { __dirty = false; } } return !__dirty; }
bool fat32_allocation_table::get_from_disk() { if(__parent->read_blockdev(reinterpret_cast<char*>(__beg()), __start_sector, __num_sectors)) { __setc(__max()); return true; } return false; }
void fat32::__release_clusters_from(uint32_t start) { uint32_t tval; do { tval = __the_table[start] & fat32_cluster_mask; __the_table[start] &= fat32_cluster_pres; start = tval; } while(tval < fat32_cluster_eof); __the_table.mark_dirty(); }
uint64_t fat32::cluster_to_sector(uint32_t cl) const noexcept { return (cl - 2) * __sectors_per_cluster + __sector_base; }
dev_t fat32::xgdevid() const noexcept { return __dev_serial; }
void fat32::add_start_cluster_ref(uint64_t cl) { std::map<uint64_t, size_t>::iterator i = __st_cluster_ref_counts.find(cl); if(i != __st_cluster_ref_counts.end()) { i->second++;  } else { __st_cluster_ref_counts.insert(std::make_pair(cl, 1UL)); }}
void fat32::rm_start_cluster_ref(uint64_t cl) { std::map<uint64_t, size_t>::iterator i = __st_cluster_ref_counts.find(cl); if(i != __st_cluster_ref_counts.end()) { i->second--; } }
fat32::~fat32() = default;
size_t fat32::block_size() { return __sector_size * __sectors_per_cluster; }
void fat32::syncdirs() { for(std::set<fat32_file_node>::iterator i = __file_nodes.begin(); i != __file_nodes.end(); i++) { i->fsync(); } for(std::set<fat32_directory_node>::iterator i = __directory_nodes.begin(); i != __directory_nodes.end(); i++) { i->fsync(); }__the_table.sync_to_disk(); }
directory_node* fat32::get_root_directory() { return __root_directory; }
bool fat32::write_clusters(uint32_t cl_st, const char* data, size_t num) { return write_blockdev(cluster_to_sector(cl_st), data, num * __sectors_per_cluster); }
bool fat32::read_clusters(char* buffer, uint32_t cl_st, size_t num) { return read_blockdev(buffer, cluster_to_sector(cl_st), num * __sectors_per_cluster); }
bool fat32::has_init() { return __has_init; }
fat32* fat32::get_instance() { return __instance; }
file_node* fat32::on_open(tnode* n) { if(fat32_file_node* fn = dynamic_cast<fat32_file_node*>(n->as_file())) { fn->on_open(); return fn; } return nullptr; }
fat32::fat32(uint32_t root_cl, uint8_t sectors_per_cl, uint16_t bps, uint64_t first_sect, uint64_t fat_sectors, dev_t drive_serial) :
	filesystem              {},
	__root_cl_num           { root_cl },
	__sectors_per_cluster   { sectors_per_cl },
	__sector_base           { first_sect + fat_sectors },
	__dev_serial            { drive_serial },
	__sector_size           { bps },
	__the_table             { fat_sectors, bps, first_sect, this }
							{}
fat32_file_node* fat32::put_file_node(std::string const& name, fat32_directory_node* parent, uint32_t cl0, size_t dirent_idx)
{
	std::pair<std::set<fat32_file_node>::iterator, bool> result = __file_nodes.emplace(this, name, parent, cl0, dirent_idx);
	if(!result.second) { return nullptr; }
	return result.first.base();
}
fat32_directory_node* fat32::put_directory_node(std::string const& name, fat32_directory_node* parent, uint32_t cl0, size_t dirent_idx)
{
	std::pair<std::set<fat32_directory_node>::iterator, bool> result = __directory_nodes.emplace(this, name, parent, cl0, dirent_idx);
	if(!result.second) { return nullptr; }
	return result.first.base();
}
bool fat32::init()
{
	if(__unlikely(!__the_table.get_from_disk())) return false;
	__root_directory = __directory_nodes.emplace(this, "", nullptr, __root_cl_num, 0UL).first.base(); 
	__root_directory->parse_dir_data(); 
	return __root_directory->valid();
}
void fat32::dlfilenode(file_node* fd)
{
	fd->prune_refs();
	std::map<uint64_t, size_t>::iterator i = __st_cluster_ref_counts.find(fd->cid());
	if(!i->second) { __release_clusters_from(static_cast<uint32_t>(i->first)); __st_cluster_ref_counts.erase(i); }
	__file_nodes.erase(*fd);
	syncdirs();
}
void fat32::dldirnode(directory_node* dd)
{    
	if(!dd->is_empty()) { throw std::logic_error(std::string("cannot delete directory ") + dd->name() + " because it is not empty"); }
	dd->prune_refs();
	std::map<uint64_t, size_t>::iterator i = __st_cluster_ref_counts.find(dd->cid());
	if(!i->second) { __release_clusters_from(static_cast<uint32_t>(i->first)); __st_cluster_ref_counts.erase(i); }
	__directory_nodes.erase(*dd);
	syncdirs();
}
file_node* fat32::mkfilenode(directory_node* parent, std::string const& name)
{
	dword cl = claim_cluster(__the_table);
	if(!cl) return nullptr;
	std::string sfname{};
	fat32_directory_node& fparent = dynamic_cast<fat32_directory_node&>(*parent);
	fparent.get_short_name(name, sfname);
	std::vector<fat32_directory_entry>::iterator avail = fparent.first_unused_entry();
	rtc_time t = rtc::get_instance().get_time();
	new(std::addressof(avail->regular_entry)) fat32_regular_entry
	{
		.created_time       { static_cast<uint8_t>(t.sec >> 1), t.min, t.hr },
		.created_date       { t.day, t.month, static_cast<uint8_t>(t.year - fat_year_base) },
		.first_cluster_hi   { cl.hi },
		.modified_time      { static_cast<uint8_t>(t.sec >> 1), t.min, t.hr },
		.modified_date      { t.day, t.month, static_cast<uint8_t>(t.year - fat_year_base) },
		.first_cluster_lo   { cl.lo }
	};
	set_filename(avail->regular_entry.filename, sfname);
	size_t idx              = static_cast<size_t>(avail - fparent.__my_dir_data.begin());
	fat32_file_node* result = put_file_node(name, std::addressof(fparent), cl, idx);
	add_start_cluster_ref(result->start_cluster());
	return result;
}
directory_node* fat32::mkdirnode(directory_node* parent, std::string const& name)
{
	dword cl = claim_cluster(__the_table);
	if(!cl) return nullptr;
	fat32_directory_node& fparent                       = dynamic_cast<fat32_directory_node&>(*parent);
	std::vector<fat32_directory_entry>::iterator avail  = fparent.first_unused_entry();
	std::string sfname{};
	fparent.get_short_name(name, sfname);
	rtc_time t = rtc::get_instance().get_time();
	new(std::addressof(avail->regular_entry)) fat32_regular_entry
	{
		.attributes         { 0x10UC },
		.created_time       { static_cast<uint8_t>(t.sec >> 1), t.min, t.hr },
		.created_date       { t.day, t.month, static_cast<uint8_t>(t.year - fat_year_base) },
		.first_cluster_hi   { cl.hi },
		.modified_time      { static_cast<uint8_t>(t.sec >> 1), t.min, t.hr },
		.modified_date      { t.day, t.month, static_cast<uint8_t>(t.year - fat_year_base) },
		.first_cluster_lo   { cl.lo }
	};
	set_filename(avail->regular_entry.filename, sfname);
	size_t idx                      = static_cast<size_t>(avail - fparent.__my_dir_data.begin());
	fat32_directory_node* result    = this->put_directory_node(name, std::addressof(fparent), cl, idx);
	add_start_cluster_ref(result->start_cluster());
	return result;
}
bool fat32::init_instance(block_device* dev)
{
	if(__has_init) return true;
	uint64_t ss = 2048UL;
	if(partitioned_block_device* pdev = dynamic_cast<partitioned_block_device*>(dev))
	{
		partition_table& pt = pdev->get_partition_table();
		// TODO: check the label to make sure this is in fact a FAT32 filesystem
		if(!pt.empty())
			ss = pt.front().start_lba;
	}
	if(__unlikely(!dev->read(std::addressof(bootsect), ss, div_round_up(sizeof(fat32_bootsect), dev->sector_size())))) return false;
	__instance = new(driver_space) fat32(bootsect.root_cluster_num, bootsect.sectors_per_cluster, bootsect.bytes_per_sector, ss + bootsect.num_reserved_sectors, bootsect.num_fats * bootsect.fat_size, bootsect.volume_serial);
	__instance->tie_block_device(dev);
	return (__has_init = __instance->init());
}
void fat32::on_close(file_node* f)
{
	filesystem::on_close(f);
	if(fat32_file_node* ff = dynamic_cast<fat32_file_node*>(f))
		ff->on_close();
}