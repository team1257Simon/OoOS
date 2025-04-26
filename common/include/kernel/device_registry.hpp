#ifndef __DEVICE_REGISTRY
#define __DEVICE_REGISTRY
#include "unordered_map"
#include "bits/basic_streambuf.hpp"
typedef std::basic_streambuf<char> device_stream;
enum device_type : uint16_t
{
    ZERO = 0,
    COM = 1,
    CHARDEV = 2,
    VGA = 3,
    ENTRO = 4,
    //...
};
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
};
#endif