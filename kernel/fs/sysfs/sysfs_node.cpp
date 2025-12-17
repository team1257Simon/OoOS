#include <fs/sysfs.hpp>
#include <sys/errno.h>
sysfs_inode& sysfs_vnode::inode() { return parent_fs.get_inode(ino); }
sysfs_inode const& sysfs_vnode::inode() const { return parent_fs.get_inode(ino); }
sysfs& sysfs_vnode::parent() { return parent_fs; }
bool sysfs_vnode::sync_parent() { return parent_fs.sync(); }
uint32_t sysfs_vnode::inode_number() const { return ino; }
std::streamsize sysfs_vnode::sector_size() const { return sysfs_data_block_size; }
void* sysfs_vnode::raw_data() { return data(); }
const void* sysfs_vnode::raw_data() const { return data(); }
bool sysfs_vnode::expand_by_size(size_t added) { return on_overflow(added) != 0UZ; }
size_t sysfs_vnode::block_capacity() const { return extent_tree.total_extent() * sector_size(); }
void sysfs_vnode::on_modify() {
	calc_inode_csum();
	__base::on_modify();
}
void sysfs_vnode::calc_inode_csum()
{
	sysfs_inode& n	= inode();
	n.checksum		= 0U;
	uint32_t csum	= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t*>(std::addressof(n)), offsetof(sysfs_inode, checksum));
	n.checksum		= csum;
}
std::streamsize sysfs_vnode::on_overflow(std::streamsize n)
{
	size_t target	= inode().size_bytes + n;
	size_t cur_cap	= block_capacity();
	if(target >= cur_cap)
	{
		size_t over	= std::max(1UZ, static_cast<size_t>(target - cur_cap));
		try { extent_tree.push(div_round_up(static_cast<uint16_t>(over), sector_size())); }
		catch(std::exception& e) { panic(e.what()); return 0UZ; }
	}
	inode().size_bytes += n;
	if(__grow_buffer_exact(n)) return n;
	throw std::bad_alloc();
}
std::streamsize sysfs_vnode::xsputn(char const* s, std::streamsize n)
{
	std::streamsize st_block	= sector_of(tell());
	std::streamsize result		= __base::xsputn(s, n);
	if(__unlikely(!result)) return 0UZ;
	std::streamsize max_block	= sector_of(tell()) + 1Z;
	dirty_blocks.push_back({ st_block, max_block });
	inode().size_bytes			= std::max(inode().size_bytes, static_cast<size_t>(tell()));
	on_modify();
	return result;
}
int sysfs_vnode::write_dev()
{
	if(dirty_blocks.empty()) return 0;
	size_t ss					= sector_size();
	try
	{
		for(block_range const& b : dirty_blocks)
		{
			for(size_t i		= b.start; i < b.end; i++)
			{
				uint32_t actual	= actual_block(i);
				const char* ptr	= sector_ptr(i);
				parent_fs.write_data(actual, ptr, std::min(ss, static_cast<size_t>(__max() - ptr)));
			}
		}
	}
	catch(std::exception& e) { panic(e.what()); return -EINVAL; }
	dirty_blocks.clear();
	return 0;
}
void sysfs_vnode::init()
{
	if(size_t ext			= extent_tree.total_extent())
	{
		if(!__grow_buffer_exact(block_capacity())) throw std::bad_alloc();
		for(size_t i		= 0UZ; i < ext; i++)
		{
			uint32_t actual	= extent_tree[i].start;
			for(size_t j	= 0UZ; j < extent_tree[i].length; j++, actual++)
				parent_fs.read_data(sector_ptr(actual), actual, sector_size());
		}
		__setc(inode().size_bytes);
		sync_ptrs();
	}
}
uint32_t sysfs_vnode::actual_block(size_t ordinal) const
{
	sysfs_extent_entry const& e	= extent_tree[ordinal];
	int diff					= ordinal - e.ordinal;
	return e.start + diff;
}
sysfs_vnode::pos_type sysfs_vnode::commit(size_t target_pos)
{
	if(__unlikely(target_pos > __capacity())) return pos_type(off_type(-1Z));
	uint32_t pos_block	= sector_of(target_pos) + 1Z;
	dirty_blocks.push_back({ 0UZ, pos_block });
	is_dirty			= true;
	return pos_type(pos_block * sector_size());
}
bool sysfs_vnode::expand_to_size(size_t target)
{
	if(__unlikely(target <= __capacity()))
		return true;
	return on_overflow(static_cast<std::streamsize>(target - __capacity())) != 0UZ;
}
sysfs_vnode::sysfs_vnode(sysfs& sysfs_parent, uint32_t inode_num) :
	parent_fs		{ sysfs_parent },
	ino				{ inode_num },
	extent_tree		{ *this },
	dirty_blocks	{}
					{ init(); }