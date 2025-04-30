#ifndef __FS_FAT32
#define __FS_FAT32
#include "kernel/fs/fs.hpp"
#include "map"
#include "sys/time.h"
constexpr uint16_t fat_year_base = 1980u;
constexpr uint32_t fsinfo_magic = 0x41615252U;      // Alex was killed by magic
constexpr uint32_t fsinfo_more_magic = 0x61417272U; // Steve was killed by more magic
constexpr uint32_t fsinfo_tail = 0xAA550000;
constexpr uint32_t fat32_cluster_mask = 0x0FFFFFFF;
constexpr uint32_t fat32_cluster_pres = 0xF0000000;
constexpr uint32_t fat32_cluster_eof = 0x0FFFFFF8;
union [[gnu::may_alias]] fat_filetime
{
    struct 
    {
        uint8_t half_seconds    : 5;
        uint8_t minutes         : 6;
        uint8_t hours           : 5;
    } __pack;
    uint16_t value{};
    constexpr fat_filetime(fat_filetime const& that) noexcept : value{ that.value } {}
    constexpr fat_filetime(fat_filetime&& that) noexcept : value{ that.value } {}
    constexpr fat_filetime(uint8_t hs, uint8_t m, uint8_t h) noexcept : half_seconds{ hs }, minutes{ m }, hours{ h } {}
    constexpr fat_filetime(uint16_t val) noexcept : value{ val } {}
    constexpr fat_filetime() noexcept = default;
    constexpr operator time_t() const noexcept { return static_cast<time_t>(half_seconds) * 2000UL + static_cast<time_t>(minutes) * 60000UL + static_cast<time_t>(hours) * 3600000UL;  }
} __pack __align(1);
union [[gnu::may_alias]] fat_filedate
{
    struct
    {
        uint8_t day     : 5;
        uint8_t month   : 4;
        uint8_t year    : 7;
    } __pack;
    uint16_t value{};
    constexpr fat_filedate(fat_filedate const& that) noexcept : value{ that.value } {}
    constexpr fat_filedate(fat_filedate&& that) noexcept : value{ that.value } {}
    constexpr fat_filedate(uint8_t d, uint8_t m, uint8_t y) noexcept : day{ d }, month{ m }, year{ y } {}
    constexpr fat_filedate(uint16_t val) noexcept : value{ val } {}
    constexpr fat_filedate() noexcept = default;
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
    uint8_t winnt_reserved = 0;
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
    char full[32]{};
    constexpr fat32_directory_entry(fat32_directory_entry const& that) { array_copy(full, that.full, 32); }
    constexpr fat32_directory_entry(fat32_directory_entry&& that) { array_move(full, that.full, 32); }
    inline fat32_directory_entry() : full{} {}
    constexpr fat32_directory_entry& operator=(fat32_directory_entry const& that) { array_copy(full, that.full, 32); return *this; }
    constexpr fat32_directory_entry& operator=(fat32_directory_entry&& that) { array_move(full, that.full, 32); return *this; }
} __pack __align(1);
constexpr bool is_unused(fat32_directory_entry const& e) { return e.longname_entry.ordinal == 0xE8 || e.longname_entry.ordinal == 0; }
constexpr bool is_longname(fat32_directory_entry const& e) { return (e.longname_entry.attributes & 0x0F) == 0x0F; }
constexpr dword start_of(fat32_regular_entry const& e) { return dword{ e.first_cluster_lo, e.first_cluster_hi }; }
constexpr std::strong_ordering operator<=>(fat32_longname_entry const& a, fat32_longname_entry const& b) noexcept { return a.ordinal <=> b.ordinal; }
template<std::unsigned_integral IT> constexpr auto operator<=>(IT a, fat32_longname_entry const& b) noexcept -> decltype(std::declval<IT>() <=> b.ordinal) { return a <=> b.ordinal; }
template<std::unsigned_integral IT> constexpr auto operator<=>(fat32_longname_entry const& a, IT b) noexcept -> decltype(a.ordinal <=> std::declval<IT>()) { return a.ordinal <=> b; }
constexpr bool operator==(fat32_longname_entry const& a, fat32_longname_entry const& b) noexcept { return __builtin_memcmp(&a, &b, sizeof(fat32_longname_entry)) == 0; }
class fat32;
class fat32_allocation_table : protected std::__impl::__dynamic_buffer<uint32_t, std::allocator<uint32_t>>
{
    typedef std::__impl::__dynamic_buffer<uint32_t, std::allocator<uint32_t>> __base;
    size_t __num_sectors;
    uint64_t __start_sector;
    fat32* __parent;
    mutable bool __dirty{ false };
public:
    typedef typename __base::__size_type size_type;
    typedef typename __base::__diff_type difference_type;
    typedef typename __base::__ref reference;
    typedef typename __base::__const_ref const_reference;
    typedef uint32_t* pointer;
    typedef uint32_t const* const_pointer;
    fat32_allocation_table(size_t num_sectors, size_t bytes_per_sector, uint64_t start_sector, fat32* parent);
    bool sync_to_disk() const;
    bool get_from_disk();
    constexpr size_type size() const noexcept { return this->__size(); }
    constexpr size_type size_sectors() const noexcept { return __num_sectors; }
    constexpr pointer data() noexcept { return this->__beg(); }
    constexpr const_pointer data() const noexcept { return this->__beg(); }
    constexpr reference operator[](size_type n) noexcept { return *(this->__get_ptr(n)); }
    constexpr const_reference operator[](size_type n) const noexcept { return *(this->__get_ptr(n)); }
    constexpr void mark_dirty() noexcept { __dirty = true; }
};
uint32_t claim_cluster(fat32_allocation_table& tb, uint32_t last_sect = 0U);
class fat32_file_node;
class fat32_filebuf : public std::ext::dynamic_streambuf<char>
{
    using __base = std::ext::dynamic_streambuf<char>;
    friend class fat32_file_node;
    friend class fat32_directory_node;
    std::vector<uint32_t> __my_clusters;
    size_t __next_cluster_idx;
    fat32_file_node* __parent;
public:
    using __base::char_type;
    using __base::int_type;
    using __base::off_type;
    using __base::traits_type;
protected:
    virtual int write_dev() override;
    virtual std::streamsize read_dev(std::streamsize n) override;
    virtual std::streamsize unread_size() override;
    virtual std::streamsize on_overflow(std::streamsize n) override;
    virtual int sync() override;
    bool grow_file(size_t added);
public:
    fat32_filebuf(std::vector<uint32_t>&& covered_clusters, fat32_file_node* parent);
};
class fat32_directory_node;
struct fat32_node 
{
    fat32* parent_fs;    
    fat32_directory_node* parent_dir;
    size_t dirent_index;
    fat32_node(fat32* pfs, fat32_directory_node* pdir, size_t didx) noexcept;
    uint32_t start_cluster() const noexcept;
    fat32_regular_entry* disk_entry() noexcept;
    fat32_regular_entry const* disk_entry() const noexcept;
};
class fat32_file_node final : public file_node, public fat32_node
{
    fat32_filebuf __my_filebuf;
    size_t __on_disk_size;
    friend class fat32_filebuf;
public:
    using file_node::traits_type;
    using file_node::difference_type;
    using file_node::size_type;
    using file_node::pos_type;
    using file_node::off_type;
    using file_node::pointer;
    using file_node::const_pointer;
    virtual size_type write(const_pointer src, size_type n) override;
    virtual size_type read(pointer dest, size_type n) override;
    virtual pos_type seek(off_type off, std::ios_base::seekdir way) override;
    virtual pos_type seek(pos_type pos) override;
    virtual bool fsync() override;
    virtual uint64_t size() const noexcept override;
    virtual pos_type tell() const;
    virtual bool truncate() override;
    virtual char* data() override;
    virtual bool grow(size_t added) override;
    void on_open();
    void set_fd(int i);
    uint32_t claim_next(uint32_t cl);
    uint64_t cl_to_s(uint32_t cl);    
    fat32_file_node(fat32* pfs, std::string const& real_name, fat32_directory_node* pdir, uint32_t cl_st, size_t dirent_idx);
    friend constexpr std::strong_ordering operator<=>(fat32_file_node const& __this, fat32_file_node const& __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(std::char_traits<char>::compare(__this.concrete_name.c_str(), __that.concrete_name.c_str(), std::max(__this.concrete_name.size(), __that.concrete_name.size()))); }
};
class fat32_directory_node final : public directory_node, public fat32_node
{
    tnode_dir __my_dir;
    std::vector<fat32_directory_entry> __my_dir_data;
    std::vector<uint32_t> __my_covered_clusters;
    bool __has_init     { false };    
    size_t __n_files    {};
    size_t __n_folders  {};
    bool __dirty        { false };
    friend class fat32_node;
    friend class fat32;    
    void __expand_dir();
    bool __read_disk_data();
    void __add_parsed_entry(fat32_regular_entry const& e, size_t j);
    bool __dir_ent_erase(std::string const& what);
    std::vector<fat32_directory_entry>::iterator __whereis(fat32_regular_entry* e);
    std::vector<fat32_directory_entry>::iterator __get_longname_start(fat32_regular_entry* e);
public:
    std::vector<fat32_directory_entry>::iterator first_unused_entry();
    virtual tnode* find(std::string const&) override;
    virtual bool link(tnode* original, std::string const& target) override;
    virtual tnode* add(fs_node* n) override;
    virtual bool unlink(std::string const& name) override;
    virtual uint64_t num_files() const noexcept override;
    virtual uint64_t num_subdirs() const noexcept override;
    virtual std::vector<std::string> lsdir() const override;
    virtual size_t readdir(std::vector<tnode*>& out_vec) override;
    virtual uint64_t size() const noexcept override;
    virtual bool fsync() override;
    virtual bool truncate() override;
    void get_short_name(std::string const &full, std::string& result);
    fat32_regular_entry* find_dirent(std::string const&);
    bool parse_dir_data();
    constexpr bool valid() const { return __has_init; }
    constexpr void mark_dirty() { __dirty = true; }
    fat32_directory_node(fat32* pfs, std::string const& real_name, fat32_directory_node* pdir, uint32_t cl_st, size_t dirent_idx);
    friend constexpr std::strong_ordering operator<=>(fat32_directory_node const& __this, fat32_directory_node const& __that) noexcept { return std::__detail::__char_traits_cmp_cat<std::char_traits<char>>(std::char_traits<char>::compare(__this.concrete_name.c_str(), __that.concrete_name.c_str(), std::max(__this.concrete_name.size(), __that.concrete_name.size()))); }
};
class fat32 final : public filesystem
{
    uint32_t __root_cl_num;
    uint8_t __sectors_per_cluster;
    uint64_t __sector_base;
    dev_t __dev_serial;
    size_t __sector_size;
    std::set<fat32_file_node> __file_nodes{};
    std::set<fat32_directory_node> __directory_nodes{};    
    std::map<uint64_t, size_t> __st_cluster_ref_counts{};
    fat32_directory_node* __root_directory{};
    fat32_allocation_table __the_table;
    static fat32 *__instance;
    static bool __has_init;
    void __release_clusters_from(uint32_t start);
    friend class fat32_directory_node;
    friend class fat32_file_node;
    friend class fat32_allocation_table;
    friend class fat32_filebuf;
protected:
    void add_start_cluster_ref(uint64_t cl);
    void rm_start_cluster_ref(uint64_t cl);
    uint64_t cluster_to_sector(uint32_t cl) const noexcept;    
    virtual directory_node* get_root_directory() override;
    virtual void dlfilenode(file_node* fd) override;
    virtual void dldirnode(directory_node* dd) override;
    virtual void syncdirs() override;
    virtual file_node* mkfilenode(directory_node* parent, std::string const& name) override;
    virtual directory_node* mkdirnode(directory_node* parent, std::string const& name) override;
    virtual dev_t xgdevid() const noexcept override;
    virtual file_node* on_open(tnode*) override;
    fat32_file_node* put_file_node(std::string const& name, fat32_directory_node* parent, uint32_t cl0, size_t dirent_idx);
    fat32_directory_node* put_directory_node(std::string const& name, fat32_directory_node* parent, uint32_t cl0, size_t dirent_idx);
    fat32(uint32_t root_cl, uint8_t sectors_per_cl, uint16_t bps, uint64_t first_sect, uint64_t fat_sectors, dev_t drive_serial);
    bool init();
    ~fat32();
    bool write_sectors(uint32_t start, const char* data, size_t num);
    bool read_sectors(char* buffer, uint32_t start, size_t num);
    bool write_clusters(uint32_t cl_st, const char* data, size_t num = 1UL);
    bool read_clusters(char* buffer, uint32_t cl_st, size_t num = 1UL);
public:
    virtual size_t block_size() override;
    static bool has_init();
    static bool init_instance();
    static fat32* get_instance();
};
#endif