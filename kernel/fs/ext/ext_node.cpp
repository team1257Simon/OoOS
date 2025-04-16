#include "fs/ext.hpp"
#include "kdebug.hpp"
static std::allocator<char> ch_alloc{};
static inline size_t unused_dirent_space(ext_dir_entry const& de) { return static_cast<size_t>(static_cast<size_t>(de.entry_size) - static_cast<size_t>(de.name_len + 8UL)); }
ext_dir_entry* ext_directory_vnode::__current_ent() { return reinterpret_cast<ext_dir_entry*>(__cur()); }
char* ext_directory_vnode::__current_block_start() { return __get_ptr(block_of_data_ptr(tell()) * parent_fs->block_size()); }
uint64_t ext_directory_vnode::num_files() const noexcept { return __n_files; }
uint64_t ext_directory_vnode::num_subdirs() const noexcept { return __n_subdirs; }
std::vector<std::string> ext_directory_vnode::lsdir() const { std::vector<std::string> result{}; for(tnode const& tn : __my_dir) result.push_back(tn.name()); return result; }
bool ext_directory_vnode::fsync() { return parent_fs->persist(this); }
ext_directory_vnode::ext_directory_vnode(extfs* parent, uint32_t inode_number) : ext_vnode(parent, inode_number), directory_node(std::move(""), inode_number) { mode = on_disk_node->mode; }
ext_directory_vnode::ext_directory_vnode(extfs* parent, uint32_t inode_number, ext_inode* inode_data) : ext_vnode(parent, inode_number, inode_data), directory_node(std::move(""), inode_number) { mode = on_disk_node->mode; }
tnode *ext_directory_vnode::add(fs_node* n) { return __my_dir.emplace(n, std::move(std::to_string(n->cid()))).first.base(); /* fast-track tnodes for newly-created files; dirents will be separately made */ }
ext_file_vnode::size_type ext_file_vnode::read(pointer dest, size_type n) { return sgetn(dest, n); }
ext_file_vnode::size_type ext_file_vnode::write(const_pointer src, size_type n) { size_t result = ext_vnode::xsputn(src, n); return result && fsync() ? result : 0; }
ext_file_vnode::pos_type ext_file_vnode::seek(off_type off, std::ios_base::seekdir way) { return std::ext::dynamic_streambuf<char>::seekoff(off, way); }
ext_file_vnode::pos_type ext_file_vnode::seek(pos_type pos) { return std::ext::dynamic_streambuf<char>::seekpos(pos); }
bool ext_file_vnode::fsync() { return parent_fs->persist(this); }
uint64_t ext_file_vnode::size() const noexcept { return qword(on_disk_node->size_lo, on_disk_node->size_hi); }
ext_file_vnode::pos_type ext_file_vnode::tell() const { return std::ext::dynamic_streambuf<char>::tell(); }
ext_file_vnode::~ext_file_vnode() = default;
ext_file_vnode::ext_file_vnode(extfs* parent, uint32_t inode_number, int fd) : ext_vnode(parent, inode_number), file_node(std::move(""), fd, inode_number) { mode = on_disk_node->mode; }
ext_file_vnode::ext_file_vnode(extfs* parent, uint32_t inode_number, ext_inode* inode_data, int fd) : ext_vnode(parent, inode_number, inode_data), file_node(std::move(""), fd, inode_number) { mode = on_disk_node->mode; }
ext_vnode::ext_vnode(extfs* parent, uint32_t inode_number) : ext_vnode(parent, inode_number, parent->get_inode(inode_number)) {}
bool ext_vnode::initialize() { return init_extents(); }
void ext_vnode::mark_write(void* pos) { addr_t addr(pos); if(!__out_of_range(addr)) { block_data[block_of_data_ptr(static_cast<size_t>(addr - addr_t(__beg())))].dirty = true; } }
ext_vnode::~ext_vnode() = default;
void ext_vnode::on_modify() { if(__beg()) { __fullsetp(__beg(), __cur(), __max()); setg(__beg(), __cur(), __max()); } }
size_t ext_vnode::block_of_data_ptr(size_t offs) { return offs / parent_fs->block_size(); }
uint64_t ext_vnode::next_block() { return block_data[last_checked_block_idx + 1].block_number; }
tnode* ext_directory_vnode::find(std::string const& name) 
{
    if(!initialize()) return nullptr; 
    if(tnode_dir::iterator i = __my_dir.find(name); i != __my_dir.end()) 
    {
        if(ext_vnode* vn = dynamic_cast<ext_vnode*>(i->ptr()); vn && vn->is_symlink())
        {
            if(vn->on_disk_node->block_info.ext4_extent.header.magic == ext_extent_magic) 
            {
                if(!vn->initialize()) throw std::runtime_error{ "symlink could not be initialized" };
                size_t n = vn->count();
                tnode* result = nullptr;
                char* buff = ch_alloc.allocate(n);
                if(vn->sgetn(buff, n)) result = parent_fs->get_path(std::move(std::string(buff, n)));
                ch_alloc.deallocate(buff, n);
                if(!result) throw std::runtime_error{ "bad symlink" };
                return result;
            }
            char* link_str = vn->on_disk_node->block_info.link_target;
            std::string separator = parent_fs->get_path_separator();
            std::string xlink_str(link_str, std::strnlen(link_str, 60));
            // Relative path...
            if(std::strncmp(separator.data(), link_str, separator.size())) { return find(xlink_str); }
            // Absolute path...
            else if(tnode* result = parent_fs->get_path(xlink_str)) return result;
            else throw std::runtime_error{ "bad symlink" };
        }
        else return i.base(); // devie node or regular file/dirnode
    } 
    else return nullptr; 
}
ext_vnode::ext_vnode(extfs* parent, uint32_t inode_num, ext_inode* inode) :
    base_buffer                 {}, 
    inode_number                { inode_num },
    parent_fs                   { parent }, 
    on_disk_node                { inode },
    extents                     { this }
                                {}
