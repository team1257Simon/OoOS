#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
#include "rtc.h"
constexpr size_t dirent_size = sizeof(fat32_directory_entry);
static std::vector<uint32_t> get_clusters_from(fat32_allocation_table const& tb, uint32_t start) 
{ 
    std::vector<uint32_t> result{}; 
    fat32_allocation_table::const_iterator i = tb.from(start); 
    do { result.push_back(i.offs()); ++i; } while(i != tb.end()); 
    return result; 
}
static void parse_longnames(std::set<fat32_longname_entry> const& entries, std::string& result) 
{ 
    for(std::set<fat32_longname_entry>::const_iterator i = entries.begin(); i != entries.end(); i++) 
    {
        char tmp[13]; 
        size_t j = 0; 
        for(size_t k = 0; k < 5 && (i->text_1[k] != static_cast<char16_t>(0xFFFF) && i->text_1[k] != u'\0'); j++, k++) { tmp[j] = i->text_1[k] > 0 && i->text_1[k] < char16_t(0x100) ? static_cast<char>(i->text_1[k]) : '_'; }
        for(size_t k = 0; k < 6 && (i->text_2[k] != static_cast<char16_t>(0xFFFF) && i->text_2[k] != u'\0'); j++, k++) { tmp[j] = i->text_2[k] > 0 && i->text_2[k] < char16_t(0x100) ? static_cast<char>(i->text_2[k]) : '_'; } 
        for(size_t k = 0; k < 2 && (i->text_3[k] != static_cast<char16_t>(0xFFFF) && i->text_3[k] != u'\0'); j++, k++) { tmp[j] = i->text_3[k] > 0 && i->text_3[k] < char16_t(0x100) ? static_cast<char>(i->text_3[k]) : '_'; } 
        result.append(std::string(tmp, j)); 
    }
    result.shrink_to_fit(); 
}
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
    for(size_t j = 0; j < 5; j++, i++) { result.text_1[j] = i < str.size() ? str.at(i) : (i > str.size() ? 0xFFFF : 0); }
    for(size_t j = 0; j < 6; j++, i++) { result.text_2[j] = i < str.size() ? str.at(i) : (i > str.size() ? 0xFFFF : 0); }
    for(size_t j = 0; j < 2; j++, i++) { result.text_3[j] = i < str.size() ? str.at(i) : (i > str.size() ? 0xFFFF : 0); }
    return result;
}
static std::vector<fat32_longname_entry> gen_longname_entries(std::string const& name)
{
    size_t delta = (name.size() % 13 == 0) ? 13 : 12;
    std::vector<fat32_longname_entry> result{};
    for(std::string::const_iterator i = name.begin(), j = name.begin() + delta; i < name.end(); i = j, j += delta)
    {
        if(j > name.end()) j = name.end();
        result.push_back(from_string(std::string(i, j, std::allocator<char>())));
        uint8_t o = static_cast<uint8_t>(result.size());
        if(j == name.end()) o |= 0x40;
        result.back().ordinal = o;
    }
    return std::vector<fat32_longname_entry>(result.rend(), result.rbegin());
}
static char shortname_buffer[13]{};
static std::string parse_filename(fat32_regular_entry const& e)
{
    bool dotted = false; 
    size_t c_spaces = 0;
    size_t n = 0;
    array_zero(shortname_buffer, 12);
    for(int i = 0; i < 8; i++) { if(e.filename[i] == ' ') c_spaces++; else if(e.filename[i]) { for(size_t j = 0; j < c_spaces; j++) { shortname_buffer[n++] = ' '; } shortname_buffer[n++] = e.filename[i]; c_spaces = 0; } }
    c_spaces = 0;
    for(size_t i = 8; i < 11; i++) { if(e.filename[i] == ' ') c_spaces++; else if(e.filename[i]) { for(size_t j = 0; j < c_spaces; j++) { shortname_buffer[n++] = ' '; } if(!dotted){ shortname_buffer[n++] = '.'; dotted = true; } shortname_buffer[n++] = e.filename[i]; c_spaces = 0; } }
    return std::string(shortname_buffer, n);
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
fat32_node::fat32_node(fat32 *pfs, fat32_folder_inode *pdir, size_t didx) noexcept : parent_fs{ pfs }, parent_dir{ pdir }, dirent_index{ didx } {}
fat32_regular_entry *fat32_node::disk_entry() noexcept { return &((parent_dir->__my_dir_data.begin() + dirent_index)->regular_entry); }
fat32_regular_entry const *fat32_node::disk_entry() const noexcept { return &((parent_dir->__my_dir_data.begin() + dirent_index)->regular_entry); }
uint32_t fat32_node::start_cluster() const noexcept { return start_of(*disk_entry()); }
uint32_t fat32_file_inode::claim_next(uint32_t cl) { return claim_cluster(parent_fs->__the_table, cl); }
uint64_t fat32_file_inode::cl_to_s(uint32_t cl) { return parent_fs->cluster_to_sector(cl); }
void fat32_file_inode::on_open() { if(ahci_hda::is_initialized() && __on_disk_size) { __my_filebuf.__ddread(__on_disk_size); } if(!__on_disk_size) { __my_filebuf.__grow_buffer(physical_block_size); } }
fat32_file_inode::fat32_file_inode(fat32* pfs, std::string const& real_name, fat32_folder_inode* pdir, uint32_t cl_st, size_t dirent_idx) : file_inode{ real_name, 0, uint64_t(cl_st) }, fat32_node{ pfs, pdir, dirent_idx }, __my_filebuf{ std::move(get_clusters_from(pfs->__the_table, cl_st)), this },  __on_disk_size{ disk_entry()->size_bytes } { fat32_regular_entry* e = disk_entry(); create_time = e->created_date + e->created_time; modif_time = e->modified_date + e->modified_time; }
uint64_t fat32_file_inode::size() const noexcept { return __on_disk_size; }
void fat32_file_inode::set_fd(int i) { this->fd = i; }
bool fat32_file_inode::fsync() { if(__my_filebuf.sync() == 0) { update_times(*disk_entry()); disk_entry()->size_bytes = size(); return true; } else { panic("fsync failed"); return false; } }
fat32_file_inode::pos_type fat32_file_inode::tell() const { return pos_type(__my_filebuf.tell()); }
fat32_file_inode::pos_type fat32_file_inode::seek(pos_type pos) { return __my_filebuf.seekpos(pos); }
fat32_file_inode::pos_type fat32_file_inode::seek(off_type off, std::ios_base::seekdir way) { return __my_filebuf.seekoff(off, way); }
fat32_file_inode::size_type fat32_file_inode::read(pointer dest, size_type n) { return __my_filebuf.xsgetn(dest, n); }
fat32_file_inode::size_type fat32_file_inode::write(const_pointer src, size_type n) { size_t result = __my_filebuf.xsputn(src, n); this->__on_disk_size += result; if(!fsync()) return 0; return result; }
fat32_regular_entry *fat32_folder_inode::find_dirent(std::string const& name) { if(tnode_dir::iterator i = __my_directory.find(name); i != __my_directory.end()) { if(fat32_node* n = dynamic_cast<fat32_node*>(i->ptr()); n && n->parent_dir) return n->disk_entry(); } return nullptr; }
uint64_t fat32_folder_inode::num_files() const noexcept { return __n_files; }
uint64_t fat32_folder_inode::num_folders() const noexcept { return __n_folders; }
std::vector<std::string> fat32_folder_inode::lsdir() const { std::vector<std::string> result{}; for(tnode_dir::const_iterator i = __my_directory.begin(); i != __my_directory.end(); i++) { result.emplace_back(i->name()); } return result; }
fat32_folder_inode::fat32_folder_inode(fat32* pfs, std::string const& real_name, fat32_folder_inode* pdir, uint32_t cl_st, size_t dirent_idx) : folder_inode{ real_name, cl_st }, fat32_node{ pfs, pdir, dirent_idx }, __my_directory{}, __my_dir_data{}, __my_covered_clusters{ get_clusters_from(pfs->__the_table, cl_st) }, __n_files{ 0UL }, __n_folders{ 0UL } { if(pdir) {  fat32_regular_entry* e = disk_entry(); create_time = e->created_date + e->created_time; modif_time = e->modified_date + e->modified_time; } }
std::vector<fat32_directory_entry>::iterator fat32_folder_inode::__whereis(fat32_regular_entry* e) { std::vector<fat32_directory_entry>::iterator i{ reinterpret_cast<fat32_directory_entry*>(e) }; if(i < __my_dir_data.end() && i >= __my_dir_data.begin()) return i; else return __my_dir_data.end(); }
void fat32_folder_inode::__expand_dir() { size_t n =  parent_fs->__sectors_per_cluster * (physical_block_size / dirent_size); if(uint32_t i = claim_cluster(parent_fs->__the_table, __my_covered_clusters.back())) { __my_covered_clusters.push_back(i); __my_dir_data.reserve(__my_dir_data.size() + n); } else throw std::runtime_error{ "out of space" }; }
std::vector<fat32_directory_entry>::iterator fat32_folder_inode::first_unused_entry() { for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++) { if(i->regular_entry.filename[0] == 0xE5 || i->regular_entry.filename[0] == 0) return i; } this->__expand_dir(); return first_unused_entry(); }
tnode* fat32_folder_inode::find(std::string const& name) { if(tnode_dir::iterator i = __my_directory.find(name); i != __my_directory.end()) return i.base(); else return nullptr; }
std::vector<fat32_directory_entry>::iterator fat32_folder_inode::__get_longname_start(fat32_regular_entry* e) { std::vector<fat32_directory_entry>::iterator i = __whereis(e), j = i - 1; if(!is_longname(*j)) { return __my_dir_data.end(); } while(!is_last_longname(j->longname_entry) && j > this->__my_dir_data.begin()) { j--; } return is_longname(*j) && is_last_longname(j->longname_entry) ? j : __my_dir_data.end(); }
bool fat32_folder_inode::__dir_ent_erase(std::string const& what) 
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
            return true; 
        } 
        catch(std::exception& ex) { panic(ex.what()); } 
    } 
    return false; 
}
bool fat32_folder_inode::__read_disk_data()
{
    try
    {
        size_t epc = (parent_fs->__sectors_per_cluster * physical_block_size) / dirent_size;
        size_t n = __my_covered_clusters.size() * epc;
        __my_dir_data.reserve(n);
        __my_dir_data.expand_to_size();
        std::vector<uint32_t>::iterator cl = __my_covered_clusters.begin();
        for(size_t i = 0; i < n && cl != __my_covered_clusters.end(); cl++, i += epc) { asm volatile("mfence" ::: "memory"); if(!ahci_hda::read(reinterpret_cast<char*>((__my_dir_data.begin() + i).base()), parent_fs->cluster_to_sector(*cl), 1)) return false; }
        return true;
    }
    catch(std::exception& e) { panic("Read failed: "); panic(e.what()); }
    return false;
}
void fat32_folder_inode::__add_parsed_entry(fat32_regular_entry const& e, size_t j)
{
    std::string name = parse_filename(e);
    uint32_t cl = start_of(e);
    asm volatile("mfence" ::: "memory");
    if(e.attributes & 0x10)
    {
        fat32_folder_inode* n = parent_fs->put_folder_node(name, this, cl, j);
        if(!n) throw std::runtime_error{ "failed to create folder node " + name };
        asm volatile("mfence" ::: "memory");
        __my_directory.emplace(n, n->name());
        asm volatile("mfence" ::: "memory");
        if(!n->parse_dir_data()) throw std::runtime_error{ "parse failed on folder " + name };
        __n_folders++;
    }
    else
    {
        fat32_file_inode* n = parent_fs->put_file_node(name, this, cl, j);
        asm volatile("mfence" ::: "memory");
        if(!n) throw std::runtime_error{ "failed to create file node " + name };
        __my_directory.emplace(n, n->name());
        asm volatile("mfence" ::: "memory");
        __n_files++;
    }
}
bool fat32_folder_inode::parse_dir_data()
{
    if(__has_init) return true;
    try
    {
        size_t n = __my_covered_clusters.size() * parent_fs->__sectors_per_cluster * physical_block_size;
        __my_dir_data.reserve(n / dirent_size);
        __my_dir_data.expand_to_size();        
        if(__read_disk_data())
        {
            asm volatile("mfence" ::: "memory");
            size_t j = 0;
            for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++, j++)
            {
                if(is_unused(*i) || is_longname(*i) || i->regular_entry.filename[0] == '.' || !start_of(i->regular_entry)) continue;
                __add_parsed_entry(i->regular_entry, j);
                asm volatile("mfence" ::: "memory");
            }
            __has_init = true;
            return true;
        }
    }
    catch(std::exception& e) { panic("Parse failed: "); panic(e.what()); }
    return false;
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
    } while(this->__my_directory.contains(result) && i <= 999999);
    if(this->__my_directory.contains(result)) throw std::logic_error{ "could not get a unique short name from " + full };
    return result;
}
tnode* fat32_folder_inode::add(inode* n)
{
    if(!n->is_device())
    {
        dword cluster = dynamic_cast<fat32_node&>(*n).start_cluster();
        if(!cluster) return nullptr;
        fat32_regular_entry* e = dynamic_cast<fat32_node&>(*n).disk_entry();
        std::vector<fat32_directory_entry>::iterator i = __whereis(e);
        std::vector<fat32_longname_entry> ln = gen_longname_entries(n->name());
        for(fat32_longname_entry const& l : ln) { i->longname_entry = l; i++; }
        dynamic_cast<fat32_node&>(*n).dirent_index = static_cast<size_t>(i - __my_dir_data.begin());
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
    }
    return __my_directory.emplace(n, n->name()).first.base();
}
bool fat32_folder_inode::fsync()
{
    if(!ahci_hda::is_initialized() || !this->parse_dir_data()) return false;
    if(parent_dir) update_times(*disk_entry());
    try
    {
        std::vector<fat32_directory_entry>::iterator i = this->__my_dir_data.begin();
        for(tnode_dir::iterator t = __my_directory.begin(); t != __my_directory.end(); t++) { if(!t->ptr()->fsync()) return false; }    
        for(std::vector<uint32_t>::iterator j = this->__my_covered_clusters.begin(); j != this->__my_covered_clusters.end() && i < this->__my_dir_data.end(); ++j, i += (physical_block_size / dirent_size)) { if(!ahci_hda::write(parent_fs->cluster_to_sector(*j), reinterpret_cast<char*>(i.base()), 1)) return false; /* TODO better error handling */ }
        return true;
    }
    catch(std::exception& e) { panic(e.what()); }
    return false;
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
            if(i == __my_directory.end() || !__my_directory.erase(i)) return false;
        }
        return __dir_ent_erase(name);
    }
    return false;
}
bool fat32_folder_inode::link(tnode* original, std::string const& target)
{
    if(__my_directory.contains(target)) return false;
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
            __my_directory.emplace(original->ptr(), target);
            parent_fs->add_start_cluster_ref(start_of(clone.regular_entry));
            return true;
        } 
        catch(std::exception& ex) { panic(ex.what()); }
    }
    return false;
}