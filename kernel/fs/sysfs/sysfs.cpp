#include <fs/sysfs.hpp>
#include <stdexcept>
static bool verify_dirent_csum(sysfs_dir_entry const& ent) { return ent.checksum == crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(ent)), offsetof(sysfs_dir_entry, checksum)); }
sysfs_index_file& sysfs::__index() { return *reinterpret_cast<sysfs_index_file*>(__index_file.data()); }
sysfs_directory_file& sysfs::__dir() { return *reinterpret_cast<sysfs_directory_file*>(__directory_file.data()); }
sysfs_extents_file& sysfs::__extents() { return *reinterpret_cast<sysfs_extents_file*>(__extents_file.data()); }
sysfs_data_file_header& sysfs::__header() { return *reinterpret_cast<sysfs_data_file_header*>(__data_file.data()); }
sysfs_data_block& sysfs::__block(size_t num) { return *reinterpret_cast<sysfs_data_block*>(std::addressof(__data_file.data()[sizeof(sysfs_data_file_header) + num * sizeof(sysfs_data_block)])); }
size_t sysfs::__num_blocks() const { return (__data_file.size() - sizeof(sysfs_data_file_header)) / sysfs_data_block_size; }
sysfs_extent_branch const& sysfs::get_extent_branch(size_t idx) const { return static_cast<sysfs_extent_branch const&>(const_cast<sysfs*>(this)->get_extent_branch(idx)); }
sysfs_inode const& sysfs::get_inode(size_t ino) const { return static_cast<sysfs_inode const&>(const_cast<sysfs*>(this)->get_inode(ino)); }
sysfs_dir_entry const& sysfs::get_dir_entry(size_t num) const { return static_cast<sysfs_dir_entry const&>(const_cast<sysfs*>(this)->get_dir_entry(num)); }
sysfs::sysfs(filesystem* backend, sysfs_file_paths const& files) :
	__backend			{ *backend },
	__data_file			{ *__backend.open_file(files.data_file) },
	__index_file		{ *__backend.open_file(files.index_file) },
	__extents_file		{ *__backend.open_file(files.extents_file) },
	__directory_file	{ *__backend.open_file(files.directory_file) },
	__directory_map		{ 64UZ },
	__opened_nodes		{ 64UZ }
						{}
