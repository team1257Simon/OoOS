#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
#include "rtc.h"
constexpr size_t dirent_size = sizeof(fat32_directory_entry);
constexpr static char16_t get_longentry_char(std::string const& str, size_t i) { return i < str.size() ? str.at(i) : (i > str.size() ? 0xFFFF : 0); }
constexpr static char trim_char16(char16_t c) { return c > 0 && c < char16_t(0x100) ? static_cast<char>(c) : '_'; }
constexpr static bool charck(char16_t c) { return c != static_cast<char16_t>(0xFFFF) && c != u'\0'; }
static void append_longname_entry(std::string& str, fat32_longname_entry const& e) { char tmp[13]; size_t i = 0; for(size_t j = 0; j < 5 && charck(e.text_1[j]); i++, j++) { tmp[i] = trim_char16(e.text_1[j]); } for(size_t j = 0; j < 6 && charck(e.text_2[j]); i++, j++) { tmp[i] = trim_char16(e.text_1[j]); } for(size_t j = 0; j < 2 && charck(e.text_3[j]); i++, j++) { tmp[i] = trim_char16(e.text_1[j]); } str.append(std::string(tmp, i)); }
static void parse_longnames(std::set<fat32_longname_entry> const& entries, std::string& result) { for(std::set<fat32_longname_entry>::const_iterator i = entries.begin(); i != entries.end(); i++) { append_longname_entry(result, *i); } result.shrink_to_fit(); }
static std::vector<uint32_t> get_clusters_from(fat32_allocation_table const& tb, uint32_t start) { std::vector<uint32_t> result{}; fat32_allocation_table::const_iterator i = tb.from(start); do{ result.push_back(i.offs()); ++i; } while(i != tb.end()); return result; }
uint32_t claim_cluster(fat32_allocation_table& tb, uint32_t last_sect) { fat32_allocation_table::iterator i = tb.next_available(); if(i == tb.end()) return 0; if(last_sect) { tb[last_sect] = (tb[last_sect] & fat32_cluster_pres) | (i.offs() & fat32_cluster_mask); } *i |= fat32_cluster_mask; return i.offs(); }
static void init_times(fat32_regular_entry& e)
{
    rtc_time t = rtc_driver::get_instance().get_time();
    e.created_date.day = t.day;
    e.created_date.month = t.month;
    e.created_date.year = t.year - fat_year_base;
    e.created_time.half_seconds = t.sec >> 1;
    e.created_time.minutes = t.min;
    e.created_time.hours = t.hr;
    e.accessed_date = e.modified_date = e.created_date;
    e.modified_time = e.created_time;
}
static void update_times(fat32_regular_entry& e)
{
    rtc_time t = rtc_driver::get_instance().get_time();
    e.modified_date.day = t.day;
    e.modified_date.month = t.month;
    e.modified_date.year = t.year - fat_year_base;
    e.modified_time.half_seconds = t.sec >> 1;
    e.modified_time.minutes = t.min;
    e.modified_time.hours = t.hr;
    e.accessed_date = e.modified_date;
}
static fat32_longname_entry from_string(std::string const& str)
{
    fat32_longname_entry result{};
    result.attributes = 0x0Fui8;
    result.reserved = result.type = 0;
    size_t i = 0;
    for(size_t j = 0; j < 5; j++, i++) { result.text_1[j] = get_longentry_char(str, i); }
    for(size_t j = 0; j < 6; j++, i++) { result.text_2[j] = get_longentry_char(str, i); }
    for(size_t j = 0; j < 2; j++, i++) { result.text_3[j] = get_longentry_char(str, i); }
    return result;
}
constexpr auto x = alignof(fat32_longname_entry);
static std::vector<fat32_longname_entry> gen_longname_entries(std::string const& name)
{
    size_t delta = (name.size() % 13 == 0) ? 13 : 12;
    std::vector<fat32_longname_entry> result{};
    for(std::string::const_iterator i = name.begin(), j = name.begin() + delta; i < name.end(); i = j, j += delta)
    {
        if(j > name.end()) j = name.end();
        result.push_back(from_string(std::string(i, j, std::allocator<char>())));
        result.back().ordinal = static_cast<uint8_t>(result.size());
    }
    uint8_t o = result.back().ordinal;
    o |= 0x40;
    result.back().ordinal = o;
    return result.reverse();
}
static void parse_filename(fat32_regular_entry const& e, std::string& result) 
{ 
    bool dotted = false; 
    size_t c_spaces = 0;
    for(int i = 0; i < 8; i++) { if(e.filename[i] == ' ') c_spaces++; else if(e.filename[i]) { for(size_t j = 0; j < c_spaces; j++) result.append(' '); result.append(e.filename[i]); c_spaces = 0; } }
    c_spaces = 0;
    for(int i = 8; i < 11; i++) 
    {
        if(e.filename[i] == ' ') c_spaces++;
        else if(e.filename[i]) 
        {
            for(size_t j = 0; j < c_spaces; j++) result.append(' ');
            if(!dotted) { result.append('.'); dotted = true; }
            result.append(e.filename[i]); 
            c_spaces = 0;
        } 
    }
    result.shrink_to_fit();
}
static void set_filename(fat32_regular_entry& e, std::string const& sname)
{
    size_t pos_dot = sname.find('.'), l = std::min(8UL, sname.size());
    if(pos_dot != std::string::npos && pos_dot < l) l = pos_dot;
    std::string clipped = sname.without('.');
    std::string::iterator i = clipped.begin();
    for(size_t j = 0; j < 8; j++) { if(j < l) { e.filename[j] = *i; i++; } else { e.filename[j] = ' '; } }
    for(size_t j = 8; j < 11; j++, i++) { e.filename[j] = (i < clipped.end()) ? *i : ' '; }
}
void fat32_file_inode::on_open() { if(ahci_hda::is_initialized() && __on_disk_size) { __my_filebuf.__ddread(__on_disk_size); } if(!__on_disk_size) { __my_filebuf.__grow_buffer(physical_block_size); } }
fat32_file_inode::fat32_file_inode(fat32* parent, std::string const& real_name, fat32_regular_entry* e) : file_inode{ real_name, parent->get_next_fd()++, uint64_t(start_of(*e)) }, fat32_node{ e }, __my_filebuf{ get_clusters_from(parent->__the_table, start_of(*e)), parent->cl_to_sect_fn, [&](uint32_t cl) -> uint32_t { return claim_cluster(parent->__the_table, cl); } }, __on_disk_size{ e->size_bytes } {  __on_disk_size = e->size_bytes; create_time = e->created_date + e->created_time; modif_time = e->modified_date + e->modified_time; }
uint64_t fat32_file_inode::size() const noexcept { return __on_disk_size; }
bool fat32_file_inode::fsync() { update_times(*disk_entry); disk_entry->size_bytes = size(); return __my_filebuf.__ddwrite() == 0; }
fat32_file_inode::pos_type fat32_file_inode::seek(pos_type pos) { return __my_filebuf.pubseekpos(pos); }
fat32_file_inode::pos_type fat32_file_inode::seek(off_type off, std::ios_base::seekdir way) { return __my_filebuf.pubseekoff(off, way); }
fat32_file_inode::size_type fat32_file_inode::read(pointer dest, size_type n) { return __my_filebuf.sgetn(dest, n); }
fat32_file_inode::size_type fat32_file_inode::write(const_pointer src, size_type n) { size_t result = __my_filebuf.sputn(src, n); this->__on_disk_size += result; if(!fsync()) return 0; return result; }
fat32_regular_entry *fat32_folder_inode::find_dirent(std::string const& name) { if(std::map<std::string, fat32_regular_entry*>::iterator i = __my_names.find(name); i != __my_names.end()) return i->second; else return nullptr; }
uint64_t fat32_folder_inode::num_files() const noexcept { return __n_files; }
uint64_t fat32_folder_inode::num_folders() const noexcept { return __n_folders; }
std::vector<std::string> fat32_folder_inode::lsdir() const { std::vector<std::string> result{ __my_names.size() }; for(std::map<std::string, fat32_regular_entry*>::const_iterator i = __my_names.begin(); i != __my_names.end(); i++) { result.push_back(i->first); } return result; }
fat32_folder_inode::fat32_folder_inode(fat32* parent, std::string const& real_name, fat32_regular_entry* e) : folder_inode{ real_name, start_of(*e) }, fat32_node{ e }, __my_directory{}, __my_names{}, __my_parent_fs{ parent }, __my_dir_data{}, __my_covered_clusters{ get_clusters_from(parent->__the_table, start_of(*e)) }, __n_files{ 0UL }, __n_folders{ 0UL } { create_time = e->created_date + e->created_time; modif_time = e->modified_date + e->modified_time; }
fat32_folder_inode::fat32_folder_inode(fat32* parent, std::string const& real_name, uint32_t root_cl) : folder_inode{ real_name, root_cl }, fat32_node{ nullptr }, __my_directory{}, __my_names{}, __my_parent_fs{ parent }, __my_dir_data{}, __my_covered_clusters{ get_clusters_from(parent->__the_table, root_cl) }, __n_files{ 0UL }, __n_folders{ 0UL } {}
std::vector<fat32_directory_entry>::iterator fat32_folder_inode::__reclaim_stray(fat32_regular_entry *e) { for(std::vector<fat32_directory_entry>::iterator i = this->__my_dir_data.begin(); i != this->__my_dir_data.end(); i++) { if(!is_longname(*i) && !std::memcmp(i->regular_entry.filename, e->filename, 11)) return i; } return this->__my_dir_data.end(); }
std::vector<fat32_directory_entry>::iterator fat32_folder_inode::__whereis(fat32_regular_entry* e) { std::vector<fat32_directory_entry>::iterator i = this->__my_dir_data.begin() + (reinterpret_cast<fat32_directory_entry*>(e) - this->__my_dir_data.data()); if (i >= this->__my_dir_data.begin() && i < this->__my_dir_data.end()) { return i; } else return __reclaim_stray(e); }
void fat32_folder_inode::__expand_dir() { size_t n =  __my_parent_fs->__sectors_per_cluster * (physical_block_size / dirent_size); if(uint32_t i = claim_cluster(__my_parent_fs->__the_table, __my_covered_clusters.back())) { __my_covered_clusters.push_back(i); __my_dir_data.reserve(__my_dir_data.size() + n); __update_name_ptrs(); } else throw std::runtime_error{ "out of space" }; }
std::vector<fat32_directory_entry>::iterator fat32_folder_inode::first_unused_entry() { for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++) { if(i->regular_entry.filename[0] == 0xE5 || i->regular_entry.filename[0] == 0) return i; } this->__expand_dir(); return first_unused_entry(); }
std::vector<fat32_directory_entry>::iterator fat32_folder_inode::__get_longname_start(fat32_regular_entry* e) { std::vector<fat32_directory_entry>::iterator i = __whereis(e), j = i - 1; if(!is_longname(*j)) { return __my_dir_data.end(); } while(!is_last_longname(j->longname_entry) && j > this->__my_dir_data.begin()) { j--; } return is_longname(*j) && is_last_longname(j->longname_entry) ? j : __my_dir_data.end(); }
void fat32_folder_inode::__update_name_ptrs() { for(std::map<std::string, fat32_regular_entry*>::iterator name_it = __my_names.begin(); name_it != __my_names.end(); name_it++) { std::vector<fat32_directory_entry>::iterator newpos = __reclaim_stray(name_it->second); if(newpos != __my_dir_data.end()) name_it->second = std::addressof(newpos->regular_entry); else name_it = __my_names.erase(name_it); } }
bool fat32_folder_inode::__dir_ent_erase(std::string const& what) 
{ 
    if(fat32_regular_entry* e = find_dirent(what)) 
    {
        std::vector<fat32_directory_entry>::iterator i = __whereis(e), j = __get_longname_start(e); 
        if(i == __my_dir_data.end() || j == __my_dir_data.end()) { return false; }
        __my_names.erase(what);
        try 
        { 
            __my_parent_fs->rm_start_cluster_ref(start_of(*e));
            size_t sz = __my_dir_data.capacity();
            __my_dir_data.erase(j, i + 1); 
            __my_dir_data.reserve(sz);
            __update_name_ptrs();
            return true; 
        } 
        catch(std::exception& ex) { panic(ex.what()); } 
    } 
    return false; 
}
bool fat32_folder_inode::parse_dir_data()
{
    if(__has_init) return true;
    try
    {
        size_t n = __my_covered_clusters.size() * __my_parent_fs->__sectors_per_cluster * physical_block_size;
        __my_dir_data.reserve(n / dirent_size);
        if(ahci_hda::is_initialized() && ahci_hda::read(reinterpret_cast<char*>(__my_dir_data.data()), __my_parent_fs->cluster_to_sector(__my_covered_clusters.front()), n))
        {
            __my_dir_data.expand_to_size();
            std::set<fat32_longname_entry> name_entries{};
            for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++)
            {
                if(is_unused(*i)) continue;
                if(is_longname(*i)) name_entries.insert(i->longname_entry);
                else
                {
                    if(i->regular_entry.filename[0] == '.' || !start_of(i->regular_entry)) continue;
                    std::map<std::string, fat32_regular_entry*>::iterator name_iterator;
                    if(name_entries.size()) 
                    {
                        std::string ln(13 * name_entries.size()); 
                        parse_longnames(name_entries, ln); 
                        name_iterator = __my_names.emplace(ln, std::addressof(i->regular_entry)).first; 
                        name_entries.clear();
                    }
                    else { std::string sn(12, std::allocator<char>()); parse_filename(i->regular_entry, sn); name_iterator = __my_names.emplace(sn, std::addressof(i->regular_entry)).first; }
                    if(i->regular_entry.attributes & 0x10) { fat32_folder_inode* n = __my_parent_fs->put_folder_node(name_iterator->first, std::addressof(i->regular_entry)); if(!n || !n->parse_dir_data()) return false; __my_directory.emplace(n, n->name()).first; }
                    else { fat32_file_inode* n = __my_parent_fs->put_file_node(name_iterator->first, std::addressof(i->regular_entry)); if(!n) return false; __my_directory.emplace(n, n->name()).first; }
                }
            }
            __has_init = true;
            return true;
        }
    }
    catch(std::exception& e) { panic("Parse failed: "); panic(e.what()); }
    return false;
}
tnode* fat32_folder_inode::find(std::string const& name)
{
    if(tnode_dir::iterator i = __my_directory.find(name); i != __my_directory.end()) 
    {
        if(i->operator bool()) return i.base();
        else if(fat32_regular_entry* e = find_dirent(name))
        {
            fat32_file_inode* n = __my_parent_fs->put_file_node(name, e);
            if(n) __my_parent_fs->add_start_cluster_ref(n->cid());
            else return nullptr;
            i->assign(n); 
            return i.base();
        }
        else return nullptr;
    }
    else return nullptr;
}
std::string fat32_folder_inode::get_short_name(std::string const &full)
{
    std::string upper = std::ext::to_upper(full);
    if(upper.size() < 13) return upper;
    bool have_dot = upper.contains('.');
    std::string trimmed = upper.without_any_of(". ");
    unsigned i = 1;
    size_t j;
    std::string tail{}, result{};
    do
    {
        tail = "~" + std::to_string(i);
        j = std::min(static_cast<size_t>(6UL - tail.size()), trimmed.size());
        if(trimmed.size() > 1 && have_dot) tail.append('.');
        if(trimmed.size() >= 3) tail.append(trimmed[trimmed.size() - 3]);
        if(trimmed.size() >= 2) tail.append(trimmed[trimmed.size() - 2]);
        if(trimmed.size() >= 1) tail.append(trimmed.back());
        result = std::string(trimmed.begin(), trimmed.begin() + j) + tail;
        i++;
    } while(this->__my_names.contains(result) && i <= 999999);
    if(this->__my_names.contains(result)) throw std::logic_error{ "could not get a unique short name from " + full };
    return result;
}
tnode* fat32_folder_inode::add(inode* n)
{
    try
    {
        dword cluster = dynamic_cast<fat32_node&>(*n).start_cluster();
        if(!cluster) return nullptr;
        fat32_regular_entry* e = dynamic_cast<fat32_node&>(*n).disk_entry;
        std::vector<fat32_directory_entry>::iterator i = __whereis(e);
        std::vector<fat32_longname_entry> ln = gen_longname_entries(n->name());
        for(fat32_longname_entry const& l : ln) { i->longname_entry = l; i++; }
        e = std::addressof(i->regular_entry);
        dynamic_cast<fat32_node&>(*n).disk_entry = e;
        set_filename(*e, get_short_name(n->name()));
        bool directory = n->is_folder();
        bool read_only = !n->mode.write_others && !n->mode.write_group && !n->mode.write_owner;
        bool hidden = !n->mode.read_others && !n->mode.read_group && !n->mode.read_owner;
        bool system = n->mode.exec_others || n->mode.exec_group || n->mode.exec_owner;
        e->attributes = (directory ? 0x10 : 0) | (read_only ? 0x01 : 0) | (hidden ? 0x02 : 0) | (system ? 0x04 : 0);
        e->first_cluster_hi = cluster.hi;
        e->first_cluster_lo = cluster.lo;
        init_times(*e);
        e->winnt_reserved = 0;
        e->size_bytes = 0;
        if(n->is_folder()) this->__n_folders++;
        else this->__n_files++;
        if(!this->fsync()) return nullptr;
        this->__my_names.insert_or_assign(n->name(), e);
        return std::addressof(*__my_directory.emplace(n, n->name()).first);
    }
    catch(...) { return nullptr; } // std::bad_cast if someone hands us an inode for the wrong fs
}
bool fat32_folder_inode::fsync()
{
    if(!ahci_hda::is_initialized() || !this->parse_dir_data()) return false;
    if(disk_entry) update_times(*disk_entry);
    std::vector<fat32_directory_entry>::iterator i = this->__my_dir_data.begin();
    for(tnode_dir::iterator first = __my_directory.begin(), last = __my_directory.end(); first != last;) { if(!first->operator bool()) first = __my_directory.erase(first); else if(!(*first)->fsync()) return false; else ++first; }    
    for(std::vector<uint32_t>::iterator j = this->__my_covered_clusters.begin(); j != this->__my_covered_clusters.end() && i < this->__my_dir_data.end(); ++j, i += (physical_block_size / dirent_size)) { if(!ahci_hda::write(__my_parent_fs->cluster_to_sector(*j), reinterpret_cast<char*>(std::addressof(*i)), 1)) return false; /* TODO better error handling */ }
    return true;
}
bool fat32_folder_inode::unlink(std::string const& name)
{
    if(fat32_regular_entry* e = find_dirent(name))
    {
        tnode_dir::iterator i = __my_directory.find(name);
        if(__builtin_expect(i == __my_directory.end(), false))
        {
            std::vector<fat32_directory_entry>::iterator j = __whereis(e), k = __get_longname_start(e);
            if(j == __my_dir_data.end() || k == __my_dir_data.end()) return false;
            std::string ln(size_t(13UL * std::distance(k, j)), std::allocator<char>());
            parse_longnames(std::set<fat32_longname_entry>(std::addressof(k->longname_entry), std::addressof(j->longname_entry)), ln);
            i = __my_directory.find(ln);
            if(i == __my_directory.end()) return false;
        }
        return __dir_ent_erase(name);
    }
    return false;
}
bool fat32_folder_inode::link(tnode* original, std::string const& target)
{
    if(__my_directory.contains(target) || __my_names.contains(target)) return false;
    if(fat32_regular_entry* e = find_dirent(original->name()))
    {
        std::vector<fat32_directory_entry>::iterator i = __whereis(e);
        if(i == __my_dir_data.end()) return false;
        try
        {
            std::vector<fat32_directory_entry>::iterator next = first_unused_entry();
            fat32_directory_entry clone{};
            clone.regular_entry = i->regular_entry;
            init_times(clone.regular_entry);
            set_filename(clone.regular_entry, get_short_name(target));
            std::vector<fat32_longname_entry> ln = gen_longname_entries(target);
            for(std::vector<fat32_longname_entry>::iterator i = ln.begin(); i != ln.end(); i++, next = first_unused_entry()) { fat32_directory_entry e{}; e.longname_entry = *i; __my_dir_data.insert(next, e); }
            __my_dir_data.insert(first_unused_entry(), clone);
            if(!this->fsync()) return false;
            __my_directory.emplace(std::addressof(original->operator*()), target);
            __my_parent_fs->add_start_cluster_ref(start_of(clone.regular_entry));
            return true;
        } catch(std::exception& ex) { panic(ex.what()); }
    }
    return false;
}