#ifndef __IOS_INTERFACE
#define __IOS_INTERFACE
#include <type_traits>
#include <stdint.h>
enum device_type : uint16_t
{
	ZERO	= 0,
	COM		= 1,
	CHARDEV	= 2,
	VGA		= 3,
	ENTRO	= 4,
	//...
};
// Interface for various types of devices as I/O streams.
template<typename T>
struct dev_stream
{
	typedef decltype(sizeof(T)) size_type;
	typedef T const* const_pointer;
	typedef T* pointer;
	typedef decltype(std::declval<T*>() - std::declval<T*>()) difference_type;
	virtual int sync()																= 0;
	virtual size_type read(pointer dest, size_type n)								= 0;
	virtual size_type write(size_type n, const_pointer src)							= 0;
	virtual size_type seek(int direction, difference_type where, uint8_t ioflags)	= 0;
	virtual size_type seek(size_type where, uint8_t ioflags)						= 0;
	virtual size_type avail() const													= 0;
	virtual size_type out_avail() const												= 0;
	virtual uint32_t get_device_id() const noexcept									= 0;
};
#endif