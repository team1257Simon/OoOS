#ifndef __FRAME_MGR
#define __FRAME_MGR
#include "kernel/kernel_mm.hpp"
#include "set"
#include "unordered_map"
struct shared_block : block_descriptor { size_t num_refs; };
class frame_manager : std::set<uframe_tag>
{
	typedef std::set<uframe_tag> __base;
	static frame_manager __instance;
	std::unordered_map<addr_t, shared_block> __global_shared_blocks;
	std::unordered_map<addr_t, int> __local_shared_blocks;
	frame_manager();
	void __release_block(block_descriptor& bd);
public:
	using __base::iterator;
	using __base::const_iterator;
	uframe_tag& create_frame(addr_t start_base, addr_t start_extent);
	uframe_tag& fork_frame(uframe_tag* old_frame);
	void destroy_frame(uframe_tag& ft);
	block_descriptor* get_global_shared(uframe_tag* tag, size_t size, addr_t start, size_t align, bool execute);
	int count_references(addr_t block_vaddr);
	static frame_manager& get();
};
#define fm frame_manager::get()
#endif