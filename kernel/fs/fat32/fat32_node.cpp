#include "fs/fat32.hpp"
#include "fs/hda_ahci.hpp"
#include "rtc.h"
constexpr static char16_t get_longentry_char(std::string const& str, size_t i) { return (i > str.size()) ? char16_t(0xFFFF) : (i == str.size()) ? 0 : static_cast<char16_t>(str[i]); }
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
static std::vector<fat32_longname_entry> gen_longname_entries(std::string const& name)
{
    size_t delta = (name.size() % 13 == 0) ? 13 : 12;
    size_t n = div_roundup(name.size(), delta);
    std::vector<fat32_longname_entry> result{ n };
    for(std::string::const_iterator i = name.begin(), j = name.begin() + delta; i < name.end(); i = j, j += delta)
    {
        if(j > name.end()) j = name.end();
        result.push_back(from_string(std::string{ i, j }));
        result.back().ordinal = static_cast<uint8_t>(result.size());
    }
    result.back().ordinal |= 0x40ui8;
    return result.reverse();
}
static std::string parse_filename(fat32_regular_entry const& e) 
{ 
    std::string result{}; 
    bool dotted = false; 
    size_t c_spaces = 0;
    for(int i = 0; i < 8; i++) { if(e.filename[i] == ' ') c_spaces++; else if(e.filename[i]) { result.append(std::string(c_spaces, ' ')); result.append(e.filename[i]); c_spaces = 0; } }
    c_spaces = 0;
    for(int i = 8; i < 11; i++) 
    {
        if(e.filename[i] == ' ') c_spaces++;
        else if(e.filename[i]) 
        {
            result.append(std::string(c_spaces, ' ')); 
            if(!dotted) { result.append('.'); dotted = true; }
            result.append(e.filename[i]); 
            c_spaces = 0;
        } 
    } 
    return result; 
}
static inline void set_filename(fat32_regular_entry& e, std::string const& sname)
{
    size_t pos_dot = sname.find('.'), l = std::min(8UL, sname.size());
    if(pos_dot != std::string::npos && pos_dot < l) l = pos_dot;
    std::string clipped = sname.without('.');
    std::string::iterator i = clipped.begin();
    for(size_t j = 0; j < 8; j++) { if(j < l) { e.filename[j] = *i; i++; } else { e.filename[j] = ' '; } }
    for(size_t j = 8; j < 11; j++, i++) { e.filename[j] = (i < clipped.end()) ? *i : ' '; }
}
constexpr static char trim_char16(char16_t c) { return c > 0 && c < char16_t(0x100) ? static_cast<char>(c) : '_'; }
constexpr static bool charck(char16_t c) { return c != static_cast<char16_t>(0xFFFF) && c != u'\0'; }
constexpr static void append_longname_entry(std::string& str, fat32_longname_entry const& e) { char tmp[13]; size_t i = 0; for(size_t j = 0; j < 5 && charck(e.text_1[j]); i++, j++) { tmp[i] = trim_char16(e.text_1[j]); } for(size_t j = 0; j < 6 && charck(e.text_2[j]); i++, j++) { tmp[i] = trim_char16(e.text_1[j]); } for(size_t j = 0; j < 2 && charck(e.text_3[j]); i++, j++) { tmp[i] = trim_char16(e.text_1[j]); } str.append(std::string(tmp, i)); }
static std::string parse_longnames(std::set<fat32_longname_entry> const& entries) { std::string result{}; for(std::set<fat32_longname_entry>::const_iterator i = entries.begin(); i != entries.end(); i++) { append_longname_entry(result, *i); } return result; }
static std::vector<uint32_t> get_clusters_from(fat32_allocation_table const& tb, uint32_t start) { std::vector<uint32_t> result{}; fat32_allocation_table::const_iterator i = tb.from(start); do{ result.push_back(i.offs()); ++i; } while(i != tb.end()); return result; }
static dword start_of(fat32_regular_entry const& e) { return dword{ e.first_cluster_lo, e.first_cluster_hi }; }
static uint32_t claim_sector(fat32_allocation_table& tb, uint32_t last_sect) { fat32_allocation_table::iterator i = tb.next_available(); if(i == tb.end()) return 0; if(last_sect) { tb[last_sect] = (tb[last_sect] & fat32_cluster_pres) | (i.offs() & fat32_cluster_mask); } *i |= fat32_cluster_mask; return i.offs(); }
fat32_file_inode::fat32_file_inode(int fd, std::string const& real_name, fat32_regular_entry const &e, fat32_allocation_table &tb, fat32_filebuf::__cl_conv_fn_t const &cluster_to_sector) : file_inode{ real_name, fd, uint64_t(start_of(e))}, __my_filebuf{ get_clusters_from(tb, start_of(e)), cluster_to_sector, [&](uint32_t cl) -> uint32_t { return claim_sector(tb, cl); } }, __on_disk_size{ e.size_bytes } { create_time = e.created_date + e.created_time; modif_time = e.modified_date + e.modified_time; }
fat32_file_inode::fat32_file_inode(int fd, std::string const &real_name, uint32_t cluster_start, fat32_allocation_table &tb, fat32_filebuf::__cl_conv_fn_t const &cluster_to_sector) : file_inode{ real_name, fd, uint64_t(cluster_start) }, __my_filebuf{ std::vector<uint32_t>(1, cluster_start, std::allocator<uint32_t>{}), cluster_to_sector, [&](uint32_t cl) -> uint32_t { return claim_sector(tb, cl); } }, __on_disk_size{ 0UL } {}
uint64_t fat32_file_inode::size() const noexcept { return __on_disk_size; }
bool fat32_file_inode::fsync() { return __my_filebuf.__ddwrite() == 0; }
fat32_file_inode::pos_type fat32_file_inode::seek(pos_type pos) { return __my_filebuf.pubseekpos(pos); }
fat32_file_inode::pos_type fat32_file_inode::seek(off_type off, std::ios_base::seekdir way) { return __my_filebuf.pubseekoff(off, way); }
fat32_file_inode::size_type fat32_file_inode::read(pointer dest, size_type n) { return __my_filebuf.sgetn(dest, n); }
fat32_file_inode::size_type fat32_file_inode::write(const_pointer src, size_type n) { size_t result = __my_filebuf.sputn(src, n); this->__on_disk_size += result; return result; }
constexpr size_t s = sizeof(fat32_directory_entry);
void fat32_folder_inode::__expand_dir()
{
    size_t n =  __my_parent_fs->__sectors_per_cluster * (physical_block_size / s);
    if(uint32_t i = claim_sector(__my_parent_fs->__the_table, __my_covered_clusters.back())) { __my_covered_clusters.push_back(i); __my_dir_data.reserve(__my_dir_data.size() + n); }
    else throw std::runtime_error{ "out of space" };
}
std::vector<fat32_directory_entry>::iterator fat32_folder_inode::__first_open_entry()
{
    for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++) { if(i->regular_entry.filename[0] == 0xE5 || i->regular_entry.filename[0] == 0) return i; }
    this->__expand_dir();
    return __first_open_entry();
}
bool fat32_folder_inode::parse_dir_data()
{
    if(__has_init) return true;
    size_t n = __my_covered_clusters.size() * __my_parent_fs->__sectors_per_cluster * physical_block_size;
    __my_dir_data.reserve(n / s);
    if(ahci_hda::is_initialized() && ahci_hda::read(reinterpret_cast<char*>(__my_dir_data.data()), __my_parent_fs->__cluster_to_sector(__my_covered_clusters.front()), n))
    {
        std::set<fat32_longname_entry> name_entries{};
        for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++)
        {
            if(is_longname(*i)) name_entries.insert(i->longname_entry);
            else
            {
                if(name_entries.size())
                {
                    std::string ln{ parse_longnames(name_entries) };
                    __my_names.insert_or_assign(ln, std::addressof(i->regular_entry));
                    __my_directory.emplace(ln);
                }
                std::string sn{ parse_filename(i->regular_entry) };
                __my_names.insert_or_assign(sn, std::addressof(i->regular_entry));
                __my_directory.emplace(sn);
            }
        }
        return true;
    }
    else return false;
}
fat32_regular_entry *fat32_folder_inode::find_dirent(std::string const& name) { if(std::map<std::string, fat32_regular_entry*>::iterator i = __my_names.find(name); i != __my_names.end()) return i->second; else return nullptr; }
uint64_t fat32_folder_inode::num_files() const noexcept { return __n_files; }
uint64_t fat32_folder_inode::num_folders() const noexcept { return __n_folders; }
fat32_folder_inode::fat32_folder_inode(fat32* parent, std::string const& real_name, fat32_regular_entry const& e) : 
    folder_inode            { real_name, start_of(e) },
    __my_parent_fs          { parent },
    __my_dir_data           {},
    __my_covered_clusters   { get_clusters_from(parent->__the_table, start_of(e)) },
    __my_directory          {},
    __my_names              {},
    __n_files               { 0UL },
    __n_folders             { 0UL } 
                            { __has_init = parse_dir_data(); create_time = e.created_date + e.created_time; modif_time = e.modified_date + e.modified_time; }
