#include <arch/keyboard_stdin.hpp>
#include <device_registry.hpp>
namespace ooos
{
	keyboard_stdin::keyboard_stdin(uint16_t id_word) : __base(device_registry::create_id(CHARDEV, id_word)) {}
	keyboard_stdin::keyboard_stdin(keyboard_stdin&&) = default;
	keyboard_stdin::~keyboard_stdin() = default;
	void keyboard_stdin::operator()(keyboard_event&& e) { if(wchar_t wc = e; wc <= 127) __base::push_to_active(static_cast<char>(wc)); }
}