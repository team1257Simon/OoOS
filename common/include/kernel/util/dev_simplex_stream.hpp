#ifndef __DEV_SIMPLEX
#define __DEV_SIMPLEX
#include <vector>
#include <kernel_api.hpp>
#include <ext/simplex_stream.hpp>
#include <fs/dev_stream.hpp>
#include <sys/errno.h>
namespace ooos
{
	template<typename T>
	class dev_simplex_stream : protected std::ext::simplex_stream<ooos::circular_queue<T>, std::vector<T>>, public dev_stream<T>
	{
		typedef std::ext::simplex_stream<ooos::circular_queue<T>, std::vector<T>> __base_impl;
	protected:
		uint32_t dev_id;
	public:
		typedef typename __base_impl::value_type value_type;
		typedef typename __base_impl::const_pointer const_pointer;
		typedef typename __base_impl::pointer pointer;
		typedef typename __base_impl::size_type size_type;
		typedef typename __base_impl::difference_type difference_type;
		dev_simplex_stream() noexcept : __base_impl(), dev_stream<T>(), dev_id() {}
		dev_simplex_stream(uint32_t id) noexcept :  __base_impl(), dev_stream<T>(), dev_id(id) {}
		dev_simplex_stream(size_type init_size) :
			__base_impl(std::piecewise_construct, std::make_tuple(init_size), std::make_tuple(init_size)),
			dev_stream<T>(),
			dev_id()
		{}
		dev_simplex_stream(uint32_t id, size_type init_size) :
			__base_impl(std::piecewise_construct, std::make_tuple(init_size), std::make_tuple(init_size)),
			dev_stream<T>(),
			dev_id(id)
		{}
		dev_simplex_stream(size_type init_in, size_type init_out) :
			__base_impl(std::piecewise_construct, std::make_tuple(init_in), std::make_tuple(init_out)),
			dev_stream<T>(),
			dev_id()
		{}
		dev_simplex_stream(uint32_t id, size_type init_in, size_type init_out) :
			__base_impl(std::piecewise_construct, std::make_tuple(init_in), std::make_tuple(init_out)),
			dev_stream<T>(),
			dev_id(id)
		{}
		virtual ~dev_simplex_stream() {}
		virtual int sync() override
		{
			try {
				this->flush();
				return 0;
			}
			catch(...) { return -ENOMEM; }
		}
		virtual size_type read(pointer dest, size_type n) override { return __base_impl::read(dest, n); }
		virtual size_type write(size_type n, const_pointer src) override { return __base_impl::write(src, src + n); }
		virtual size_type seek(int direction, difference_type where, uint8_t /* ignored */) override { return __base_impl::seek(direction, where); }
		virtual size_type seek(size_type where, uint8_t /* ignored */) override { return __base_impl::seek(where); }
		virtual size_type avail() const override { return __base_impl::in_avail(); }
		virtual size_type tellg() const	override { return __base_impl::out_pos; }
		virtual uint32_t get_device_id() const noexcept { return dev_id; }
	};
}
#endif