tnode *fat32_folder_inode::find(std::string const& name)
{
    if(tnode_dir::iterator i = __my_directory.find(name); i != __my_directory.end()) 
    {
        if(i->operator bool()) return std::addressof(*i);
        else if(fat32_regular_entry* e = find_dirent(name))
        {
            if(e->attributes.directory) { i->assign(std::addressof(*__my_parent_fs->__opened_folders.emplace(this->__my_parent_fs, name, *e).first)); }
            else { i->assign(std::addressof(*__my_parent_fs->__opened_files.emplace(__my_parent_fs->next_fd, name, *e, __my_parent_fs->__the_table, [&](uint32_t cl) -> uint64_t { return __my_parent_fs->__cluster_to_sector(cl); }).first)); }
            return std::addressof(*i);
        }
        else return nullptr;
    }
    else return nullptr;
}
std::string fat32_folder_inode::get_short_name(std::string const &full)
{
    std::string upper = std::ext::to_upper(full);
    if(upper.size() < 13 && upper.contains('.') && upper.find('.', upper.begin()) + 3 >= upper.end()) return upper;
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
tnode *fat32_folder_inode::add(inode *n)
{
    dword cluster = claim_sector(__my_parent_fs->__the_table, 0);
    if(!cluster) return nullptr;
    std::vector<fat32_directory_entry>::iterator i = __first_open_entry();
    std::vector<fat32_longname_entry> ln = gen_longname_entries(n->name());
    for(fat32_longname_entry const& e : ln) { i->longname_entry = e; i++; }
    set_filename(i->regular_entry, get_short_name(n->name()));
    i->regular_entry.attributes.directory = n->is_folder();
    i->regular_entry.attributes.read_only = !n->mode.write_others && !n->mode.write_group && !n->mode.write_owner;
    i->regular_entry.attributes.hidden = !n->mode.read_others && !n->mode.read_group && !n->mode.read_owner;
    i->regular_entry.attributes.system = n->mode.exec_others || n->mode.exec_group || n->mode.exec_owner;
    i->regular_entry.first_cluster_hi = cluster.hi;
    i->regular_entry.first_cluster_lo = cluster.lo;
    rtc_time t = rtc_driver::get_instance().get_time();
    i->regular_entry.created_date.day = t.day;
    i->regular_entry.created_date.month = t.month;
    i->regular_entry.created_date.year = t.year - fat_year_base;
    i->regular_entry.created_time.half_seconds = t.sec >> 1;
    i->regular_entry.created_time.minutes = t.min;
    i->regular_entry.created_time.hours = t.hr;
    i->regular_entry.accessed_date = i->regular_entry.modified_date = i->regular_entry.created_date;
    i->regular_entry.modified_time = i->regular_entry.created_time;
    i->regular_entry.winnt_reserved = 0;
    i->regular_entry.size_bytes = 0;
    if(n->is_folder()) this->__n_folders++;
    else this->__n_files++;
    return std::addressof(*__my_directory.emplace(n, n->name()).first);
}