#include "fs/posix_dirent.hpp"
#include "fs/ext.hpp"
#include "stdexcept"
posix_directory::~posix_directory() { if(__owning_frame && __buffer_mapped_vaddr && __my_dir_buffer) __owning_frame->mmap_remove(__buffer_mapped_vaddr, __my_dir_buffer->buffer_size); }
static addr_t map_dirent_buffer(uframe_tag* tag, size_t num_entries)
{
	if(!tag) throw std::invalid_argument("[COMPAT/DIRENT] frame tag must not be null");
	addr_t result = tag->mmap_add(nullptr, (num_entries + 1) * sizeof(dirent) + sizeof(directory_buffer), true, false);
	if(!result) throw std::bad_alloc();
	return result;
}
static directory_buffer* create_dir_buffer(addr_t vaddr, uframe_tag* tag, directory_node* dirnode)
{
	std::vector<tnode*> tnodes(dirnode->size());
	size_t buffer_len           = (dirnode->readdir(tnodes) + 1) * sizeof(dirent);
	addr_t start_addr           = vaddr.plus(sizeof(directory_buffer));
	addr_t end_addr             = start_addr.plus(buffer_len);
	directory_buffer* result    = new(tag->translate(vaddr)) directory_buffer
	{
		.buffer_start   { tag->translate(start_addr) },
		.buffer_size    { buffer_len },
		.cstruct_dir
		{
			.fd         { dirnode->vid() },
			.buf_start  { start_addr },
			.buf_pos    { start_addr },
			.buf_end    { end_addr }
		}
	};
	addr_t cur = result->buffer_start;
	for(tnode* t : tnodes)
	{
		const char* name        = t->name();
		size_t name_len         = std::strnlen(name, 255UZ);
		uint8_t type            = t->is_directory() ? DT_DIR : (t->is_device() ? DT_FIFO : DT_REG);
		ext_vnode* vn           = dynamic_cast<ext_vnode*>(t->ptr());
		if(vn && vn->is_symlink()) { type = DT_LNK; }
		dirent* ent             = new(cur) dirent
		{
			.d_ino      { static_cast<long>(t->ref().cid()) },
			.d_reclen   { static_cast<uint16_t>(sizeof(dirent)) },
			.d_type     { type },
			.d_name     {}
		};
		__builtin_memcpy(ent->d_name, name, name_len);
		ent->d_name[name_len]   = 0;
		cur                     += sizeof(dirent);
	}
	array_zero(cur.as<uint64_t>(), sizeof(dirent) / sizeof(uint64_t));
	return result;
}
posix_directory& posix_directory::operator=(posix_directory const& that)
{
	this->__owning_frame            = nullptr;
	this->__buffer_mapped_vaddr     = that.__buffer_mapped_vaddr;
	this->__my_dir_buffer           = nullptr;
	return *this;
}
posix_directory& posix_directory::operator=(posix_directory&& that)
{
	this->__owning_frame            = that.__owning_frame;
	this->__buffer_mapped_vaddr     = that.__buffer_mapped_vaddr;
	this->__my_dir_buffer           = that.__my_dir_buffer;
	that.__owning_frame             = nullptr;
	that.__buffer_mapped_vaddr      = nullptr;
	that.__my_dir_buffer            = nullptr;
	return *this;
}
posix_directory::posix_directory(directory_node* dirnode, uframe_tag* calling_tag) :
	__owning_frame          { calling_tag },
	__buffer_mapped_vaddr   { map_dirent_buffer(calling_tag, dirnode->size()) },
	__my_dir_buffer         { create_dir_buffer(__buffer_mapped_vaddr, calling_tag, dirnode) }
							{}
posix_directory::posix_directory(uframe_tag* owning_tag, addr_t buffer_vaddr) :
	__owning_frame          { owning_tag },
	__buffer_mapped_vaddr   { buffer_vaddr },
	__my_dir_buffer         { owning_tag->translate(buffer_vaddr) }
							{}
posix_directory::posix_directory(posix_directory&& that) :
	__owning_frame          { that.__owning_frame },
	__buffer_mapped_vaddr   { that.__buffer_mapped_vaddr },
	__my_dir_buffer         { that.__my_dir_buffer }
{
	that.__owning_frame         = nullptr;
	that.__buffer_mapped_vaddr  = nullptr;
	that.__my_dir_buffer        = nullptr;
}
posix_directory::posix_directory(posix_directory const& that) :
	__owning_frame          { nullptr },
	__buffer_mapped_vaddr   { that.__buffer_mapped_vaddr },
	__my_dir_buffer         { nullptr }
							{}