#ifndef __DEVICE_REGISTRY
#define __DEVICE_REGISTRY
#include "unordered_map"
#include "fs/dev_stream.hpp"
typedef dev_stream<char> device_stream;
class dev_class_map : public std::unordered_map<uint16_t, device_stream*>
{
	using __base = std::unordered_map<uint16_t, device_stream*>;
public:
	constexpr dev_class_map() : __base(256) {}
	constexpr ~dev_class_map() = default;
};
class device_registry : public std::unordered_map<uint16_t, dev_class_map>
{
	using __base = std::unordered_map<uint16_t, dev_class_map>;
	static device_registry __instance;
public:
	device_registry();
	~device_registry();
	static device_registry& get_instance();
	device_stream* operator[](dword id) const;
	uint32_t add(device_stream* dev, device_type type);
	bool remove(device_stream* dev);
};
#define dreg device_registry::get_instance()
#endif