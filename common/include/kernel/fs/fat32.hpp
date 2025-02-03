#ifndef __FS_FAT32
#define __FS_FAT32
#include "fs/fs.hpp"
#include "map"
#include "sys/time.h"
constexpr uint16_t fat_year_base = 1980u;
constexpr uint32_t fsinfo_magic = 0x41615252U;      // Alex was killed by magic
constexpr uint32_t fsinfo_more_magic = 0x61417272U; // Steve was killed by more magic
constexpr uint32_t fsinfo_tail = 0xAA550000;
constexpr uint32_t fat32_cluster_mask = 0x0FFFFFFF;
constexpr uint32_t fat32_cluster_pres = 0xF0000000;
constexpr uint32_t fat32_cluster_eof = 0x0FFFFFF8;
struct fat_filetime
{
    uint8_t half_seconds    : 5;
    uint8_t minutes         : 6;
    uint8_t hours           : 5;
    constexpr fat_filetime(fat_filetime const&) = default;
    constexpr fat_filetime(fat_filetime&&) = default;
    constexpr fat_filetime& operator=(fat_filetime const& that) noexcept { half_seconds = that.half_seconds; minutes = that.minutes; hours = that.hours; return *this; }
    constexpr fat_filetime(uint8_t hs, uint8_t m, uint8_t h) noexcept : half_seconds{ hs }, minutes{ m }, hours{ h } {}
    constexpr fat_filetime(uint16_t val) noexcept : fat_filetime{ static_cast<uint8_t>(val & 0x001F), static_cast<uint8_t>((val & 0x07E0) >> 5), static_cast<uint8_t>((val & 0xF800) >> 11)} {}
    constexpr fat_filetime() noexcept : fat_filetime{ 0U } {}
    constexpr operator time_t() const noexcept { return static_cast<time_t>(half_seconds) * 2000UL + static_cast<time_t>(minutes) * 60000UL + static_cast<time_t>(hours) * 3600000UL;  }
} __pack __align(1);
struct fat_filedate
{
    uint8_t day     : 5;
    uint8_t month   : 4;
    uint8_t year    : 7;
    constexpr fat_filedate(fat_filedate const&) = default;
    constexpr fat_filedate(fat_filedate&&) = default;
    constexpr fat_filedate& operator=(fat_filedate const& that) noexcept { day = that.day; month = that.month; year = that.year; return *this; }
    constexpr fat_filedate(uint8_t d, uint8_t m, uint8_t y) noexcept : day{ d }, month{ m }, year{ y } {}
    constexpr fat_filedate(uint16_t val) noexcept : fat_filedate{ static_cast<uint8_t>(val & 0x001F), static_cast<uint8_t>((val & 0x01E0) >> 5), static_cast<uint8_t>((val & 0xFE00) >> 9) } {}
    constexpr fat_filedate() noexcept : fat_filedate{ 0U } {}
    constexpr operator time_t() const noexcept{ return (static_cast<time_t>(day) + static_cast<time_t>(day_of_year(month, day, (year + fat_year_base) % 4 == 0)) + static_cast<time_t>(years_to_days(year + fat_year_base, unix_year_base))) * 86400000UL; }
} __pack __align(1);
struct fsinfo
{
    uint32_t magic; 
    uint8_t reserved_bytes[480];
    uint32_t more_magic;
    uint32_t last_free;
    uint32_t first_available;
    uint8_t more_reserved_bytes[12];
    uint32_t trail;
} __pack;
struct fat32_bootsect
{
    uint8_t bpb_boot_guard[3];      // 0xEB 0xNN 0x90 or 0xE9 0xNN 0x90 -> JMP 0xNN(%rip); NOP. The value NN will be the offset of some bootable code
    uint8_t oem_id[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint16_t num_root_dirents;
    uint16_t small_sector_count;    // set if the number of sectors is at most 0xFFFF (65535); otherwise 0 and the large field is set instead
    uint8_t media_descriptor_type;
    uint16_t unused;                // only for fat12/16
    uint16_t sectors_per_track;
    uint16_t media_shape;           // storage media heads/sides
    uint32_t sectors_hidden;
    uint32_t large_sector_count;    // set if the number of sectors is at least 0x10000 (65536)
    uint32_t fat_size;              // size of the FAT in sectors
    uint16_t flags;
    uint8_t vers_minor;
    uint8_t vers_major;
    uint32_t root_cluster_num;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_num;
    uint8_t winnt_flags;            // reserved
    uint8_t signature;
    uint32_t volume_serial;
    char volume_label[11];
    char sid_string[8];
    uint8_t boot_code[420];
    uint16_t boot_signature;        // 0xAA55
} __pack __align(1);
struct fat32_regular_entry
{
    char filename[11];
    uint8_t attributes;
    uint8_t winnt_reserved;
    uint8_t created_time_millis;    // in hundredths of a second, so this times 10 is the milliseconds field
    fat_filetime created_time;
    fat_filedate created_date;
    fat_filedate accessed_date;
    uint16_t first_cluster_hi;
    fat_filetime modified_time;
    fat_filedate modified_date;
    uint16_t first_cluster_lo;
    uint32_t size_bytes;
} __pack __align(1);
constexpr uint8_t sn_checksum(fat32_regular_entry const& e)
{
    uint8_t result = 0;
    for(uint8_t i = 0; i < 11; i++) { result = (result & 0x01 ? 0x80 : 0) + (result >> 1) + static_cast<uint8_t>(e.filename[i]); }
    return result;
}
struct fat32_longname_entry
{
    uint8_t ordinal;
    char16_t text_1[5];
    uint8_t attributes;     // 0x0F
    uint8_t type;
    uint8_t shortname_checksum;
    char16_t text_2[6];
    uint16_t reserved;
    char16_t text_3[2];
} __pack __align(1);
constexpr bool is_last_longname(fat32_longname_entry const& e) noexcept { return (e.ordinal & 0x40) != 0; }
union [[gnu::may_alias]] fat32_directory_entry
{
    fat32_regular_entry regular_entry;
    fat32_longname_entry longname_entry;
    constexpr fat32_directory_entry& operator=(fat32_directory_entry const& that) noexcept { __builtin_memcpy(std::bit_cast<uint8_t*>(this), &that, sizeof(fat32_directory_entry)); return *this; }
} __pack __align(1);
constexpr bool is_unused(fat32_directory_entry const& e) { return e.longname_entry.ordinal == 0xE8 || e.longname_entry.ordinal == 0; }
constexpr bool is_longname(fat32_directory_entry const& e) { return (e.longname_entry.attributes & 0x0F) == 0x0F; }
constexpr dword start_of(fat32_regular_entry const& e) { return dword{ e.first_cluster_lo, e.first_cluster_hi }; }
constexpr std::strong_ordering operator<=>(fat32_longname_entry const& a, fat32_longname_entry const& b) noexcept { return a.ordinal <=> b.ordinal; }
template<std::unsigned_integral IT> constexpr auto operator<=>(IT a, fat32_longname_entry const& b) noexcept -> decltype(std::declval<IT>() <=> b.ordinal) { return a <=> b.ordinal; }
template<std::unsigned_integral IT> constexpr auto operator<=>(fat32_longname_entry const& a, IT b) noexcept -> decltype(a.ordinal <=> std::declval<IT>()) { return a.ordinal <=> b; }
constexpr bool operator==(fat32_longname_entry const& a, fat32_longname_entry const& b) noexcept { return __builtin_memcmp(&a, &b, sizeof(fat32_longname_entry)) == 0; }
class fat32_allocation_table : protected std::__impl::__dynamic_buffer<uint32_t, std::allocator<uint32_t>>
{
    typedef std::__impl::__dynamic_buffer<uint32_t, std::allocator<uint32_t>> __base;
    size_t __num_sectors;
    uint64_t __start_sector;
public:
    typedef typename __base::__size_type size_type;
    typedef typename __base::__diff_type difference_type;
    typedef typename __base::__ref reference;
    typedef typename __base::__const_ref const_reference;
    typedef uint32_t* pointer;
    typedef uint32_t const* const_pointer;
    struct pointer_advance { constexpr pointer operator()(pointer begin, uint32_t val, pointer end) const noexcept { val &= fat32_cluster_mask; return (val < fat32_cluster_eof && begin + val < end) ? (begin + val) : end; } };
    struct const_pointer_advance { constexpr const_pointer operator()(const_pointer begin, uint32_t val, const_pointer end) const noexcept { val &= fat32_cluster_mask; return (val < fat32_cluster_eof && begin + val < end) ? (begin + val) : end; } };
    typedef __impl::__dereference_to_advance_iterator<const_pointer, fat32_allocation_table, const_pointer_advance> const_iterator;
    typedef __impl::__dereference_to_advance_iterator<pointer, fat32_allocation_table, pointer_advance> iterator;
    fat32_allocation_table(size_t num_sectors, size_t bytes_per_sector, uint64_t start_sector);
    bool sync_to_disk() const;
    bool get_from_disk();
    constexpr size_type size() const noexcept { return this->__size(); }
    constexpr size_type size_sectors() const noexcept { return __num_sectors; }
    constexpr pointer data() noexcept { return this->__beg(); }
    constexpr const_pointer data() const noexcept { return this->__beg(); }
    constexpr iterator begin() noexcept { return iterator{ this->__beg(), this->__beg(), this->__cur() }; }
    constexpr const_iterator cbegin() const noexcept { return const_iterator{ this->__beg(), this->__beg(), this->__cur() }; }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr iterator from(uint32_t val) noexcept { return iterator{ this->__beg(), val < fat32_cluster_eof && this->__beg() + val < this->__cur() ? this->__beg() + val : this->__cur(), this->__cur() }; }
    constexpr const_iterator cfrom(uint32_t val) const noexcept { val &= fat32_cluster_mask; return const_iterator{ this->__beg(), val < fat32_cluster_eof && this->__beg() + val < this->__cur() ? this->__beg() + val : this->__cur(), this->__cur() }; }
    constexpr const_iterator from(uint32_t value) const noexcept { return cfrom(value); }
    constexpr iterator end() noexcept { return iterator{ this->__beg(), this->__cur(), this->__cur() }; }
    constexpr const_iterator cend() const noexcept { return const_iterator{ this->__beg(), this->__cur(), this->__cur() }; }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr reference operator[](size_type n) noexcept { return *(this->__get_ptr(n)); }
    constexpr const_reference operator[](size_type n) const noexcept { return *(this->__get_ptr(n)); }
    constexpr iterator next_available() noexcept { for(size_t i = 3; i < this->size(); i++) { if(((*this)[i] & fat32_cluster_mask )== 0) return from(static_cast<uint32_t>(i)); } return end(); }
    constexpr const_iterator next_available() const noexcept { for(size_t i = 3; i < this->size(); i++) { if(((*this)[i] & fat32_cluster_mask) == 0) return from(static_cast<uint32_t>(i)); } return end(); }
};
uint32_t claim_cluster(fat32_allocation_table& tb, uint32_t last_sect = 0U);
class fat32_filebuf : public vfs_filebuf_base<char>
{
    using __base = vfs_filebuf_base<char>;
    using __cl_conv_fn_t = std::function<uint64_t(uint32_t)>;
    using __cl_get_fn_t = std::function<uint32_t(uint32_t)>;
    std::vector<uint32_t> __my_clusters;
    size_t __next_cluster_idx;
    __cl_conv_fn_t __cluster_to_sector_fn;
    __cl_get_fn_t __add_cluster_fn;
    friend class fat32_file_inode;
    friend class fat32_folder_inode;
public:
    using __base::char_type;
    using __base::int_type;
    using __base::off_type;
    using __base::traits_type;
protected:
    virtual int __ddwrite() override;
    virtual std::streamsize __ddread(std::streamsize n) override;
    virtual std::streamsize __ddrem() override;
    virtual std::streamsize __overflow(std::streamsize n) override;
public:
    fat32_filebuf(std::vector<uint32_t>&& covered_clusters, __cl_conv_fn_t const& cluster_convert_fn, __cl_get_fn_t const& add_cluster_fn);
};
struct fat32_node 
{
    fat32_regular_entry* disk_entry;
    constexpr fat32_node(fat32_regular_entry* e) noexcept : disk_entry{ e } {}
    constexpr uint32_t start_cluster() const noexcept { return disk_entry ? uint32_t(start_of(*disk_entry)) : 0; }
    friend constexpr std::strong_ordering operator<=>(fat32_node const& __this, fat32_node const& __that) noexcept { return vaddr_t(__this.disk_entry) <=> vaddr_t(__that.disk_entry); }

};
class fat32;
class fat32_file_inode : public virtual file_inode, public fat32_node
{
    fat32_filebuf __my_filebuf;
    size_t __on_disk_size;
public:
    using file_inode::traits_type;
    using file_inode::difference_type;
    using file_inode::size_type;
    using file_inode::pos_type;
    using file_inode::off_type;
    using file_inode::pointer;
    using file_inode::const_pointer;
    virtual size_type write(const_pointer src, size_type n) override;
    virtual size_type read(pointer dest, size_type n) override;
    virtual pos_type seek(off_type off, std::ios_base::seekdir way) override;
    virtual pos_type seek(pos_type pos) override;
    virtual bool fsync() override;
    virtual uint64_t size() const noexcept override;
    void on_open();
    fat32_file_inode(fat32* parent, std::string const& real_name, fat32_regular_entry* e);
    friend constexpr std::strong_ordering operator<=>(fat32_file_inode const& __this, fat32_file_inode const& __that) noexcept { return __this.disk_entry <=> __that.disk_entry; }
};
class fat32_folder_inode : public virtual folder_inode, public fat32_node
{
    tnode_dir __my_directory;
    std::map<std::string, fat32_regular_entry*> __my_names;    
    fat32* __my_parent_fs;
    std::vector<fat32_directory_entry> __my_dir_data;
    std::vector<uint32_t> __my_covered_clusters;
    size_t __n_files;
    size_t __n_folders;
    bool __has_init{ false };
    void __update_name_ptrs();
    void __expand_dir();
    bool __dir_ent_erase(std::string const& what);
    std::vector<fat32_directory_entry>::iterator __reclaim_stray(fat32_regular_entry* e);
    std::vector<fat32_directory_entry>::iterator __whereis(fat32_regular_entry* e);
    std::vector<fat32_directory_entry>::iterator __get_longname_start(fat32_regular_entry* e);
public:
    std::vector<fat32_directory_entry>::iterator first_unused_entry();
    virtual tnode* find(std::string const&) override;
    virtual bool link(tnode* original, std::string const& target) override;
    virtual tnode* add(inode* n) override;
    virtual bool unlink(std::string const& name) override;
    virtual uint64_t num_files() const noexcept override;
    virtual uint64_t num_folders() const noexcept override;
    virtual std::vector<std::string> lsdir() const override;
    virtual bool fsync() override;
    std::string get_short_name(std::string const& full);
    fat32_regular_entry* find_dirent(std::string const&);
    bool parse_dir_data();
    fat32_folder_inode(fat32* parent, std::string const& real_name, fat32_regular_entry* e);
    fat32_folder_inode(fat32* parent, std::string const& real_name, uint32_t root_cluster);
    friend constexpr std::strong_ordering operator<=>(fat32_folder_inode const& __this, fat32_folder_inode const& __that) noexcept { return __this.disk_entry <=> __that.disk_entry; }
};
class fat32 : public filesystem
{
    friend class fat32_folder_inode;
    friend class fat32_file_inode;
    std::set<fat32_file_inode> __file_nodes;
    std::set<fat32_folder_inode> __folder_nodes;    
    std::map<uint64_t, size_t> __st_cluster_ref_counts;
    uint8_t __sectors_per_cluster;
    uint64_t __sector_base;
    dev_t __dev_serial;
    fat32_allocation_table __the_table;
    fat32_folder_inode __root_directory;
    void __release_clusters_from(uint32_t start);
    friend void fat32_tests();
protected:
    void add_start_cluster_ref(uint64_t cl);
    void rm_start_cluster_ref(uint64_t cl);
    uint64_t cluster_to_sector(uint32_t cl) const noexcept;    
    virtual folder_inode* get_root_directory() override;
    virtual void dlfilenode(file_inode* fd) override;
    virtual void dldirnode(folder_inode* dd) override;
    virtual void syncdirs() override;
    virtual file_inode* mkfilenode(folder_inode* parent, std::string const& name) override;
    virtual folder_inode* mkdirnode(folder_inode* parent, std::string const& name) override;
    virtual dev_t xgdevid() const noexcept override;
    virtual file_inode* open_fd(tnode*) override;
    fat32(fat32_bootsect const& bootsect, uint64_t start_sector);
    fat32(uint64_t start_sector);
    constexpr int& get_next_fd() noexcept { return next_fd; }
    std::function<uint64_t (uint32_t)> cl_to_sect_fn{ [&](uint32_t cl) -> uint64_t { return cluster_to_sector(cl); } };
    fat32_file_inode* put_file_node(std::string const& name, fat32_regular_entry* e);
    fat32_folder_inode* put_folder_node(std::string const& name, fat32_regular_entry* e);
public:
    fat32();
    bool init();
};
#endif