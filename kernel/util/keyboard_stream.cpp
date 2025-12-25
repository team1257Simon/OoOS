#include <util/keyboard_stream.hpp>
#include <device_registry.hpp>
namespace ooos
{
	keyboard_event_stream::keyboard_event_stream(uint16_t id_word) noexcept : __base(device_registry::create_id(CHARDEV, id_word)) {}
	keyboard_event_stream::keyboard_event_stream(uint16_t id_word, size_t init_buffer_size) : __base(device_registry::create_id(CHARDEV, id_word), init_buffer_size) {}
	keyboard_event_stream::keyboard_event_stream(uint16_t id_word, size_t in_queue_init_size, size_t out_buffer_init_size) : __base(device_registry::create_id(CHARDEV, id_word), in_queue_init_size, out_buffer_init_size) {}
	keyboard_event_stream::~keyboard_event_stream() = default;
	void keyboard_event_stream::operator()(keyboard_event&& e) { push(e); }
	keyboard_stdin::keyboard_stdin(uint16_t id_word) noexcept : __base(device_registry::create_id(CHARDEV, id_word)) {}
	keyboard_stdin::keyboard_stdin(uint16_t id_word, size_t init_buffer_size) : __base(device_registry::create_id(CHARDEV, id_word), init_buffer_size) {}
	keyboard_stdin::keyboard_stdin(uint16_t id_word, size_t in_queue_init_size, size_t out_buffer_init_size) : __base(device_registry::create_id(CHARDEV, id_word), in_queue_init_size, out_buffer_init_size) {}
	keyboard_stdin::~keyboard_stdin() = default;
	void keyboard_stdin::operator()(keyboard_event&& e) { if(wchar_t wch = e; wch < 128) push(static_cast<char>(wch)); }
	keyboard_event_stream::size_type keyboard_event_stream::write(size_type, const_pointer) { throw std::invalid_argument("[FS/DEV] illegal attempt to write to input-only stream"); }
	keyboard_stdin::size_type keyboard_stdin::write(size_type, const_pointer) { throw std::invalid_argument("[FS/DEV] illegal attempt to write to input-only stream"); }
}