#include "fs/ext.hpp"
#include "kdebug.hpp"
static std::allocator<char> ch_alloc{};;
static uint32_t compute_csum_seed(extfs* parent, uint32_t inode_num, ext_inode* inode);
ext_vnode_base::ext_vnode_base(extfs* parent, uint32_t inode_num, ext_inode* inode) : inode_number{ inode_num }, parent_fs{ parent }, on_disk_node{ inode }, checksum_seed{ compute_csum_seed(parent, inode_num, inode) } {}
ext_vnode_base::ext_vnode_base(extfs* parent, uint32_t inode_number) : ext_vnode_base(parent, inode_number, parent->get_inode(inode_number)) {}
ext_vnode_base::ext_vnode_base() = default;
ext_vnode_base::~ext_vnode_base() = default;
ext_vnode::ext_vnode(extfs* parent, uint32_t inode_number, ext_inode* inode) : ext_vnode_base(parent, inode_number, inode), base_buffer(), extents(this) {}
ext_vnode::ext_vnode(extfs* parent, uint32_t inode_number) : ext_vnode(parent, inode_number, parent->get_inode(inode_number)) {}
bool ext_vnode::initialize() { return init_extents(); }
void ext_vnode::mark_write(void* pos) { addr_t addr(pos); if(!__out_of_range(addr)) { block_data[block_of_data_ptr(static_cast<size_t>(addr - addr_t(__beg())))].dirty = true; } }
ext_vnode::ext_vnode() = default;
ext_vnode::~ext_vnode() = default;
size_t ext_vnode::block_of_data_ptr(size_t offs) { return offs / parent_fs->block_size(); }
uint64_t ext_vnode::next_block() { return block_data[last_checked_block_idx + 1].block_number; }
void ext_vnode::on_modify() { base_buffer::on_modify(); if(fs_node* fn = dynamic_cast<fs_node*>(this)) { fn->fsync(); } }
static inline size_t unused_dirent_space(ext_dir_entry const& de) { return static_cast<size_t>(static_cast<size_t>(de.entry_size) - static_cast<size_t>(de.name_len + 8UL)); }
ext_dir_entry* ext_directory_vnode::__current_ent() { return reinterpret_cast<ext_dir_entry*>(__cur()); }
char* ext_directory_vnode::__current_block_start() { return __get_ptr(block_of_data_ptr(tell()) * parent_fs->block_size()); }
bool ext_directory_vnode::fsync() { return update_inode() && parent_fs->persist(this); }
ext_directory_vnode::ext_directory_vnode(extfs* parent, uint32_t inode_number, int fd) : ext_vnode(parent, inode_number), directory_node(std::move(""), fd, inode_number) { mode = on_disk_node->mode; }
ext_directory_vnode::ext_directory_vnode(extfs* parent, uint32_t inode_number, ext_inode* inode_data, int fd) : ext_vnode(parent, inode_number, inode_data), directory_node(std::move(""), fd, inode_number) { mode = on_disk_node->mode; }
ext_directory_vnode::~ext_directory_vnode() = default;
tnode* ext_directory_vnode::add(fs_node* n) { for(tnode& node : directory_tnodes) { if(node.ptr() == n) return std::addressof(node); } return nullptr; }
bool ext_directory_vnode::truncate() { return parent_fs->truncate_node(this); }
ext_file_vnode::size_type ext_file_vnode::read(pointer dest, size_type n) { return sgetn(dest, n); }
ext_file_vnode::size_type ext_file_vnode::write(const_pointer src, size_type n) { size_t result = ext_vnode::xsputn(src, n); return result && fsync() ? result : 0; }
ext_file_vnode::pos_type ext_file_vnode::seek(off_type off, std::ios_base::seekdir way) { return way == std::ios_base::end ? seek(size() + off) : std::ext::dynamic_streambuf<char>::seekoff(off, way); }
ext_file_vnode::pos_type ext_file_vnode::seek(pos_type pos) { return std::ext::dynamic_streambuf<char>::seekpos(pos); }
uint64_t ext_file_vnode::size() const noexcept { return qword(on_disk_node->size_lo, on_disk_node->size_hi); }
ext_file_vnode::pos_type ext_file_vnode::tell() const { return std::ext::dynamic_streambuf<char>::tell(); }
ext_file_vnode::ext_file_vnode(extfs* parent, uint32_t inode_number, int fd) : ext_vnode(parent, inode_number), file_node(std::move(""), fd, inode_number) { mode = on_disk_node->mode; }
ext_file_vnode::ext_file_vnode(extfs* parent, uint32_t inode_number, ext_inode* inode_data, int fd) : ext_vnode(parent, inode_number, inode_data), file_node(std::move(""), fd, inode_number) { mode = on_disk_node->mode; }
ext_file_vnode::~ext_file_vnode() = default;
bool ext_file_vnode::truncate() { return parent_fs->truncate_node(this); }
char* ext_file_vnode::data() { return __beg(); }
void ext_file_vnode::force_write() { for(disk_block& b : block_data) { b.dirty = true; } }
ext_device_vnode::ext_device_vnode(extfs* parent, uint32_t inode_num, ext_inode* inode, device_stream* dev, int fd) : ext_vnode_base(parent, inode_num, inode), device_node("", fd, dev, inode->device_hardlink_id) { mode = on_disk_node->mode; }
ext_device_vnode::ext_device_vnode(extfs* parent, uint32_t inode_num, device_stream* dev, int fd) : ext_device_vnode(parent, inode_num, parent->get_inode(inode_num), dev, fd) {}
ext_pipe_vnode::ext_pipe_vnode(extfs* parent, uint32_t inode_number, int fd) : ext_pipe_vnode(parent, inode_number, parent->get_inode(inode_number), fd) {}
ext_pipe_vnode::ext_pipe_vnode(extfs *parent, uint32_t inode_number, int fd, size_t pipe_id) : ext_pipe_vnode(parent, inode_number, parent->get_inode(inode_number), fd, pipe_id) {}
ext_device_vnode::~ext_device_vnode() = default;
bool ext_device_vnode::fsync() { return update_inode() && device_node::fsync(); }
ext_pipe_vnode::ext_pipe_vnode(extfs* parent, uint32_t inode_num, ext_inode* inode, int fd, size_t pipe_id) : file_node(std::move(std::string(inode->block_info.link_target)), fd, inode_num), ext_vnode_base(parent, inode_num, inode), pipe_node(fd, pipe_id) { mode = on_disk_node->mode; }
ext_pipe_vnode::ext_pipe_vnode(extfs* parent, uint32_t inode_num, ext_inode* inode, int fd) : file_node(std::move(std::string(inode->block_info.link_target)), fd, inode_num), ext_vnode_base(parent, inode_num, inode), pipe_node(fd) { mode = on_disk_node->mode; }
ext_pipe_vnode::~ext_pipe_vnode() = default;
ext_pipe_pair::ext_pipe_pair(extfs *parent, uint32_t inode_number, std::string const& name, int fd0, int fd1) : fs_node(name, fd0, inode_number), in(parent, inode_number, fd0), out(parent, inode_number, fd1, in.pipe_id()) {}
ext_pipe_pair::~ext_pipe_pair() = default;
uint64_t ext_pipe_pair::size() const noexcept { return in.size(); }
bool ext_pipe_pair::fsync() { return in.update_inode(); }
bool ext_pipe_pair::truncate() { return in.truncate(); }
bool ext_pipe_pair::is_pipe() const noexcept { return true; }
struct guarded_inode_buffer
{
    std::allocator<uint8_t> alloc;
    size_t sz;
    uint8_t* buffer;
    void accept(ext_inode const& n) { ext_inode* copy = new(buffer) ext_inode(n); copy->checksum_lo = copy->checksum_hi = 0; }
    guarded_inode_buffer(size_t s) : alloc(), sz(s), buffer(alloc.allocate(s)) {}
    ~guarded_inode_buffer() { alloc.deallocate(buffer, sz); }
};
static uint32_t compute_inode_csum(uint32_t checksum_seed, extfs* parent_fs, ext_inode* on_disk_node)
{
    guarded_inode_buffer buff(parent_fs->inode_size());
    buff.accept(*on_disk_node);
    return crc32c_x86_3way(checksum_seed, buff.buffer, buff.sz);
}
static uint32_t compute_csum_seed(extfs* parent, uint32_t inode_num, ext_inode* inode)
{
    dword gen = inode->version_lo;
    dword ino = inode_num;
    uint32_t csum = crc32c(crc32c(parent->get_uuid_csum(), ino), gen);
    if(uint32_t checkval = dword(inode->checksum_lo, inode->checksum_hi))
    {
        uint32_t calculated = compute_inode_csum(csum, parent, inode);
        if(calculated != checkval) 
            throw std::runtime_error{ "inode checksum " + std::to_string(checkval, std::ext::hex) + " does not match calculated " + std::to_string(calculated, std::ext::hex) };
    }
    return csum;
}
bool ext_vnode_base::update_inode()
{
    qword timestamp = sys_time(nullptr);
    on_disk_node->modified_time = timestamp.lo;
    on_disk_node->mod_time_extra = (timestamp.hi.hi.hi >> 4) & 0x03;
    on_disk_node->accessed_time = timestamp.lo;
    on_disk_node->access_time_hi = (timestamp.hi.hi.hi >> 4) & 0x03;
    if(fs_node* fn = dynamic_cast<fs_node*>(this))
    {
        fn->modif_time = timestamp;
        if(static_cast<uint16_t>(on_disk_node->mode) != static_cast<uint16_t>(fn->mode))
            on_disk_node->mode = fn->mode; 
    }
    dword csval = compute_inode_csum(checksum_seed, parent_fs, on_disk_node);
    on_disk_node->checksum_lo = csval.lo;
    if(parent_fs->inode_size() >= offsetof(ext_inode, checksum_hi)) on_disk_node->checksum_hi = csval.hi;
    return parent_fs->persist_inode(inode_number);
}
bool ext_vnode::init_extents()
{
    if((on_disk_node->flags & use_extents) ? extents.parse_ext4() : extents.parse_legacy()) return true;
    for(size_t i = 0; i < cached_metadata.size(); i++) { parent_fs->free_block_buffer(cached_metadata[i]); }
    return false;
}
void ext_vnode::truncate_buffer()
{
    __destroy();
    on_disk_node->blocks_count_hi = on_disk_node->blocks_count_lo = 0U;
    on_disk_node->size_hi = on_disk_node->size_lo = 0U;
}
void ext_vnode::update_block_ptrs()
{
    if(!__beg()) return;
    on_modify();
    size_t bs = parent_fs->block_size();
    char* pos = __beg();
    for(size_t i = 0; i < block_data.size() && pos < __max(); i++) { block_data[i].data_buffer = pos; pos += block_data[i].chain_len * bs; }
}
bool ext_vnode::expand_buffer(size_t added_bytes, size_t written_bytes)
{
    if(!__grow_buffer(added_bytes)) return false;
    qword fsz(on_disk_node->size_lo, on_disk_node->size_hi);
    fsz += written_bytes;
    on_disk_node->size_lo = fsz.lo;
    on_disk_node->size_hi = fsz.hi;
    update_block_ptrs();
    return true;
}
bool ext_vnode::expand_buffer(size_t added_bytes)
{
    if(!__grow_buffer(added_bytes)) return false;
    qword fsz(on_disk_node->size_lo, on_disk_node->size_hi);
    fsz += added_bytes;
    on_disk_node->size_lo = fsz.lo;
    on_disk_node->size_hi = fsz.hi;
    update_block_ptrs();
    return true;
}
std::streamsize ext_vnode::xsputn(const char* s, std::streamsize n)
{
    if(n > __capacity() && !on_overflow(static_cast<std::streamsize>(n - __capacity()))) return 0;
    uint64_t sblk = block_of_data_ptr(__size());
    size_t nblk = div_round_up(n, parent_fs->block_size());
    array_copy(__cur(), s, n);
    __bumpc(n);
    for(size_t i = 0; i < nblk; i++) block_data[sblk + i].dirty = true;
    on_modify();
    return n;
}
bool ext_file_vnode::grow(size_t added)
{
    size_t sz = size();
    size_t bs = parent_fs->block_size();
    size_t blk_cap = up_to_nearest(sz, bs);
    qword target_size(sz + added);
    if(target_size < blk_cap)
    {
        on_disk_node->size_lo = target_size.lo;
        on_disk_node->size_hi = target_size.hi;
        return fsync();
    }
    return on_overflow(added) != 0;
}
bool ext_file_vnode::fsync() 
{
    size_t updated_size = __size();
    if(size() < updated_size)
    {
        qword fsz(updated_size);
        on_disk_node->size_lo = fsz.lo;
        on_disk_node->size_hi = fsz.hi;
    }
    return update_inode() && parent_fs->persist(this); 
}
std::streamsize ext_file_vnode::on_overflow(std::streamsize n)
{
    size_t bs = parent_fs->block_size();
    size_t needed = div_round_up(n, bs);
    size_t ccap = __capacity();
    if(disk_block* blk = parent_fs->claim_blocks(this, needed)) 
    { 
        if(!expand_buffer(bs * blk->chain_len, n)) return 0;
        array_zero(__get_ptr(ccap), bs * blk->chain_len);
        return bs * blk->chain_len; 
    }
    panic("no blocks");
    return 0;
}
bool ext_file_vnode::initialize()
{
    if(__initialized) { __rst(); return true; }
    if(!init_extents()) return false;
    if(extents.total_extent)
    {
        if(!__grow_buffer(extents.total_extent * parent_fs->block_size())) return false;
        update_block_ptrs();
        for(disk_block& db : block_data) { if(!parent_fs->read_hd(db)) { panic("block read failed"); return false; } }
        qword timestamp = sys_time(nullptr);
        on_disk_node->accessed_time = timestamp.lo;
        on_disk_node->access_time_hi = (timestamp.hi.hi.hi >> 4) & 0x03;
        return (__initialized = update_inode());
    }
    return (__initialized = true);
}
bool ext_directory_vnode::initialize()
{
    if(__initialized) return true;
    size_t bs = parent_fs->block_size();
    if(!init_extents()) return false;
    if(!__grow_buffer(extents.total_extent * bs)) { panic("failed to allocate buffer for directory data"); return false; }
    update_block_ptrs();
    for(size_t i = 0; i < block_data.size(); i++) { if(!parent_fs->read_hd(block_data[i])) { std::string errstr = "read failed on directory block " + std::to_string(block_data[i].block_number); panic(errstr.c_str()); return false; } }
    return (__initialized = __parse_entries(bs));
}
tnode* ext_directory_vnode::__resolve_link_r(ext_vnode* vn, std::set<fs_node*>& checked_elements)
{
    std::string separator = parent_fs->get_path_separator();
    if(vn->on_disk_node->block_info.ext4_extent.header.magic == ext_extent_magic) 
    {
        if(!vn->initialize()) throw std::runtime_error{ "symlink inode read failed" };
        size_t n = vn->count();
        tnode* result = nullptr;
        char* buff = ch_alloc.allocate(n);
        if(vn->sgetn(buff, n))
        {
            std::string xlink_str(buff, std::strnlen(buff, 255));
            if(std::strncmp(buff, separator.c_str(), separator.size()))
                result = parent_fs->resolve_symlink(this, xlink_str, checked_elements); // relative
            else result = parent_fs->resolve_symlink(nullptr, buff, checked_elements);  // absolute
        }
        ch_alloc.deallocate(buff, n);
        if(!result) throw std::runtime_error{ "bad symlink" };
        return result;
    }
    char* link_str = vn->on_disk_node->block_info.link_target;
    std::string xlink_str(link_str, std::strnlen(link_str, 60));
    if(tnode* result = parent_fs->resolve_symlink(std::strncmp(link_str, separator.c_str(), separator.size()) ? this : nullptr, xlink_str, checked_elements))
        return result;
    throw std::runtime_error{ "bad symlink" };
}
tnode* ext_directory_vnode::find(std::string const& name) 
{
    if(!initialize()) return nullptr;
    std::set<fs_node*> checked_elements{};
    return find_r(name, checked_elements);
}
tnode* ext_directory_vnode::find_l(std::string const& name)
{
    if(!initialize()) return nullptr;
    if(tnode_dir::iterator i = directory_tnodes.find(name); i != directory_tnodes.end()) return i.base();
    return nullptr;
}
tnode* ext_directory_vnode::find_r(std::string const& name, std::set<fs_node*>& checked_elements)
{
    if(tnode_dir::iterator i = directory_tnodes.find(name); i != directory_tnodes.end())
    {
        if(ext_vnode* vn = dynamic_cast<ext_vnode*>(i->ptr()); vn && vn->is_symlink())
        {
            if(checked_elements.contains(i->ptr()))
                throw std::overflow_error{ "circular link" };
            checked_elements.insert(i->ptr());
            return __resolve_link_r(vn, checked_elements); // symlink
        }
        else return i.base(); // device node or regular file/dirnode
    }
    else return nullptr;
}
std::streamsize ext_directory_vnode::on_overflow(std::streamsize n)
{
    size_t bs = parent_fs->block_size();
    if(disk_block* blk = parent_fs->claim_blocks(this))
    {
        if(!expand_buffer(bs)) return 0;
        // the data pointer of returned block should point at the newly added space
        __setc(blk->data_buffer);
        array_zero(__cur(), bs);
        ext_dir_entry* dirent = __current_ent();
        size_t rem = static_cast<size_t>(bs - n - 12);
        dirent->entry_size = n;
        reinterpret_cast<ext_dir_entry*>(__cur() + n)->entry_size = rem;
        return bs;
    }
    return 0;
}
bool ext_directory_vnode::__seek_available_entry(size_t needed_len)
{
    while(__cur() < __max())
    {
        ext_dir_entry* dirent = __current_ent();
        if(!dirent->inode_idx && dirent->entry_size >= needed_len) return true; // already-usable entry
        else if(unused_dirent_space(*dirent) >= needed_len) return false; // have to divide the entry
        __bumpc(dirent->entry_size);
    }
    __setc(__max());
    return false; // indicate no open entries by having the pointer at the end of the directory file and returning false
}
void ext_directory_vnode::__write_dir_entry(ext_vnode_base* vnode, ext_dirent_type type, const char* name, size_t name_len)
{
    ext_dir_entry* dirent = __current_ent();
    size_t bs =  parent_fs->block_size();
    dirent->name_len = name_len;
    dirent->type_ind = type;
    array_copy(dirent->name, name, name_len);
    if(dirent->entry_size > name_len + 8) array_zero(dirent->name + name_len, static_cast<size_t>(dirent->entry_size - 8 - name_len));
    dirent->inode_idx = vnode->inode_number;
    mark_write(dirent);
    char* cblk = __current_block_start();
    ext_dir_tail* tail = new(static_cast<void*>(cblk + bs - 12)) ext_dir_tail(0U);
    uint32_t csum = crc32c(parent_fs->get_uuid_csum(), cblk, bs);
    tail->csum = csum;
}
bool ext_directory_vnode::add_dir_entry(ext_vnode_base* vnode, ext_dirent_type type, const char* nm, size_t name_len)
{
    size_t needed_len = up_to_nearest(name_len + 8, 4);
    if(!__seek_available_entry(needed_len)) 
    {
        if(__cur() < __max())
        {
            ext_dir_entry* dirent = __current_ent();
            size_t nsz = up_to_nearest(dirent->name_len + 8UL, 4);
            size_t rem = dirent->entry_size - nsz;
            dirent->entry_size = nsz;
            mark_write(dirent);
            __bumpc(nsz);
            __current_ent()->entry_size = rem;
        }
        else if(!on_overflow(needed_len)) return false;
    }
    __write_dir_entry(vnode, type, nm, name_len);
    vnode->on_disk_node->referencing_dirents++;
    if(type == dti_dir) subdir_count++;
    else file_count++;
    if(fs_node* fnode = dynamic_cast<fs_node*>(vnode))
    {
        tnode* tn = directory_tnodes.emplace(fnode, nm).first.base();
        size_t i = block_of_data_ptr(tell());
        __dir_index.insert_or_assign(tn, std::move(dirent_idx{ .block_num = static_cast<uint32_t>(i), .block_offs = static_cast<uint32_t>(tell() % parent_fs->block_size()) }) );
    }
    return fsync();
}
bool ext_directory_vnode::link(tnode* original, std::string const& target)
{
    ext_dirent_type type = original->is_file() ? dti_regular : dti_dir;
    if(ext_directory_vnode* vnode = dynamic_cast<ext_directory_vnode*>(original->ptr()))
        return add_dir_entry(vnode, type, target.data(), target.size()) && vnode->update_inode();
    else if(ext_vnode* vnode = dynamic_cast<ext_vnode*>(original->ptr()))
        return add_dir_entry(vnode, type, target.data(), target.size()) && vnode->update_inode();
    return false;
}
bool ext_directory_vnode::unlink(std::string const& name)
{
    bool success = false;
    size_t bs = parent_fs->block_size();
    if(tnode_dir::iterator i = directory_tnodes.find(name); i != directory_tnodes.end())
    {
        if(std::map<tnode*, dirent_idx>::iterator j = __dir_index.find(i.base()); j != __dir_index.end())
        {
            __setc(__beg() + j->second.block_num * bs + j->second.block_offs);
            ext_dir_entry* dirent = __current_ent();
            array_zero(dirent->name, dirent->name_len);
            dirent->name_len = 0;
            dirent->inode_idx = 0;
            dirent->type_ind = 0;
            char* cblk = __current_block_start();
            ext_dir_tail* tail = new(static_cast<void*>(cblk + bs - 12)) ext_dir_tail(0U);
            uint32_t csum = crc32c(parent_fs->get_uuid_csum(), cblk, bs);
            tail->csum = csum;
            mark_write(dirent);
            __dir_index.erase(j);
            if(ext_vnode* vn = dynamic_cast<ext_vnode*>(i->ptr())) { vn->on_disk_node->referencing_dirents--; success = vn->update_inode() && fsync(); }
            else success = fsync();
        }
    }
    return success;
}
bool ext_directory_vnode::__parse_entries(size_t bs)
{
    // a directory in ext is essentially another file consisting of a series of directory entries, which correspond pretty much exactly with the tnodes we've already been using.
    try
    {
        for(size_t i = 0; i < extents.total_extent; i++)
        {
            char* current_pos = __beg() + bs * i;
            ext_dir_entry* dirent;
            for(size_t n = 0; n < bs; n += dirent->entry_size)
            {
                dirent = reinterpret_cast<ext_dir_entry*>(current_pos + n);
                if(!dirent->inode_idx) break;
                tnode* tn = directory_tnodes.emplace(parent_fs->dirent_to_vnode(dirent), std::move(std::string(dirent->name, dirent->name_len))).first.base();
                __dir_index.insert_or_assign(tn, std::move(dirent_idx{ .block_num = static_cast<uint32_t>(i), .block_offs = static_cast<uint32_t>(n) }));
            }
        }
        qword timestamp = sys_time(nullptr);
        on_disk_node->accessed_time = timestamp.lo;
        on_disk_node->access_time_hi = (timestamp.hi.hi.hi >> 4) & 0x03;
        return update_inode();
    }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}
bool ext_directory_vnode::init_dir_blank(ext_directory_vnode* parent)
{
    size_t bs = parent_fs->block_size();
    extents.has_init = true;
    if(disk_block* db = parent_fs->claim_blocks(this))
    {
        if(!expand_buffer(bs * db->chain_len)) return false;
        __setc(0UZ);
        ext_dir_entry* dirent = __current_ent();
        dirent->entry_size = 12;
        __write_dir_entry(this, dti_dir, ".", 1);
        __bumpc(12L);
        dirent = __current_ent();
        dirent->entry_size = bs - 24; // need 12 bytes at the end for the checksum
        __write_dir_entry(parent, dti_dir, "..", 2);
        mark_write(__cur());
        parent->on_disk_node->referencing_dirents++;
        on_disk_node->referencing_dirents++;
        directory_tnodes.emplace(this, ".");
        directory_tnodes.emplace(parent, "..");
        return (__initialized =  parent->fsync());
    }
    return false;
}