#include "device_registry.hpp"
device_registry device_registry::__instance{};
device_registry::device_registry() : __base(256) {}
device_registry::~device_registry() = default;
device_registry& device_registry::get_instance() { return __instance; }
device_stream* device_registry::operator[](dword id) const
{
	const_iterator i = find(id.hi);
	if(i == end()) return nullptr;
	dev_class_map::const_iterator j = i->second.find(id.lo);
	if(j) return j->second;
	return nullptr;
}
uint32_t device_registry::add(device_stream* dev, device_type type)
{
	dev_class_map& m	= (*static_cast<__base*>(this))[type];
	uint16_t minor		= static_cast<uint16_t>(m.size());
	m.emplace(minor, dev);
	return dword(minor, type);
}
bool device_registry::remove(device_stream* dev)
{
	dword id = dev->get_device_id();
	if(this->erase(id.hi)) return true;
	return false;
}