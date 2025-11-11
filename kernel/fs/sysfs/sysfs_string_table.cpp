#include "fs/sysfs.hpp"
sysfs_string_table::sysfs_string_table(sysfs_vnode& vn) : node(vn) {}
sysfs_string_table::sysfs_string_table(sysfs& parent, uint32_t ino) : sysfs_string_table(parent.open(ino)) {}
size_t sysfs_string_table::size() const { return node.range(); }
bool sysfs_string_table::write(std::string const& str, off_t where)
{
	std::streamoff result	= node.pubseekpos(where);
	if(__unlikely(result < 0)) return false;
	return (node.sputn(str.c_str(), str.size()) != 0) && (node.pubsync() == 0) && node.parent().sync();
}
off_t sysfs_string_table::write(std::string const& str)
{
	std::streamoff result	= node.pubseekoff(0, std::ios_base::end);
	if(__unlikely(result < 0)) return result;
	return ((node.sputn(str.c_str(), str.size()) != 0) && (node.pubsync() == 0) && node.parent().sync()) ? result : static_cast<off_t>(-1);
}
std::string sysfs_string_table::read(off_t pos) const
{
	std::streamoff result	= node.pubseekpos(pos);
	if(result < 0) throw std::out_of_range("[FS/SYSFS/STRTAB] out of range: " + std::to_string(pos) + " for size " + std::to_string(node.range()));
	const char* ptr			= node.data() + pos;
	return std::string(ptr, std::strnlen(ptr, 1024UZ));
}