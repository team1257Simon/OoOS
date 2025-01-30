#ifndef __FS_FAT32
#define __FS_FAT32
#include "fs/fs.hpp"
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
    constexpr fat_filetime(uint8_t hs, uint8_t m, uint8_t h) noexcept : half_seconds{ hs }, minutes{ m }, hours{ h } {}
    constexpr fat_filetime(uint16_t val) noexcept : fat_filetime{ static_cast<uint8_t>(val & 0x001F), static_cast<uint8_t>((val & 0x07E0) >> 5), static_cast<uint8_t>((val & 0xF800) >> 11)} {}
    constexpr fat_filetime() noexcept : fat_filetime{ 0U } {}
    constexpr operator uint16_t() const noexcept { return *std::bit_cast<uint16_t*>(this); }
} __pack;
struct fat_filedate
{
    uint8_t day     : 5;
    uint8_t month   : 4;
    uint8_t year    : 7;
    constexpr fat_filedate(uint8_t d, uint8_t m, uint8_t y) noexcept : day{ d }, month{ m }, year{ y } {}
    constexpr fat_filedate(uint16_t val) noexcept : fat_filedate{ static_cast<uint8_t>(val & 0x001F), static_cast<uint8_t>((val & 0x01E0) >> 5), static_cast<uint8_t>((val & 0xFE00) >> 9) } {}
    constexpr fat_filedate() noexcept : fat_filedate{ 0U } {}
    constexpr operator uint16_t() const noexcept { return *std::bit_cast<uint16_t*>(this); }
} __pack;
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
struct fat32_dir_entry
{
    char filename[11];
    struct
    {
        bool read_only : 1;
        bool hidden    : 1;
        bool system    : 1;
        bool volume_id : 1;
        bool directory : 1;
        bool archive   : 1;
        bool           : 1;
        bool           : 1;
    } __pack attributes;
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
} __pack;
struct longname_entry
{
    uint8_t ordinal;
    char16_t text_1[5];
    uint8_t attributes;
    uint8_t type;
    uint8_t shortname_checksum;
    char16_t text_2[6];
    uint16_t reserved;
    char16_t text_3[2];
} __pack;
constexpr std::strong_ordering operator<=>(longname_entry const& a, longname_entry const& b) noexcept { return a.ordinal <=> b.ordinal; }
template<std::unsigned_integral IT> constexpr auto operator<=>(IT a, longname_entry const& b) noexcept -> decltype(std::declval<IT>() <=> b.ordinal) { return a <=> b.ordinal; }
template<std::unsigned_integral IT> constexpr auto operator<=>(longname_entry const& a, IT b) noexcept -> decltype(a.ordinal <=> std::declval<IT>()) { return a.ordinal <=> b; }
constexpr bool operator==(longname_entry const& a, longname_entry const& b) noexcept { return __builtin_memcmp(&a, &b, sizeof(longname_entry)) == 0; }
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
    struct pointer_advance { constexpr pointer operator()(pointer begin, uint32_t val, pointer end) const noexcept { return (val < fat32_cluster_eof && begin + val < end) ? (begin + (val & fat32_cluster_mask)) : end; } };
    struct const_pointer_advance { constexpr const_pointer operator()(const_pointer begin, uint32_t val, const_pointer end) const noexcept { return (val < fat32_cluster_eof && begin + val < end) ? (begin + (val & fat32_cluster_mask)) : end; } };
    typedef __impl::__dereference_to_advance_iterator<const_pointer, fat32_allocation_table, const_pointer_advance> const_iterator;
    typedef __impl::__dereference_to_advance_iterator<pointer, fat32_allocation_table, pointer_advance> iterator;
    fat32_allocation_table(size_t num_sectors, uint64_t start_sector);
    bool sync_to_disk() const;
    bool get_from_disk();
    constexpr size_type size() const noexcept { return this->__size(); }
    constexpr size_type size_sectors() const noexcept { return __num_sectors; }
    constexpr pointer data() noexcept { return this->__beg(); }
    constexpr const_pointer data() const noexcept { return this->__beg(); }
    constexpr iterator begin() noexcept { return iterator{ this->__beg(), this->__beg(), this->__cur() }; }
    constexpr const_iterator cbegin() const noexcept { return const_iterator{ this->__beg(), this->__beg(), this->__cur() }; }
    constexpr const_iterator begin() const noexcept { return cbegin(); }
    constexpr iterator from(uint32_t value) noexcept { return iterator{ this->__beg(), pointer_advance{}(this->__beg(), value, this->__cur()), this->__cur() }; }
    constexpr const_iterator cfrom(uint32_t value) const noexcept { return const_iterator{ this->__beg(), const_pointer_advance{}(this->__beg(), value, this->__cur()), this->__cur() }; }
    constexpr const_iterator from(uint32_t value) const noexcept { return cfrom(value); }
    constexpr iterator end() noexcept { return iterator{ this->__beg(), this->__cur(), this->__cur() }; }
    constexpr const_iterator cend() const noexcept { return const_iterator{ this->__beg(), this->__cur(), this->__cur() }; }
    constexpr const_iterator end() const noexcept { return cend(); }
    constexpr reference operator[](size_type n) noexcept { return *(this->__get_ptr(n)); }
    constexpr const_reference operator[](size_type n) const noexcept { return *(this->__get_ptr(n)); }
    constexpr iterator next_available() noexcept { for(size_t i = 3; i < this->size(); i++) { if(((*this)[i] & fat32_cluster_mask )== 0) return from(static_cast<uint32_t>(i)); } return end(); }
    constexpr const_iterator next_available() const noexcept { for(size_t i = 3; i < this->size(); i++) { if(((*this)[i] & fat32_cluster_mask) == 0) return from(static_cast<uint32_t>(i)); } return end(); }
};
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
class fat32_file_inode : public file_inode
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
    fat32_file_inode(int fd, std::string const& real_name, fat32_dir_entry const& e, fat32_allocation_table& tb, fat32_filebuf::__cl_conv_fn_t const& cluster_to_sector);
};
#endif