bool ext_vnode::init_extents()
{
    if((on_disk_node->flags & use_extents) ? extents.parse_ext4() : extents.parse_legacy()) return true;
    for(size_t i = 0; i < cached_metadata.size(); i++) { parent_fs->free_block_buffer(cached_metadata[i]); }
    return false;
}
void ext_vnode::update_block_ptrs()
{
    if(!__beg()) return;
    on_modify();
    size_t bs = parent_fs->block_size();
    char* pos = __beg();
    for(size_t i = 0; i < block_data.size() && pos < __max(); i++) { block_data[i].data_buffer = pos; pos += block_data[i].chain_len * bs; }
}
bool ext_vnode::expand_buffer(size_t added_bytes)
{
    if(!__grow_buffer(added_bytes)) return false;
    qword fsz(on_disk_node->size_lo, on_disk_node->size_hi);
    fsz += added_bytes;
    on_disk_node->size_lo = fsz.lo;
    on_disk_node->size_hi = fsz.hi;
    update_block_ptrs();
    return parent_fs->persist_inode(inode_number);
}
std::streamsize ext_file_vnode::on_overflow(std::streamsize n)
{
    size_t bs = parent_fs->block_size();
    size_t needed = div_round_up(n, bs);
    size_t ccap = __capacity();
    if(disk_block* blk = parent_fs->claim_blocks(this, needed)) 
    { 
        if(!expand_buffer(bs * blk->chain_len)) return 0;
        array_zero(__get_ptr(ccap), bs * blk->chain_len);
        return bs * blk->chain_len; 
    }
    return 0;
}
std::streamsize ext_vnode::xsputn(const char* s, std::streamsize n)
{
    if(n > __capacity() && !on_overflow(static_cast<std::streamsize>(n - __capacity()))) return 0;
    uint64_t sblk = block_of_data_ptr(__size());
    size_t nblk = div_round_up(n, parent_fs->block_size());
    array_copy(__cur(), s, n);
    __advance(n);
    for(size_t i = 0; i < nblk; i++) block_data[sblk + i].dirty = true;
    on_modify();
    return n;
}
bool ext_file_vnode::initialize()
{
    if(!init_extents()) return false;
    if(extents.total_extent)
    {
        if(!__grow_buffer(extents.total_extent * parent_fs->block_size())) return false;
        update_block_ptrs();
        for(disk_block& db : block_data) { if(!parent_fs->read_hd(db)) { panic("block read failed"); return false; } }   
    }
    return true;
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
bool ext_directory_vnode::__seek_available_entry(size_t name_len)
{
    while(__cur() < __max())
    {
        ext_dir_entry* dirent = __current_ent();
        if(!dirent->inode_idx && dirent->entry_size >= name_len + 8UL) return true; // already-usable entry
        else if(unused_dirent_space(*dirent) >= name_len + 8UL) return false; // have to divide the entry
        __bumpc(dirent->entry_size);
    }
    __setc(__max());
    return false; // indicate no open entries by having the pointer at the end of the directory file and returning false
}
void ext_directory_vnode::__write_dir_entry(ext_vnode* vnode, ext_dirent_type type, const char* name, size_t name_len)
{
    ext_dir_entry* dirent = __current_ent();
    size_t bs =  parent_fs->block_size();
    dirent->name_len = name_len;
    dirent->type_ind = type;
    array_copy(dirent->name, name, name_len);
    dirent->inode_idx = vnode->inode_number;
    mark_write(dirent);
    char* cblk = __current_block_start();
    new(static_cast<void*>(cblk + bs - 12)) ext_dir_tail(crc32c(~0U, cblk, bs));
}
bool ext_directory_vnode::add_dir_entry(ext_vnode* vnode, ext_dirent_type type, const char* nm, size_t name_len)
{
    if(!__seek_available_entry(name_len)) 
    {
        if(__cur() < __max())
        {
            ext_dir_entry* dirent = __current_ent();
            size_t rem = unused_dirent_space(*dirent);
            size_t nsz = dirent->name_len + 8UL;
            dirent->entry_size = nsz;
            mark_write(dirent);
            __bumpc(nsz);
            __current_ent()->entry_size = rem;
        }
        else if(!on_overflow(name_len + 8)) return false;
    }
    __write_dir_entry(vnode, type, nm, name_len);
    vnode->on_disk_node->referencing_dirents++;
    if(type == dti_dir) __n_subdirs++;
    else __n_files++;
    if(fs_node* fnode = dynamic_cast<fs_node*>(vnode))
    {
        tnode* tn = __my_dir.emplace(fnode, nm).first.base();
        size_t i = block_of_data_ptr(tell());
        __dir_index.insert_or_assign(tn, std::move(dirent_idx{ .block_num = static_cast<uint32_t>(i), .block_offs = static_cast<uint32_t>(tell() % parent_fs->block_size()) }) );
    }
    return parent_fs->persist(this);
}
bool ext_directory_vnode::link(tnode* original, std::string const& target)
{
    ext_dirent_type type = original->is_file() ? dti_regular : dti_dir;
    if(ext_vnode* vnode = dynamic_cast<ext_vnode*>(original->ptr())) return add_dir_entry(vnode, type, target.data(), target.size()) && parent_fs->persist_inode(vnode->inode_number);
    // TODO device hardlinks
    return false;
}
bool ext_directory_vnode::unlink(std::string const& name)
{
    bool success = false;
    size_t bs = parent_fs->block_size();
    if(tnode_dir::iterator i = __my_dir.find(name); i != __my_dir.end())
    {
        if(std::map<tnode*, dirent_idx>::iterator j = __dir_index.find(i.base()); j != __dir_index.end())
        {
            __setc(__beg() + j->second.block_num * bs + j->second.block_offs);
            ext_dir_entry* dirent = __current_ent();
            array_zero(dirent->name, dirent->name_len);
            dirent->name_len = 0;
            dirent->inode_idx = 0;
            dirent->type_ind = 0;
            mark_write(dirent);
            __dir_index.erase(j);
            if(ext_vnode* vn = dynamic_cast<ext_vnode*>(i->ptr())) { vn->on_disk_node->referencing_dirents--; success = parent_fs->persist_inode(vn->inode_number) && parent_fs->persist(this); }
            else success = parent_fs->persist(this);
        }
    }
    return success;
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
                tnode* tn = __my_dir.emplace(parent_fs->dirent_to_vnode(dirent), std::move(std::string(dirent->name, dirent->name_len))).first.base();
                __dir_index.insert_or_assign(tn, std::move(dirent_idx{ .block_num = static_cast<uint32_t>(i), .block_offs = static_cast<uint32_t>(n) }));
            }
        }
        return true;
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
        ext_dir_entry* dirent = __current_ent();
        dirent->name_len = 1;
        dirent->entry_size = 9;
        dirent->type_ind = dti_dir;
        dirent->inode_idx = inode_number;
        dirent->name[0] = '.';
        __bumpc(9);
        dirent = __current_ent();
        dirent->entry_size = bs - 21;
        __write_dir_entry(parent, dti_dir, "..", 2);
        mark_write(__cur());
        parent->on_disk_node->referencing_dirents++;
        on_disk_node->referencing_dirents++;
        return (__initialized = parent_fs->persist_inode(parent->inode_number));
    }
    return false;
}