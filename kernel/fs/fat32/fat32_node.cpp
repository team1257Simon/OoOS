#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
#include "rtc.h"
#include "stdexcept"
constexpr size_t dirent_size = sizeof(fat32_directory_entry);
static void update_times(fat32_regular_entry& e) { rtc_time t = rtc::get_instance().get_time(); new (std::addressof(e.modified_date)) fat_filedate{ t.day, t.month, static_cast<uint8_t>(t.year - fat_year_base) }; new (std::addressof(e.modified_time)) fat_filetime{ static_cast<uint8_t>(t.sec >> 1), t.min, t.hr }; new (std::addressof(e.accessed_date)) fat_filedate{ e.modified_date }; }
fat32_node::fat32_node(fat32* pfs, fat32_directory_node* pdir, size_t didx) noexcept : parent_fs{ pfs }, parent_dir{ pdir }, dirent_index{ didx } {}
fat32_regular_entry* fat32_node::disk_entry() noexcept { return &((parent_dir->__my_dir_data.begin() + dirent_index)->regular_entry); }
fat32_regular_entry const* fat32_node::disk_entry() const noexcept { return &((parent_dir->__my_dir_data.begin() + dirent_index)->regular_entry); }
uint32_t fat32_node::start_cluster() const noexcept { return start_of(*disk_entry()); }
uint32_t fat32_file_node::claim_next(uint32_t cl) { return claim_cluster(parent_fs->__the_table, cl); }
uint64_t fat32_file_node::cl_to_s(uint32_t cl) { return parent_fs->cluster_to_sector(cl); }
void fat32_file_node::on_open() { if(hda_ahci::is_initialized() && __on_disk_size) { __my_filebuf.read_dev(__on_disk_size); } if(!__on_disk_size) { __my_filebuf.__grow_buffer(physical_block_size); } }
fat32_file_node::fat32_file_node(fat32* pfs, std::string const& real_name, fat32_directory_node* pdir, uint32_t cl_st, size_t dirent_idx) : file_node{ real_name, pfs->next_fd++, uint64_t(cl_st) }, fat32_node{ pfs, pdir, dirent_idx }, __my_filebuf{ std::vector<uint32_t>{}, this }, __on_disk_size{ disk_entry()->size_bytes } { fat32_regular_entry* e = disk_entry(); create_time = e->created_date + e->created_time; modif_time = e->modified_date + e->modified_time; uint32_t cl = cl_st & fat32_cluster_mask; do { __my_filebuf.__my_clusters.push_back(cl); cl = parent_fs->__the_table[cl] & fat32_cluster_mask; } while(cl < fat32_cluster_eof); }
uint64_t fat32_file_node::size() const noexcept { return __on_disk_size; }
void fat32_file_node::set_fd(int i) { this->fd = i; }
bool fat32_file_node::fsync() { if(__my_filebuf.sync() == 0) { update_times(*disk_entry()); disk_entry()->size_bytes = size(); return true; } else { panic("fsync failed"); return false; } }
fat32_file_node::pos_type fat32_file_node::tell() const { return pos_type(__my_filebuf.tell()); }
fat32_file_node::pos_type fat32_file_node::seek(pos_type pos) { return __my_filebuf.seekpos(pos); }
fat32_file_node::pos_type fat32_file_node::seek(off_type off, std::ios_base::seekdir way) { return __my_filebuf.seekoff(off, way); }
fat32_file_node::size_type fat32_file_node::read(pointer dest, size_type n) { return __my_filebuf.xsgetn(dest, n); }
fat32_file_node::size_type fat32_file_node::write(const_pointer src, size_type n) { size_t result = __my_filebuf.xsputn(src, n); this->__my_filebuf.__cur()[0] = std::char_traits<char>::eof(); this->__my_filebuf.is_dirty = true; this->__on_disk_size = size_t(this->tell()); sys_time(std::addressof(this->modif_time)); parent_dir->mark_dirty(); return result; }
fat32_regular_entry* fat32_directory_node::find_dirent(std::string const& name) { if(tnode_dir::iterator i = __my_directory.find(name); i != __my_directory.end()) { if(fat32_node* n = dynamic_cast<fat32_node*>(i->ptr()); n && n->parent_dir) return n->disk_entry(); } return nullptr; }
uint64_t fat32_directory_node::num_files() const noexcept { return __n_files; }
uint64_t fat32_directory_node::num_subdirs() const noexcept { return __n_folders; }
std::vector<std::string> fat32_directory_node::lsdir() const { std::vector<std::string> result{}; for(tnode_dir::const_iterator i = __my_directory.begin(); i != __my_directory.end(); i++) { result.emplace_back(i->name()); } return result; }
fat32_directory_node::fat32_directory_node(fat32* pfs, std::string const& real_name, fat32_directory_node* pdir, uint32_t cl_st, size_t dirent_idx) : directory_node{ real_name, pfs->next_fd++, cl_st }, fat32_node{ pfs, pdir, dirent_idx }, __my_directory{}, __my_dir_data{}, __my_covered_clusters{} { if(pdir) { fat32_regular_entry* e = disk_entry(); create_time = e->created_date + e->created_time; modif_time = e->modified_date + e->modified_time; } uint32_t cl = cl_st; do { __my_covered_clusters.push_back(cl); cl = parent_fs->__the_table[cl] & fat32_cluster_mask; } while(cl < fat32_cluster_eof); }
std::vector<fat32_directory_entry>::iterator fat32_directory_node::__whereis(fat32_regular_entry* e) { std::vector<fat32_directory_entry>::iterator i{ reinterpret_cast<fat32_directory_entry*>(e) }; if(i < __my_dir_data.end() && i >= __my_dir_data.begin()) return i; else return __my_dir_data.end(); }
void fat32_directory_node::__expand_dir() { size_t n =  parent_fs->__sectors_per_cluster * (physical_block_size / dirent_size); if(uint32_t i = claim_cluster(parent_fs->__the_table, __my_covered_clusters.back())) { __my_covered_clusters.push_back(i); __my_dir_data.reserve(__my_dir_data.size() + n); } else throw std::runtime_error{ "out of space" }; }
std::vector<fat32_directory_entry>::iterator fat32_directory_node::first_unused_entry() { __dirty = true; for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++) { if(i->regular_entry.filename[0] == 0xE5 || i->regular_entry.filename[0] == 0) return i; } this->__expand_dir(); return first_unused_entry(); }
tnode* fat32_directory_node::find(std::string const& name) { if(tnode_dir::iterator i = __my_directory.find(name); i != __my_directory.end()) return i.base(); else return nullptr; }
std::vector<fat32_directory_entry>::iterator fat32_directory_node::__get_longname_start(fat32_regular_entry* e) { std::vector<fat32_directory_entry>::iterator i = __whereis(e), j = i - 1; if(!is_longname(*j)) { return __my_dir_data.end(); } while(!is_last_longname(j->longname_entry) && j > this->__my_dir_data.begin()) { j--; } return is_longname(*j) && is_last_longname(j->longname_entry) ? j : __my_dir_data.end(); }
tnode* fat32_directory_node::add(fs_node* n) { return __my_directory.emplace(n, n->name()).first.base(); }
bool fat32_directory_node::link(tnode* original, std::string const& target) { panic("fat32 does not support hard links"); return false; }
bool fat32_directory_node::__dir_ent_erase(std::string const& what) 
{ 
    if(fat32_regular_entry* e = find_dirent(what)) 
    {
        std::vector<fat32_directory_entry>::iterator i = __whereis(e), j = __get_longname_start(e); 
        if(i == __my_dir_data.end() || j == __my_dir_data.end()) { return false; }
        try 
        { 
            parent_fs->rm_start_cluster_ref(start_of(*e));
            size_t sz = __my_dir_data.capacity();
            size_t erased = std::distance(j, i + 1);
            std::vector<fat32_directory_entry>::iterator r = __my_dir_data.erase(j, i + 1);
            size_t n = static_cast<size_t>(r - __my_dir_data.begin());
            __my_dir_data.reserve(sz);
            for(tnode_dir::iterator tn = __my_directory.begin(); tn != __my_directory.end(); tn++) { if(fat32_node* fn = dynamic_cast<fat32_node*>(tn->ptr()); fn->dirent_index >= n) { fn->dirent_index -= erased; } }
            __dirty = true;
            return true; 
        } 
        catch(std::exception& ex) { panic(ex.what()); } 
    } 
    return false; 
}
bool fat32_directory_node::__read_disk_data()
{
    std::allocator<char> b_alloc{};
    size_t bpc = physical_block_size * parent_fs->__sectors_per_cluster;
    size_t total = bpc * __my_covered_clusters.size();
    char* buffer = b_alloc.allocate(total);
    bool success = false;
    try
    {
        for(size_t i = 0; i < __my_covered_clusters.size(); i++) { if(!parent_fs->read_clusters(buffer + i * bpc, __my_covered_clusters[i])) throw std::runtime_error{ "cluster " + std::to_string(__my_covered_clusters[i]) }; }
        char* end = buffer + total;
        __my_dir_data.push_back(reinterpret_cast<fat32_directory_entry*>(buffer), reinterpret_cast<fat32_directory_entry*>(end));
        success = true;
    }
    catch(std::exception& e) { panic("Read failed: "); panic(e.what()); }
    b_alloc.deallocate(buffer, total);
    return success;
}
void fat32_directory_node::__add_parsed_entry(fat32_regular_entry const& e, size_t j)
{
    bool dotted = false; 
    size_t c_spaces = 0;
    std::string name{};
    for(int i = 0; i < 8; i++) { if(e.filename[i] == ' ') c_spaces++; else if(e.filename[i]) { for(size_t k = 0; k < c_spaces; k++) { name.append(' '); } name.append(e.filename[i]); c_spaces = 0; } }
    c_spaces = 0;
    for(size_t i = 8; i < 11; i++) { if(e.filename[i] == ' ') c_spaces++; else if(e.filename[i]) { for(size_t k = 0; k < c_spaces; k++) { name.append(' '); } if(!dotted) { name.append('.'); dotted = true; } name.append(e.filename[i]); c_spaces = 0; } }
    uint32_t cl = start_of(e);
    if(e.attributes & 0x10)
    {
        fat32_directory_node* n = parent_fs->put_directory_node(name, this, cl, j);
        if(!n) throw std::runtime_error{ "failed to create directory node " + name };
        __my_directory.emplace(n, name);
        if(!n->parse_dir_data()) throw std::runtime_error{ "parse failed on directory " + name};
        __n_folders++;
    }
    else
    {
        fat32_file_node* n = parent_fs->put_file_node(name, this, cl, j);
        if(!n) throw std::runtime_error{ "failed to create file node " + name };
        __my_directory.emplace(n, name);
        __n_files++;
    }
}
bool fat32_directory_node::parse_dir_data()
{
    if(__has_init) return true;
    try
    {
        if(__read_disk_data())
        {
            size_t j = 0;
            for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++, j++) { if(!(is_unused(*i) || is_longname(*i) || i->regular_entry.filename[0] == '.' || !start_of(i->regular_entry))) __add_parsed_entry(i->regular_entry, j); }
            return (__has_init = true);
        }
    }
    catch(std::exception& e) { panic("Parse failed: "); panic(e.what()); }
    return false;
}
void fat32_directory_node::get_short_name(std::string const& full, std::string& result)
{
    std::string upper = std::ext::to_upper(full);
    if(upper.size() < 13) { result = std::string(upper.c_str(), std::min(12UL, upper.size())); return; }
    bool have_dot = upper.contains('.');
    std::string trimmed = upper.without_any_of(". ");
    unsigned i = 1;
    size_t j;
    do {
        std::string tail = "~" + std::to_string(i);
        j = std::min(static_cast<size_t>(6UL - tail.size()), trimmed.size());
        if(trimmed.size() > 1 && have_dot) tail.append('.');
        if(trimmed.size() >= 3) tail.append(trimmed[trimmed.size() - 3]);
        if(trimmed.size() >= 2) tail.append(trimmed[trimmed.size() - 2]);
        if(trimmed.size() >= 1) tail.append(trimmed.back());
        result = std::string(trimmed.c_str(), j);
        result.append(tail);
        i++;
    } while(__my_directory.contains(result) && i <= 999999);
    if(__my_directory.contains(result)) throw std::logic_error{ "could not get a unique short name from " + upper };
}
bool fat32_directory_node::fsync()
{
    if(!hda_ahci::is_initialized() || !parse_dir_data()) return false;
    if(!__dirty) return true;
    if(parent_dir && dirent_index) update_times(*disk_entry());
    try
    { 
        char const* pos = reinterpret_cast<char const*>(__my_dir_data.begin().base());
        for(size_t i = 0; i < __my_covered_clusters.size(); i++) { if(parent_fs->write_clusters(__my_covered_clusters[i], pos)) { pos += physical_block_size * parent_fs->__sectors_per_cluster; } else { panic("write failed"); return false; } }
        __dirty = false;
        return true;
    }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}
bool fat32_directory_node::unlink(std::string const& name)
{
    tnode_dir::iterator i = __my_directory.find(name);
    if(__builtin_expect(i == __my_directory.end(), false)) { panic("target does not exist"); return false; }
    return __dir_ent_erase(name);
}