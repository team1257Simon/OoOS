#include <device_registry.hpp>
#include <stdexcept>
device_registry device_registry::__instance{};
device_registry::device_registry() : __base(256UZ) {}
device_registry::~device_registry() = default;
device_registry& device_registry::get_instance() { return __instance; }
device_stream* device_registry::operator[](dword id) const
{
	const_iterator i				= find(id.hi);
	if(i == end()) return nullptr;
	dev_class_map::const_iterator j	= i->second.find(id.lo);
	if(j) return j->second;
	return nullptr;
}
uint32_t device_registry::add(device_stream* dev, device_type type)
{
	dev_class_map& m	= (*static_cast<__base*>(this))[type];
	uint16_t minor		= static_cast<uint16_t>(m.size());
	m.emplace(minor, dev);
	return create_id(type, minor);
}
bool device_registry::remove(device_stream* dev)
{
	dword id			= dev->get_device_id();
	if(this->erase(id.hi)) return true;
	return false;
}
uint32_t device_registry::add(device_stream* dev, device_type type, uint16_t minor_id)
{
	dev_class_map& m	= (*static_cast<__base*>(this))[type];
	if(m.contains(minor_id)) throw std::invalid_argument("[DEV] the identifier " + std::to_string(create_id(type, minor_id), std::ext::hex) + " is already in use");
	m.emplace(minor_id, dev);
	return create_id(type, minor_id);
}
bool device_registry::registry_contains(device_type type, uint16_t minor)
{
	if(!__instance.contains(type))
		return false;
	return (static_cast<__base&>(__instance))[type].contains(minor);
}