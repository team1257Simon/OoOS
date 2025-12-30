#include <fs/fat32.hpp>
#include <rtc.h>
#include <stdexcept>
constexpr size_t dirent_size = sizeof(fat32_directory_entry);
fat32_vnode::fat32_vnode(fat32* pfs, fat32_directory_vnode* pdir, size_t didx) noexcept : parent_fs(pfs), parent_dir(pdir), dirent_index(didx) {}
fat32_vnode::~fat32_vnode() = default;
fat32_file_vnode::~fat32_file_vnode() = default;
fat32_directory_vnode::~fat32_directory_vnode() = default;
fat32_regular_entry* fat32_vnode::disk_entry() noexcept { return std::addressof((parent_dir->__my_dir_data.begin() + dirent_index)->regular_entry); }
fat32_regular_entry const* fat32_vnode::disk_entry() const noexcept { return std::addressof((parent_dir->__my_dir_data.begin() + dirent_index)->regular_entry); }
uint32_t fat32_vnode::start_cluster() const noexcept { return start_of(*disk_entry()); }
uint32_t fat32_file_vnode::claim_next(uint32_t cl) { return claim_cluster(parent_fs->__the_table, cl); }
uint64_t fat32_file_vnode::cl_to_s(uint32_t cl) { return parent_fs->cluster_to_sector(cl); }
uint64_t fat32_file_vnode::size() const noexcept { return __on_disk_size; }
fat32_file_vnode::pos_type fat32_file_vnode::tell() const { return pos_type(__my_filebuf.tell()); }
char* fat32_file_vnode::data() { return __my_filebuf.__beg(); }
fat32_file_vnode::pos_type fat32_file_vnode::seek(pos_type pos) { return __my_filebuf.seekpos(pos); }
fat32_file_vnode::pos_type fat32_file_vnode::seek(off_type off, std::ios_base::seekdir way) { return __my_filebuf.seekoff(off, way); }
fat32_file_vnode::size_type fat32_file_vnode::read(pointer dest, size_type n) { return __my_filebuf.xsgetn(dest, n); }
tnode* fat32_directory_vnode::add(vnode* n) { return directory_tnodes.emplace(n, n->name()).first.base(); }
bool fat32_directory_vnode::link(tnode* original, std::string const& target) { panic("[FS/FAT32] FAT does not support hard links"); return false; }
void fat32_file_vnode::force_write() { __my_filebuf.is_dirty = true; }
void fat32_file_vnode::on_close() { __my_filebuf.release(); }
bool fat32_file_vnode::truncate()
{
	array_zero(__my_filebuf.__beg(), __my_filebuf.__capacity());
	__on_disk_size = 0UZ;
	return fsync();
}
static void update_times(fat32_regular_entry& e)
{
	rtc_time t = rtc::get_instance().get_time();
	new(std::addressof(e.modified_date)) fat_filedate(t.day, t.month, static_cast<uint8_t>(t.year - fat_year_base));
	new(std::addressof(e.modified_time)) fat_filetime(static_cast<uint8_t>(t.sec >> 1), t.min, t.hr);
	new(std::addressof(e.accessed_date)) fat_filedate(e.modified_date);
}
bool fat32_file_vnode::on_open()
{
	if(__on_disk_size)
		return __my_filebuf.read_dev(__on_disk_size) != 0UZ;
	else return __my_filebuf.__grow_buffer(parent_fs->block_size());
}
bool fat32_file_vnode::fsync()
{
	if(__my_filebuf.sync() == 0)
	{
		update_times(*disk_entry());
		disk_entry()->size_bytes = size();
		return true;
	}
	panic("[FS/FAT32] fsync failed");
	return false;
}
fat32_file_vnode::fat32_file_vnode(fat32* pfs, std::string const& real_name, fat32_directory_vnode* pdir, uint32_t cl_st, size_t dirent_idx) :
	file_vnode(real_name, pfs->next_fd++, uint64_t(cl_st)),
	fat32_vnode(pfs, pdir, dirent_idx),
	__my_filebuf(std::vector<uint32_t>{}, this),
	__on_disk_size(disk_entry()->size_bytes)
{
	fat32_regular_entry* e	= disk_entry();
	create_time				= e->created_date + e->created_time;
	modif_time				= e->modified_date + e->modified_time;
	uint32_t cl				= cl_st & fat32_cluster_mask;
	do {
		__my_filebuf.__my_clusters.push_back(cl);
		cl	= parent_fs->__the_table[cl] & fat32_cluster_mask;
	} while(cl < fat32_cluster_eof);
}
fat32_file_vnode::size_type fat32_file_vnode::write(const_pointer src, size_type n)
{
	size_t result					= __my_filebuf.xsputn(src, n);
	this->__my_filebuf.__cur()[0]	= std::char_traits<char>::eof();
	this->__my_filebuf.is_dirty		= true;
	this->__on_disk_size			= size_t(this->tell());
	sys_time(std::addressof(this->modif_time));
	parent_dir->mark_dirty();
	return result;
}
fat32_regular_entry* fat32_directory_vnode::find_dirent(std::string const& name)
{
	if(tnode_dir::iterator i	= directory_tnodes.find(name); i != directory_tnodes.end())
		if(fat32_vnode* n		= dynamic_cast<fat32_vnode*>(i->ptr()); n && n->parent_dir)
			return n->disk_entry();
	return nullptr;
}
fat32_directory_vnode::fat32_directory_vnode(fat32* pfs, std::string const& real_name, fat32_directory_vnode* pdir, uint32_t cl_st, size_t dirent_idx) :
	directory_vnode(real_name, pfs->next_fd++, cl_st),
	fat32_vnode(pfs, pdir, dirent_idx),
	__my_dir_data(),
	__my_covered_clusters()
{
	if(pdir)
	{
		fat32_regular_entry* e = disk_entry();
		create_time = e->created_date + e->created_time;
		modif_time = e->modified_date + e->modified_time;
	}
	uint32_t cl = cl_st;
	do {
		__my_covered_clusters.push_back(cl);
		cl = parent_fs->__the_table[cl] & fat32_cluster_mask;
	} while(cl < fat32_cluster_eof);
}
std::vector<fat32_directory_entry>::iterator fat32_directory_vnode::__whereis(fat32_regular_entry* e)
{
	std::vector<fat32_directory_entry>::iterator i(reinterpret_cast<fat32_directory_entry*>(e));
	if(i < __my_dir_data.end() && i >= __my_dir_data.begin()) return i;
	else return __my_dir_data.end();
}
void fat32_directory_vnode::__expand_dir()
{
	size_t n = parent_fs->block_size() / dirent_size;
	if(uint32_t i = claim_cluster(parent_fs->__the_table, __my_covered_clusters.back())) {
		__my_covered_clusters.push_back(i);
		__my_dir_data.reserve(__my_dir_data.size() + n);
	}
	else throw std::runtime_error("[FS/FAT32] out of space");
}
std::vector<fat32_directory_entry>::iterator fat32_directory_vnode::first_unused_entry()
{
	__dirty = true;
	for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++)
		if(i->regular_entry.filename[0] == 0xE5 || i->regular_entry.filename[0] == 0)
			return i;
	this->__expand_dir();
	return first_unused_entry();
}
std::vector<fat32_directory_entry>::iterator fat32_directory_vnode::__get_longname_start(fat32_regular_entry* e)
{
	std::vector<fat32_directory_entry>::iterator i = __whereis(e), j = i - 1;
	if(!is_longname(*j)) return __my_dir_data.end();
	while(!is_last_longname(j->longname_entry) && j > this->__my_dir_data.begin()) j--;
	return is_longname(*j) && is_last_longname(j->longname_entry) ? j : __my_dir_data.end();
}
bool fat32_file_vnode::grow(size_t added)
{
	if(!__my_filebuf.grow_file(added)) return false;
	__on_disk_size += added;
	return fsync();
}
bool fat32_directory_vnode::__dir_ent_erase(std::string const& what)
{
	if(fat32_regular_entry* e = find_dirent(what))
	{
		std::vector<fat32_directory_entry>::iterator i = __whereis(e), j = __get_longname_start(e);
		if(i == __my_dir_data.end() || j == __my_dir_data.end()) { return false; }
		try
		{
			parent_fs->rm_start_cluster_ref(start_of(*e));
			size_t sz										= __my_dir_data.capacity();
			size_t erased									= std::distance(j, i + 1);
			std::vector<fat32_directory_entry>::iterator r	= __my_dir_data.erase(j, i + 1);
			size_t n										= static_cast<size_t>(r - __my_dir_data.begin());
			__my_dir_data.reserve(sz);
			for(tnode_dir::iterator tn = directory_tnodes.begin(); tn != directory_tnodes.end(); tn++)
				if(fat32_vnode* fn = dynamic_cast<fat32_vnode*>(tn->ptr()); fn->dirent_index >= n)
					fn->dirent_index -= erased;
			__dirty											= true;
			return true;
		}
		catch(std::exception& ex) { panic(ex.what()); }
	}
	return false;
}
bool fat32_directory_vnode::__read_disk_data()
{
	std::allocator<char> b_alloc{};
	size_t bpc		= physical_block_size * parent_fs->__sectors_per_cluster;
	size_t total	= bpc * __my_covered_clusters.size();
	char* buffer	= b_alloc.allocate(total);
	bool success	= false;
	try
	{
		for(size_t i = 0UZ; i < __my_covered_clusters.size(); i++)
			if(!parent_fs->read_clusters(buffer + i * bpc, __my_covered_clusters[i]))
				throw std::runtime_error("cluster " + std::to_string(__my_covered_clusters[i]));
		char* end	= buffer + total;
		__my_dir_data.push_back(reinterpret_cast<fat32_directory_entry*>(buffer), reinterpret_cast<fat32_directory_entry*>(end));
		success		= true;
	}
	catch(std::exception& e) { xpanic(std::string("[FS/FAT32] read failed: ") + e.what()); }
	b_alloc.deallocate(buffer, total);
	return success;
}
void fat32_directory_vnode::__add_parsed_entry(fat32_regular_entry const& e, size_t j)
{
	bool dotted		= false;
	size_t c_spaces	= 0UZ;
	std::string name{};
	for(int i = 0; i < 8; i++)
	{
		if(e.filename[i] == ' ') c_spaces++;
		else if(e.filename[i])
		{
			for(size_t k = 0UZ; k < c_spaces; k++) name.append(' ');
			name.append(e.filename[i]);
			c_spaces = 0UZ;
		}
	}
	c_spaces		= 0UZ;
	for(size_t i	= 8UZ; i < 11; i++)
	{
		if(e.filename[i] == ' ') c_spaces++;
		else if(e.filename[i])
		{
			for(size_t k = 0UZ; k < c_spaces; k++) name.append(' ');
			if(!dotted) name.append('.');
			dotted		= true;
			name.append(e.filename[i]);
			c_spaces	= 0UZ;
		}
	}
	uint32_t cl		= start_of(e);
	if(e.attributes & 0x10)
	{
		fat32_directory_vnode* n	= parent_fs->put_directory_node(name, this, cl, j);
		if(!n) throw std::runtime_error("[FS/FAT32] failed to create directory node " + name);
		directory_tnodes.emplace(n, name);
		if(!n->parse_dir_data()) throw std::runtime_error("[FS/FAT32] parse failed on directory " + name);
		subdir_count++;
	}
	else
	{
		fat32_file_vnode* n			= parent_fs->put_file_node(name, this, cl, j);
		if(!n) throw std::runtime_error("[FS/FAT32] failed to create file node " + name);
		directory_tnodes.emplace(n, name);
		file_count++;
	}
}
bool fat32_directory_vnode::parse_dir_data()
{
	if(__has_init) return true;
	try
	{
		if(__read_disk_data())
		{
			size_t j = 0UZ;
			for(std::vector<fat32_directory_entry>::iterator i = __my_dir_data.begin(); i != __my_dir_data.end(); i++, j++)
				if(!(is_unused(*i) || is_longname(*i) || i->regular_entry.filename[0] == '.' || !start_of(i->regular_entry)))
					__add_parsed_entry(i->regular_entry, j);
			return (__has_init = true);
		}
	}
	catch(std::exception& e) { panic("[FS/FAT32] parse failed: "); panic(e.what()); }
	return false;
}
void fat32_directory_vnode::get_short_name(std::string const& full, std::string& result)
{
	std::string upper	= std::ext::to_upper(full);
	if(upper.size() < 13UZ)
		result			= std::string(upper.c_str(), std::min(12UZ, upper.size()));
	else
	{
		bool have_dot		= upper.contains('.');
		std::string trimmed	= upper.without_any_of(". ");
		unsigned i			= 1U;
		size_t j;
		do {
			std::string tail	= "~" + std::to_string(i);
			j					= std::min(static_cast<size_t>(6UL - tail.size()), trimmed.size());
			if(trimmed.size() > 1UZ && have_dot) tail.append('.');
			if(trimmed.size() >= 3UZ) tail.append(trimmed[trimmed.size() - 3Z]);
			if(trimmed.size() >= 2UZ) tail.append(trimmed[trimmed.size() - 2Z]);
			if(trimmed.size() >= 1UZ) tail.append(trimmed.back());
			result				= std::string(trimmed.c_str(), j);
			result.append(tail);
			i++;
		} while(directory_tnodes.contains(result) && i <= 999999U);
		if(directory_tnodes.contains(result)) throw std::logic_error("[FS/FAT32] unique short name from " + upper + " cannot be formed");
	}
}
bool fat32_directory_vnode::fsync()
{
	if(!parse_dir_data()) return false;
	if(!__dirty) return true;
	if(parent_dir && dirent_index) update_times(*disk_entry());
	try
	{
		char const* pos		= reinterpret_cast<char const*>(__my_dir_data.begin().base());
		for(size_t i = 0UZ; i < __my_covered_clusters.size(); i++)
		{
			if(parent_fs->write_clusters(__my_covered_clusters[i], pos))
				pos += parent_fs->block_size();
			else return panic("write failed"), false;
		}
		__dirty				= false;
		return true;
	}
	catch(std::exception& e) { panic(e.what()); }
	return false;
}
bool fat32_directory_vnode::unlink(std::string const& name)
{
	tnode_dir::iterator i	= directory_tnodes.find(name);
	if(__unlikely(i == directory_tnodes.end()))
		return panic("[FS/FAT32] target does not exist"), false;
	return __dir_ent_erase(name);
}
bool fat32_directory_vnode::truncate()
{
	directory_tnodes.clear();
	array_zero(__my_dir_data.data(), __my_dir_data.size());
	return fsync();
}