#ifndef __KEYBD_STDIN
#define __KEYBD_STDIN
#include <arch/keyboard.hpp>
#include <util/multiprocess_device_stream.hpp>
#include <functional>
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
		void activate(pid_t id = ooos::active_pid());
	};
	namespace __internal
	{
		template<typename T> concept __keyboard_struct = requires(T& t)
		{
			{ t.id_word() } -> std::same_as<uint16_t>;
			{ t.create_listener(std::declval<void*>(), std::declval<keyboard_listener&&>()) } -> std::assignable_from<keyboard_listener>;
			{ t.listener_for(std::declval<void*>()) } -> std::assignable_from<keyboard_listener>;
			{ t.has_listener(std::declval<void*>()) } -> std::same_as<bool>;
			{ t.remove_listener(std::declval<void*>()) } -> std::same_as<bool>;
		};
	}
	struct keyboard_interface
	{
		std::function<uint16_t()> id_word;
		std::function<keyboard_listener&(void*, keyboard_listener&&)> create_listener;
		std::function<keyboard_listener&(void*)> listener_for;
		std::function<bool(void*)> has_listener;
		std::function<bool(void*)> remove_listener;
	};
	template<__internal::__keyboard_struct T>
	constexpr keyboard_interface make_interface(T& t)
	{
		return keyboard_interface
		{
			.id_word			{ std::bind(&T::id_word, std::addressof(t)) },
			.create_listener	{ std::bind(&T::create_listener, std::addressof(t), std::placeholders::_1, std::placeholders::_2) },
			.listener_for		{ std::bind(&T::listener_for, std::addressof(t), std::placeholders::_1) },
			.has_listener		{ std::bind(&T::has_listener, std::addressof(t), std::placeholders::_1) },
			.remove_listener	{ std::bind(&T::remove_listener, std::addressof(t), std::placeholders::_1) }
		};
	}
}
#endif