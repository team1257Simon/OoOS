#ifndef __PIPES
#define __PIPES
#include <ext/dynamic_queue_streambuf.hpp>
#include <unordered_map>
#include <atomic>
class simplex_pipe : public std::ext::dynamic_queue_streambuf<char>
{
	using __base = std::ext::dynamic_queue_streambuf<char>;
protected:
	virtual void on_modify_queue() override;
	virtual pos_type seekpos(pos_type pos, std::ios_base::openmode mode) noexcept override;
	virtual pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode mode) noexcept override;
public:
	std::atomic<size_t> readers;
	std::atomic<size_t> writers;
	simplex_pipe();
	bool grow(size_type added);
	bool truncate();
	size_type size() const;
	pos_type tell(std::ios_base::openmode which) const;
};
#endif