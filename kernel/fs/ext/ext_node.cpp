#include "fs/ext.hpp"
static inline size_t unused_dirent_space(ext_dir_entry const& de) { return static_cast<size_t>(static_cast<size_t>(de.entry_size) - static_cast<size_t>(de.name_len + 8UL)); }
std::streamsize ext_directory_vnode::__ddread(std::streamsize n) { return std::streamsize(0); }
std::streamsize ext_directory_vnode::__ddrem() { return std::streamsize(0); }
uint64_t ext_directory_vnode::num_files() const noexcept { return __n_files; }
uint64_t ext_directory_vnode::num_subdirs() const noexcept { return __n_subdirs; }
std::vector<std::string> ext_directory_vnode::lsdir() const { std::vector<std::string> result{}; for(tnode_dir::const_iterator i = __my_dir.begin(); i != __my_dir.end(); i++) result.push_back(i->name()); return result; }
tnode *ext_directory_vnode::find(std::string const& name) { if(tnode_dir::iterator i = __my_dir.find(name); i != __my_dir.end()) { if(ext_directory_vnode* d = dynamic_cast<ext_directory_vnode*>(i->ptr())) { if(!d->initialize()) { panic("directory init failed"); return nullptr; } }  return i.base(); } else return nullptr; }
bool ext_directory_vnode::fsync() { return parent_fs->persist(this); }
ext_directory_vnode::ext_directory_vnode(extfs *parent, uint32_t inode_number) : ext_vnode{ parent, inode_number }, directory_node{ "", inode_number } { mode = on_disk_node->mode; }
ext_directory_vnode::ext_directory_vnode(extfs *parent, uint32_t inode_number, ext_inode *inode_data) : ext_vnode{ parent, inode_number, inode_data }, directory_node{ "", inode_number } { mode = on_disk_node->mode; }
tnode *ext_directory_vnode::add(fs_node* n) { return __my_dir.emplace(n, std::to_string(n->cid())).first.base(); /* fast-track tnodes for newly-created files; dirents will be separately made */ }
ext_file_vnode::size_type ext_file_vnode::read(pointer dest, size_type n) { return this->sgetn(dest, n); }
ext_file_vnode::pos_type ext_file_vnode::seek(off_type off, std::ios_base::seekdir way) { return this->data_buffer<char>::seekoff(off, way); }
ext_file_vnode::pos_type ext_file_vnode::seek(pos_type pos) { return this->data_buffer<char>::seekpos(pos); }
bool ext_file_vnode::fsync() { return parent_fs->persist(this); }
uint64_t ext_file_vnode::size() const noexcept { return qword(on_disk_node->size_lo, on_disk_node->size_hi); }
ext_file_vnode::pos_type ext_file_vnode::tell() const { return this->vfs_filebuf_base<char>::tell(); }
ext_file_vnode::~ext_file_vnode() = default;
ext_file_vnode::ext_file_vnode(extfs *parent, uint32_t inode_number, int fd) : ext_vnode{ parent, inode_number }, file_node{ "", fd, inode_number } { mode = on_disk_node->mode; }
ext_file_vnode::ext_file_vnode(extfs *parent, uint32_t inode_number, ext_inode *inode_data, int fd) : ext_vnode{ parent, inode_number, inode_data }, file_node{ "", fd, inode_number } { mode = on_disk_node->mode; }
ext_vnode::ext_vnode(extfs *parent, uint32_t inode_number) : ext_vnode{ parent, inode_number, parent->read_inode(inode_number) } {}
bool ext_vnode::initialize() { return init_extents(); }
std::streamsize ext_file_vnode::__ddrem() { return std::streamsize(size() - (this->__capacity())); }
ext_vnode::~ext_vnode() = default;
int ext_vnode::__ddwrite() { return 0; /* disk r/w through fsync only */ }
size_t ext_vnode::block_of_data_ptr(size_t offs) { return offs / parent_fs->block_size(); }
uint64_t ext_vnode::next_block() { return block_data[last_checked_block_idx + 1].block_number; }
ext_vnode::ext_vnode(extfs *parent, uint32_t inode_num, ext_inode *inode) :
    vfs_filebuf_base<char>      {}, 
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
    size_t acc_offs = 0;
    size_t bs = parent_fs->block_size();
    for(size_t i = 0; i < block_data.size() && acc_offs < this->__capacity(); i++) { block_data[i].data_buffer = (this->__beg() + acc_offs); acc_offs += block_data[i].chain_len * bs; }
}
bool ext_vnode::expand_buffer(size_t added_bytes)
{
    if(!this->__grow_buffer(added_bytes)) return false;
    update_block_ptrs();
    qword fsz(on_disk_node->size_lo, on_disk_node->size_hi);
    fsz += added_bytes;
    on_disk_node->size_lo = fsz.lo;
    on_disk_node->size_hi = fsz.hi;
    return parent_fs->persist_inode(inode_number);
}
std::streamsize ext_file_vnode::__overflow(std::streamsize n)
{
    size_t bs = parent_fs->block_size();
    size_t needed = div_roundup(n, bs);
    if(disk_block *blk = parent_fs->claim_blocks(this, needed)) { if(!expand_buffer(bs * blk->chain_len)) return 0; return bs * blk->chain_len; }
    return 0;
}
std::streamsize ext_file_vnode::__ddread(std::streamsize n)
{
    if(last_checked_block_idx + 1 < block_data.size())
    {
        size_t bs = parent_fs->block_size();
        disk_block& bl = block_data[++last_checked_block_idx];
        if(!this->__grow_buffer(bs * bl.chain_len)) return 0;
        update_block_ptrs();
        if(parent_fs->read_from_disk(bl)) return std::min(n, bs * bl.chain_len);
    }
    return 0;
}
ext_file_vnode::size_type ext_file_vnode::write(const_pointer src, size_type n) 
{
    size_type result = this->sputn(src, n);
    for(std::vector<disk_block>::iterator i = block_data.begin(); i != block_data.end(); i++) i->dirty = true;   
    if(!fsync()) return 0UL;
    return result;
}
bool ext_file_vnode::initialize()
{
    if(!init_extents()) return false;
    if(on_disk_node->size_lo || on_disk_node->size_hi)
    {
        if(!this->__grow_buffer(extents.total_extent * parent_fs->block_size())) return false;
        update_block_ptrs();
        for(std::vector<disk_block>::iterator i = block_data.begin(); i != block_data.end(); i++) { if(!parent_fs->read_from_disk(*i)) { panic("block read failed"); return false; } }   
    }
    return true;
}
std::streamsize ext_directory_vnode::__overflow(std::streamsize n)
{
    size_t bs = parent_fs->block_size();
    size_t needed = div_roundup(n, bs);
    if(disk_block* blk = parent_fs->claim_blocks(this, needed))
    {
        if(!expand_buffer(bs * blk->chain_len)) return 0;
        // the data pointer of returned block should point at the newly added space
        this->__setc(blk->data_buffer);
        array_zero(this->__cur(), bs);
        ext_dir_entry* dirent = reinterpret_cast<ext_dir_entry*>(this->__cur());
        size_t rem = static_cast<size_t>(bs - n);
        dirent->entry_size = n;
        reinterpret_cast<ext_dir_entry*>(this->__cur() + n)->entry_size = rem;
        return bs * blk->chain_len;
    }
    return 0;
}
bool ext_directory_vnode::__seek_available_entry(size_t name_len)
{
    size_t bs = parent_fs->block_size();
    for(size_t i = block_of_data_ptr(this->tell()); i < block_data.size(); i++)
    {
        this->__setc(block_data[i].data_buffer);
        size_t tbl = (bs * block_data[i].chain_len);
        ext_dir_entry* dirent;
        for(size_t n = 0; n < tbl; n += dirent->entry_size)
        {
            dirent = reinterpret_cast<ext_dir_entry*>(this->__cur() + n);
            this->__setc(reinterpret_cast<char*>(dirent));
            if(!dirent->inode_idx && dirent->entry_size >= name_len + 8UL) return true; // already-usable entry
            else if(unused_dirent_space(*dirent) >= name_len + 8UL) return false; // have to divide the entry
        }
    }
    this->__setc(this->__max());
    return false; // indicate no open entries by having the pointer at the end of the directory file and returning false
}
void ext_directory_vnode::__write_dir_entry(ext_vnode *vnode, ext_dirent_type type, const char *name, size_t name_len)
{
    ext_dir_entry* dirent = reinterpret_cast<ext_dir_entry*>(this->__cur());
    dirent->name_len = name_len;
    dirent->type_ind = type;
    arraycopy(dirent->name, name, name_len);
    dirent->inode_idx = vnode->inode_number;
    this->block_data[block_of_data_ptr(this->tell())].dirty = true;
}
bool ext_directory_vnode::add_dir_entry(ext_vnode *vnode, ext_dirent_type type, const char *nm, size_t name_len)
{
    if(!__seek_available_entry(name_len)) 
    {
        if(__cur() < __max())
        {
            ext_dir_entry* dirent = reinterpret_cast<ext_dir_entry*>(this->__cur());
            size_t rem = unused_dirent_space(*dirent);
            size_t nsz = dirent->name_len + 8UL;
            dirent->entry_size = nsz;
            this->__setc(reinterpret_cast<char*>(dirent + nsz));
            reinterpret_cast<ext_dir_entry*>(this->__cur())->entry_size = rem;
        }
        else if(!this->__overflow(name_len + 8)) return false;
    }
    __write_dir_entry(vnode, type, nm, name_len);
    vnode->on_disk_node->referencing_dirents++;
    if(type == dti_dir) __n_subdirs++;
    else __n_files++;
    if(fs_node* fn = dynamic_cast<fs_node*>(vnode)) { __my_dir.insert(tnode(fn, nm)); }
    return parent_fs->persist_inode(vnode->inode_number) && parent_fs->persist(this);
}
bool ext_directory_vnode::link(tnode* original, std::string const& target)
{
    ext_dirent_type type = original->is_file() ? dti_regular : dti_dir;
    if(ext_vnode* vnode = dynamic_cast<ext_vnode*>(original->ptr())) return add_dir_entry(vnode, type, target.data(), target.size());
    // TODO device hardlinks
    return false;
}
bool ext_directory_vnode::unlink(std::string const& name)
{
    if(tnode_dir::iterator i = __my_dir.find(name); i != __my_dir.end())
    {
        if(std::map<tnode*, dirent_idx>::iterator j = __dir_index.find(i.base()); j != __dir_index.end())
        {
            this->__setc(block_data[j->second.block_num].data_buffer);
            ext_dir_entry* dirent = reinterpret_cast<ext_dir_entry*>(this->__cur() + j->second.block_offs);
            array_zero(dirent->name, dirent->name_len);
            dirent->name_len = 0;
            dirent->inode_idx = 0;
            dirent->type_ind = 0;
            block_data[j->second.block_num].dirty = true;
            __dir_index.erase(j);
            if(ext_vnode* vn = dynamic_cast<ext_vnode*>(i->ptr())) { vn->on_disk_node->referencing_dirents--; if(!parent_fs->persist_inode(vn->inode_number)) return false; }
            return parent_fs->persist(this);
        }
    }
    return false;
}
bool ext_directory_vnode::initialize()
{
    if(__initialized) return true;
    size_t bs = parent_fs->block_size();
    if(!init_extents() || !this->__grow_buffer(extents.total_extent * bs)) return false;
    for(size_t i = 0, n = 0; i < block_data.size(); i++)
    {
        block_data[i].data_buffer = this->__beg() + n;
        if(!parent_fs->read_from_disk(block_data[i])) { panic("read failed on directory block"); return false; }
        n += block_data[i].chain_len * bs;
    }
    return __parse_entries(bs);
}
bool ext_directory_vnode::__parse_entries(size_t bs)
{
    // a directory in ext is essentially another file consisting of a series of directory entries, which correspond pretty much exactly with the tnodes we've already been using.
    try
    {
        for(size_t i = 0; i < block_data.size(); i++)
        {
            char* current_pos = block_data[i].data_buffer;
            ext_dir_entry* dirent;
            for(size_t n = 0; n < bs; n += dirent->entry_size)
            {
                dirent = reinterpret_cast<ext_dir_entry*>(current_pos + n);
                if(!dirent->inode_idx) break;
                tnode* tn = __my_dir.emplace(parent_fs->dirent_to_vnode(dirent), std::string(dirent->name, dirent->name_len)).first.base();
                __dir_index.insert_or_assign(tn, dirent_idx{ .block_num = static_cast<uint32_t>(i), .block_offs = static_cast<uint32_t>(n) });
            }
        }
        return true;
    }
    catch(std::exception& e) { panic(e.what()); }
    return false;
}