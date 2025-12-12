#ifndef __NETDEV_MOD
#define __NETDEV_MOD
#include <module.hpp>
namespace ooos
{
	class abstract_netdev_module : public abstract_module_base, public abstract_netdev
	{
		netdev_api_helper* __net_api;
	public:
		virtual void fini_dev()	= 0;
		inline virtual bool initialize() final override;
		inline virtual void finalize() final override;
		inline std::ext::resettable_queue<netstack_buffer>::iterator transfer_head() noexcept { return __net_api->get_transfer_buffers().begin(); }
		template<std::derived_from<abstract_protocol_handler> PT> requires(std::constructible_from<PT, protocol_ethernet*>)
		inline PT& add_protocol_handler(net16 id) { return __net_api->add_protocol(id, std::move(create_handler<PT>(std::addressof(__net_api->get_ethernet())))).template cast<PT>(); }
	};
	inline bool ooos::abstract_netdev_module::initialize()
	{
		this->__net_api	= api_global->create_net_helper(*this);
		if(__unlikely(!this->__net_api || !this->__net_api->construct_transfer_buffers())) return false;
		return this->init_dev();
	}
	inline void ooos::abstract_netdev_module::finalize()
	{
		this->fini_dev();
		this->__net_api->~netdev_api_helper();
		this->release_buffer(this->__net_api, 8UZ);
		this->__net_api	= nullptr;
	}
}
#endif