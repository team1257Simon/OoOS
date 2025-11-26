#include "fs/simplex_pipe.hpp"
void simplex_pipe::on_modify_queue() { __trim_stale(); sync(); }
simplex_pipe::simplex_pipe() : __base(4096UZ, std::allocator<char>()) { __set_stale_op_threshold(2U); __set_stale_size_threshold(256L); }
simplex_pipe::size_type simplex_pipe::size() const { return __qcapacity(); }
simplex_pipe::pos_type simplex_pipe::tell(std::ios_base::openmode which) const
{
	if(which.in)
		return __tell();
	return __qsize();
}
bool simplex_pipe::grow(size_type added)
{
	if(__unlikely(!__q_grow_buffer(added))) return false;
	on_modify_queue();
	return true;
}
bool simplex_pipe::truncate()
{
	try { __qclear(); }
	catch(...) { return false; }
	array_zero(__qbeg(), __qcapacity());
	return true;
}
simplex_pipe::pos_type simplex_pipe::seekpos(pos_type pos, std::ios_base::openmode mode) noexcept
{
	if(__q_out_of_range(__q_get_ptr(pos)))
		return pos_type(off_type(-1));
	if(mode.in)
		__qsetn(pos);
	if(mode.out)
		__qsete(pos);
	sync();
	return __tell();
}
simplex_pipe::pos_type simplex_pipe::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode mode) noexcept
{
	if(mode.in)
	{
		pointer pos	= (way > 0 ? (__end() + off) : way < 0 ? (__qbeg() + off) : __qcur() + off);
		if(__unlikely(__q_out_of_range(pos))) return pos_type(off_type(-1));
		__qsetn(pos);
		sync();
	}
	if(mode.out)
	{
		pointer pos	= (way > 0 ? (__end() + off) : way < 0 ? (__qbeg() + off) : __end() + off);
		if(__unlikely(__q_out_of_range(pos))) return pos_type(off_type(-1));
		__qsete(pos);
		sync();
		return __qsize();
	}
	return __tell();
}