#if (__cplusplus > 202302L) && (__cpp_impl_reflection >= 202506L)
#include "describe.hpp"
#include <iostream>
typedef long register_t;
typedef __int128 int128_t;
#include "../common/include/kernel/sched/task.h"
#include "../common/include/kernel/sched/thread.hpp"
constexpr std::size_t task_regs_idx		= index_for_member<task_t>("saved_regs");
constexpr std::size_t thread_regs_idx	= index_for_member<thread_t>("saved_regs");
constexpr std::size_t task_ctl_idx		= index_for_member<task_t>("task_ctl");
constexpr std::size_t thread_ctl_idx	= index_for_member<thread_t>("ctl_info");
using tctl_sub_0						= [: type_of(nth_member_of(^^tctl_t, 0)) :];
using tctl_sub_1						= [: type_of(nth_member_of(^^tctl_t, 1)) :];
using lctl_sub_0						= [: type_of(nth_member_of(^^thread_ctl, 0)) :];
using lctl_sub_1						= [: type_of(nth_member_of(^^thread_ctl, 1)) :];
extern "C"
{
	auto task_fields					= describe_fields<task_t>()();
	auto thread_fields					= describe_fields<thread_t>()();
	auto task_registers					= describe_member_subobjects<task_t, task_regs_idx>();
	auto thread_registers				= describe_member_subobjects<thread_t, thread_regs_idx>();
	auto tsub_0_fields					= describe_fields<tctl_sub_0>()(describe::for_offset(nth_member_offset<task_t>(task_ctl_idx)));
	auto tsub_1_fields					= describe_fields<tctl_sub_1>()(describe::for_offset(nth_member_offset<tctl_t>(1), nth_member_offset<task_t>(task_ctl_idx).bytes));
	auto lsub_0_fields					= describe_fields<lctl_sub_0>()(describe::for_offset(nth_member_offset<thread_t>(thread_ctl_idx)));
	auto lsub_1_fields					= describe_fields<lctl_sub_1>()(describe::for_offset(nth_member_offset<thread_ctl>(1), nth_member_offset<task_t>(thread_ctl_idx).bytes));
}
using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::uppercase;
uint64_t calc_bit_mask(member_name_and_offset const& m)
{
	uint64_t result	= 0UZ;
	for(size_t i = 0UZ; i < m.bit_width; i++) result	|= (1 << (i + m.offset_bits));
	return result;
}
void def_task_m(member_name_and_offset const& m)
{
	cout << "#define TASK_MEMBER_" << m.name << "_OFFSET 0x" << m.offset_bytes << "\n";
	if(m.is_bit_flag)
		cout << dec << "#define TASK_MEMBER_" << m.name << "_BIT " << m.offset_bits << "\n" << hex;
	else if(m.bit_width) {
		cout << dec << "#define TASK_MEMBER_" << m.name << "_SHIFT " << m.offset_bits << "\n" << hex;
		cout << "#define TASK_MEMBER_" << m.name << "_MASK 0x" << calc_bit_mask(m) << "\n";
	}
}
void def_thread_m(member_name_and_offset const& m)
{
	cout << "#define THREAD_MEMBER_" << m.name << "_OFFSET 0x" << m.offset_bytes << "\n";
	if(m.is_bit_flag)
		cout << dec << "#define THREAD_MEMBER_" << m.name << "_BIT " << m.offset_bits << "\n" << hex;
	else if(m.bit_width) {
		cout << dec << "#define THREAD_MEMBER_" << m.name << "_SHIFT " << m.offset_bits << "\n" << hex;
		cout << "#define THREAD_MEMBER_" << m.name << "_MASK 0x" << calc_bit_mask(m) << "\n";
	}
}
int main()
{
	cout << hex << uppercase;
	for(member_name_and_offset const& m : task_fields) def_task_m(m);
	for(member_name_and_offset const& m : thread_fields) def_thread_m(m);
	for(member_name_and_offset const& m : task_registers) def_task_m(m);
	for(member_name_and_offset const& m : thread_registers) def_thread_m(m);
    for(member_name_and_offset const& m : tsub_0_fields) def_task_m(m);
	for(member_name_and_offset const& m : tsub_1_fields) def_task_m(m);
	for(member_name_and_offset const& m : lsub_0_fields) def_thread_m(m);
	for(member_name_and_offset const& m : lsub_1_fields) def_thread_m(m);
	cout << "#define T_OFFS(m) TASK_MEMBER_##m##_OFFSET\n";
	cout << "#define L_OFFS(m) THREAD_MEMBER_##m##_OFFSET\n";
	cout << "#define T_BIT(m) TASK_MEMBER_##m##_BIT\n";
	cout << "#define L_BIT(m) THREAD_MEMBER_##m##_BIT\n";
	cout << "#define T_SHIFT(m) TASK_MEMBER_##m##_SHIFT\n";
	cout << "#define L_SHIFT(m) THREAD_MEMBER_##m##_SHIFT\n";
	cout << "#define T_MASK(m) TASK_MEMBER_##m##_MASK\n";
	cout << "#define L_MASK(m) THREAD_MEMBER_##m##_MASK\n";
	cout << "#define T_SIZE 0x" << sizeof(task_t) << "\n";
	cout << "#define L_SIZE 0x" << sizeof(thread_t) << endl;
	return 0;
}
#endif