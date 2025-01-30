#include "fs/fat32.hpp"
static std::vector<uint32_t> get_sectors_from(fat32_allocation_table const& tb, uint32_t start) { std::vector<uint32_t> result{}; fat32_allocation_table::const_iterator i = tb.from(start); do{ result.push_back(i.offs()); ++i; } while(i != tb.end()); return result; }
static dword start_sector_of(fat32_dir_entry const& e) { return dword{ e.first_cluster_lo, e.first_cluster_hi }; }
static uint32_t claim_sector(fat32_allocation_table& tb, uint32_t last_sect) { fat32_allocation_table::iterator i = tb.next_available(); if(i == tb.end()) return 0; tb[last_sect] = (tb[last_sect] & fat32_cluster_pres) | (i.offs() & fat32_cluster_mask); *i |= fat32_cluster_mask; return i.offs(); }
fat32_file_inode::fat32_file_inode(int fd, std::string const& real_name, fat32_dir_entry const &e, fat32_allocation_table &tb, fat32_filebuf::__cl_conv_fn_t const &cluster_to_sector) : file_inode{ real_name, fd, uint64_t(start_sector_of(e))}, __my_filebuf{ get_sectors_from(tb, start_sector_of(e)), cluster_to_sector, [&](uint32_t cl) -> uint32_t { return claim_sector(tb, cl); } }, __on_disk_size{ e.size_bytes } {}
uint64_t fat32_file_inode::size() const noexcept { return __on_disk_size; }
bool fat32_file_inode::fsync() { return __my_filebuf.__ddwrite() == 0; }
fat32_file_inode::pos_type fat32_file_inode::seek(pos_type pos) { return __my_filebuf.pubseekpos(pos); }
fat32_file_inode::pos_type fat32_file_inode::seek(off_type off, std::ios_base::seekdir way) { return __my_filebuf.pubseekoff(off, way); }
fat32_file_inode::size_type fat32_file_inode::read(pointer dest, size_type n) { return __my_filebuf.sgetn(dest, n); }
fat32_file_inode::size_type fat32_file_inode::write(const_pointer src, size_type n) { return __my_filebuf.sputn(src, n); }