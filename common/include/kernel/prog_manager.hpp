#ifndef __PROG_MANAGER
#define __PROG_MANAGER
#include "kernel/elf64_dynamic_exec.hpp"
#include "list"
class prog_manager : std::list<elf64_executable>, std::list<elf64_dynamic_executable>
{
	using __static_base = std::list<elf64_executable>;
	using __dynamic_base = std::list<elf64_dynamic_executable>;
#pragma GCC diagnostic push
//  GCC will yell at us for using offsetof on a non-standard-layout type, except the part of the node we're sizing is standard-layout in effect.
//  The purpose of this is really just to allow for theoretical future changes to that data structure without breaking this code.
//  These offsets are just used to compute the location of the containing node given an executable's object handle.
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
	constexpr static off_t __static_node_offset = offsetof(typename __static_base::__node_type, __data);
	constexpr static off_t __dynamic_node_offset = offsetof(typename __dynamic_base::__node_type, __data);
#pragma GCC diagnostic pop
	static prog_manager __instance;
	elf64_executable* __add(addr_t img_start, size_t img_size, size_t stack_sz);
	prog_manager();
public:
	elf64_executable* add(file_vnode* exec_file, size_t stack_sz = S04);
	elf64_executable* clone(elf64_executable const* exec);
	void remove(elf64_executable* e);
	static prog_manager& get_instance();
};
#endif