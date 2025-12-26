#ifndef __KEYBD_STDIN
#define __KEYBD_STDIN
#include <arch/keyboard.hpp>
#include <util/multiprocess_device_stream.hpp>
namespace ooos
{
	class keyboard_stdin : public multiprocess_device_stream<char>
	{
		typedef multiprocess_device_stream<char> __base;
	public:
		keyboard_stdin(uint16_t id_word);
		keyboard_stdin(keyboard_stdin&&);
		virtual ~keyboard_stdin();
		void operator()(keyboard_event&& e);
	};
}
#endif