sysfs::~sysfs()
{
	__backend.close_file(std::addressof(__data_file));
	__backend.close_file(std::addressof(__index_file));
	__backend.close_file(std::addressof(__extents_file));
	__backend.close_file(std::addressof(__directory_file));
}
void sysfs::write_data(size_t st_block, const char* data, size_t n)
{
	if(__unlikely(!st_block))
		throw std::out_of_range("[FS/SYSFS] block number must be at least 1");
	if(__unlikely(st_block > __num_blocks()))
		throw std::out_of_range("[FS/SYSFS] block number " + std::to_string(st_block) + " does not exist");
	--st_block;
	array_copy(__block(st_block), data, n);
	__data_file.force_write();
}
void sysfs::read_data(char* out, size_t st_block, size_t n)
{
	if(__unlikely(!st_block))
		throw std::out_of_range("[FS/SYSFS] block number must be at least 1");
	if(__unlikely(st_block > __num_blocks()))
		throw std::out_of_range("[FS/SYSFS] block number " + std::to_string(st_block) + " does not exist");
	--st_block;
	array_copy<char>(out, __block(st_block), n);
}
bool sysfs::sync()
{
	if(__unlikely(!__data_file.fsync())) panic("[FS/SYSFS] failed to sync data file");
	else if(__unlikely(!__index_file.fsync())) panic("[FS/SYSFS] failed to sync index file");
	else if(__unlikely(!__extents_file.fsync())) panic("[FS/SYSFS] failed ty sync extents file");
	else if(__unlikely(!__directory_file.fsync())) panic("[FS/SYSFS] failed to sync directory file");
	else return __backend.pubsyncdirs(), true;
	return false;
}
sysfs_extent_branch& sysfs::get_extent_branch(size_t idx)
{
	if(__unlikely(!idx))
		throw std::out_of_range("[FS/SYSFS] extent branch index must be at least 1");
	if(__unlikely(idx > __extents().total_branches))
		throw std::out_of_range("[FS/SYSFS] extent branch number " + std::to_string(idx) + " does not exist");
	--idx;
	 __extents_file.force_write();
	return addr_t(std::addressof(__extents())).plus(offsetof(sysfs_extents_file, branches) + idx * sizeof(sysfs_extent_branch)).deref<sysfs_extent_branch>();
}
sysfs_inode& sysfs::get_inode(size_t ino)
{
	if(__unlikely(!ino))
		throw std::out_of_range("[FS/SYSFS] inode index must be at least 1");
	if(__unlikely(ino > __index().total_inodes))
		throw std::out_of_range("[FS/SYSFS] inode number " + std::to_string(ino) + " does not exist");
	--ino;
	__index_file.force_write();
	return __index().inodes[ino];
}
sysfs_dir_entry& sysfs::get_dir_entry(size_t num)
{
	if(__unlikely(!num))
		throw std::out_of_range("[FS/SYSFS] directory entry index must be at least 1");
	if(__unlikely(num > __dir().total_entries))
		throw std::out_of_range("[FS/SYSFS] directory entry number " + std::to_string(num) + " does not exist");
	--num;
	__directory_file.force_write();
	return __dir().entries[num];
}
uint32_t sysfs::add_extent_branch()
{
	uint32_t n = static_cast<uint32_t>(__extents().total_branches) + 1;
	if(__extents_file.grow(sizeof(sysfs_extent_branch)))
	{
		sysfs_extents_file& ext	= __extents();
		ext.total_branches++;
		ext.header_checksum 	= 0U;
		uint32_t csum			= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(ext)), offsetof(sysfs_extents_file, header_checksum));
		ext.header_checksum 	= csum;
		return n;
	}
	return 0U;
}
uint32_t sysfs::add_blocks(uint16_t how_many)
{
	uint32_t n = static_cast<uint32_t>(__num_blocks()) + 1;
	if(__data_file.grow(how_many * sysfs_data_block_size))
	{
		sysfs_data_file_header& hdr	= __header();
		hdr.total_size				+= how_many * sysfs_data_block_size;
		hdr.header_checksum			= 0U;
		uint32_t csum				= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(hdr)), offsetof(sysfs_data_file_header, header_checksum));
		hdr.header_checksum			= csum;
		return n;
	}
	return 0U;
}
uint32_t sysfs::add_inode()
{
	uint32_t n	= static_cast<uint32_t>(__index().total_inodes) + 1;
	if(__index_file.grow(sizeof(sysfs_inode)))
	{
		sysfs_index_file& idx	= __index();
		idx.total_inodes++;
		idx.header_checksum		= 0U;
		uint32_t csum			= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(idx)), offsetof(sysfs_index_file, header_checksum));
		idx.header_checksum		= csum;
		return n;
	}
	return 0U;
}
uint32_t sysfs::add_directory_entry()
{
	uint32_t n	= static_cast<uint32_t>(__dir().total_entries) + 1;
	if(__directory_file.grow(sizeof(sysfs_dir_entry)))
	{
		sysfs_directory_file& dir	= __dir();
		dir.total_entries++;
		dir.header_checksum			= 0U;
		uint32_t csum				= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(dir)), offsetof(sysfs_directory_file, header_checksum));
		dir.header_checksum			= csum;
		return n;
	}
	return 0U;
}
sysfs_extent_branch& sysfs::extend_to_leaf(size_t from_idx, uint32_t ordinal)
{
	if(sysfs_extent_branch& br	= get_extent_branch(from_idx); !br.depth) return br;
	uint32_t next				= add_extent_branch();
	if(__unlikely(!next)) throw std::runtime_error("[FS/SYSFS] failed to expand extents file");
	sysfs_extent_branch& br		= get_extent_branch(from_idx);
	sysfs_extent_branch* added	= new(std::addressof(get_extent_branch(next))) sysfs_extent_branch
	{
		.depth		{ br.depth - 1Z },
		.entries	{},
		.checksum	{}
	};
	uint32_t csum               = crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(added), offsetof(sysfs_extent_branch, checksum));
	added->checksum             = csum;
	new(br.entries) sysfs_extent_entry
	{
		.ordinal	{ ordinal },
		.length		{},
		.start		{ next }
	};
	return extend_to_leaf(next, ordinal);
}
std::pair<sysfs_extent_branch*, size_t> sysfs::next_available_extent_entry(size_t from_idx)
{
	sysfs_extent_branch& br = get_extent_branch(from_idx);
	if(!br.depth)
	{
		for(size_t i = 0; i < sysfs_extent_branch::num_entries; i++)
			if(!br.entries[i].start)
				return std::pair(std::addressof(br), i);
		return std::pair(nullptr, 0UZ);
	}
	if(!br.entries[0].start) return std::pair(std::addressof(br), 0UZ);
	for(size_t i = 1; i < sysfs_extent_branch::num_entries; i++)
	{
		if(!br.entries[i].start)
		{
			std::pair<sysfs_extent_branch*, size_t> result = next_available_extent_entry(br.entries[i - 1].start);
			if(!result.first)
				return std::pair(std::addressof(br), i);
			return result;
		}
	}
	return std::pair(nullptr, 0UZ);
}
void sysfs::dir_add_object(std::string const& name, uint32_t ino)
{
	uint32_t eno			= add_directory_entry();
	if(__unlikely(!eno)) throw std::runtime_error("[FS/SYSFS] failed to expand directory file");
	std::pair<std::string, uint32_t> map_entry(name, ino);
	sysfs_dir_entry& ent	= get_dir_entry(eno);
	ent.inode_number		= ino;
	std::strncpy(ent.object_name, name.c_str(), sysfs_object_name_size_max);
	ent.checksum			= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(ent)), offsetof(sysfs_dir_entry, checksum));
	__directory_map.insert(map_entry);
}
uint32_t sysfs::find_node(std::string const& name)
{
	std::unordered_map<std::string, uint32_t>::iterator result = __directory_map.find(name);
	if(result != __directory_map.end()) return result->second;
	return 0U;
}
void sysfs::init_blank(sysfs_backup_filenames const& bak)
{
	size_t needed_data		= sizeof(sysfs_data_file_header) - std::min(__data_file.size(), sizeof(sysfs_data_file_header));
	size_t needed_index		= sizeof(sysfs_index_file) - std::min(__index_file.size(), sizeof(sysfs_index_file));
	size_t needed_extents	= sizeof(sysfs_extents_file) - std::min(__extents_file.size(), sizeof(sysfs_extents_file));
	size_t needed_directory	= sizeof(sysfs_directory_file) - std::min(__directory_file.size(), sizeof(sysfs_directory_file));
	if((needed_data && !__data_file.grow(needed_data)) || (needed_index && !__index_file.grow(needed_index)) || (needed_extents && !__extents_file.grow(needed_extents)) || (needed_directory && !__directory_file.grow(needed_directory)))
		throw std::runtime_error("[FS/SYSFS] no space on disk for storage");
	sysfs_data_file_header* data	= new(std::addressof(__header())) sysfs_data_file_header();
	sysfs_index_file* idx			= new(std::addressof(__index())) sysfs_index_file();
	sysfs_extents_file* exts		= new(std::addressof(__extents())) sysfs_extents_file();
	sysfs_directory_file* dirfile	= new(std::addressof(__dir())) sysfs_directory_file();
	array_copy(data->backup_file_name, bak.data_backup_file_name, 16UZ);
	array_copy(idx->backup_file_name, bak.index_backup_file_name, 16UZ);
	array_copy(exts->backup_file_name, bak.extents_backup_file_name, 16UZ);
	array_copy(dirfile->backup_file_name, bak.directory_backup_file_name, 16UZ);
	data->header_checksum			= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(data), offsetof(sysfs_data_file_header, header_checksum));
	idx->header_checksum			= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(idx), offsetof(sysfs_index_file, header_checksum));
	exts->header_checksum			= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(exts), offsetof(sysfs_extents_file, header_checksum));
	dirfile->header_checksum		= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(dirfile), offsetof(sysfs_directory_file, header_checksum));
	__data_file.force_write();
	__index_file.force_write();
	__extents_file.force_write();
	__directory_file.force_write();
	sync();
}
void sysfs::init_load()
{
	sysfs_directory_file& dirfile	= __dir();
	for(size_t i		= 0UZ; i < dirfile.total_entries; i++)
	{
		uint32_t ino	= dirfile.entries[i].inode_number;
		if(verify_dirent_csum(dirfile.entries[i]))
			__directory_map.insert(std::make_pair(std::string(dirfile.entries[i].object_name, std::strnlen(dirfile.entries[i].object_name, sizeof(sysfs_dir_entry::object_name))), ino));
		else xklog("[FS/SYSFS] W: directory entry checksum of " + std::to_string(dirfile.entries[i].checksum, std::ext::hex) + " is incorrect; skipping");
	}
}
sysfs_vnode& sysfs::open(uint32_t ino)
{
	std::unordered_map<uint32_t, sysfs_vnode>::iterator result = __opened_nodes.find(ino);
	if(result != __opened_nodes.end())
		return result->second;
	if(ino > __index().total_inodes)
		throw std::out_of_range("[FS/SYSFS] inode number " + std::to_string(ino) + " does not exist");
	return __opened_nodes.emplace(std::piecewise_construct, std::tuple<uint32_t>(ino), std::forward_as_tuple(*this, ino)).first->second;
}
uint32_t sysfs::mknod(std::string const& name, sysfs_object_type type)
{
	uint32_t result		= add_inode();
	if(!result) throw std::runtime_error("[FS/SYSFS] failed to expand index file");
	sysfs_inode& node	= get_inode(result);
	node.type			= type;
	node.checksum		= 0U;
	uint32_t csum		= crc32c_x86_3way(~0U, reinterpret_cast<uint8_t const*>(std::addressof(node)), offsetof(sysfs_inode, checksum));
	node.checksum		= csum;
	dir_add_object(name, result);
	return result;
}