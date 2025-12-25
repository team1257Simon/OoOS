#ifndef __KBD_STREAM
#define __KBD_STREAM
#include <arch/keyboard.hpp>
#include <util/dev_simplex_stream.hpp>
namespace ooos
{
	class keyboard_event_stream : public dev_simplex_stream<keyboard_event>
	{
		typedef dev_simplex_stream<keyboard_event> __base;
	public:
		keyboard_event_stream(uint16_t id_word) noexcept;
		keyboard_event_stream(uint16_t id_word, size_t init_buffer_size);
		keyboard_event_stream(uint16_t id_word, size_t in_queue_init_size, size_t out_buffer_init_size);
		virtual ~keyboard_event_stream();
		virtual size_type write(size_type, const_pointer) override;
		void operator()(keyboard_event&& e);
	};
	class keyboard_stdin : public dev_simplex_stream<char>
	{
		typedef dev_simplex_stream<char> __base;
	public:
		keyboard_stdin(uint16_t id_word) noexcept;
		keyboard_stdin(uint16_t id_word, size_t init_buffer_size);
		keyboard_stdin(uint16_t id_word, size_t in_queue_init_size, size_t out_buffer_init_size);
		virtual ~keyboard_stdin();
		virtual size_type write(size_type, const_pointer) override;
		void operator()(keyboard_event&& e);
	};
}
#endif