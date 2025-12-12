#ifndef __NETSKB
#define __NETSKB
#include <ext/dynamic_duplex_streambuf.hpp>
#include <functional>
#include <net/net_types.hpp>
class netstack_buffer : public std::ext::dynamic_duplex_streambuf<char>
{
	friend class net_device;
	friend struct abstract_packet_base;
	typedef std::ext::dynamic_duplex_streambuf<char> __base;
public:
	typedef std::function<int(netstack_buffer&)> poll_functor;
	poll_functor rx_poll;
	poll_functor tx_poll;
	size_type rx_limit;
	size_type tx_limit;
protected:
	virtual int_type overflow(int_type c) override;
	virtual std::streamsize xsputn(const char* s, size_type n) override;
	virtual std::streamsize xsgetn(char* s, size_type n) override;
public:
	virtual int tx_flush() final;
	virtual int rx_flush() final;
	virtual net16 rx_packet_type() const final;
	virtual size_t ipv4_size() const final;
	virtual void rx_accumulate(netstack_buffer& that) final;
	netstack_buffer();
	netstack_buffer(size_type initial_rx_cap, size_type initial_tx_cap, poll_functor&& rxp, poll_functor&& txp, size_type tx_limit, size_type rx_limit);
	[[gnu::__nonnull__]] netstack_buffer(net_device* dev);
	constexpr void* tx_base() { return pbase(); }
	constexpr void* rx_base() { return eback(); }
};
#endif