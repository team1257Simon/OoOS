#ifndef __KEYBD
#define __KEYBD
#include <arch/ps2.hpp>
#include <util/circular_queue.hpp>
#include <util/event_listener.hpp>
#include <unordered_map>
namespace ooos
{
	typedef circular_queue<uint8_t> byte_queue;
	typedef event_listener<keyboard_event> keyboard_listener;
	typedef std::unordered_map<void*, keyboard_listener> keyboard_listener_registry;
	struct keyboard_command
	{
		bool has_sub;
		bool expects_response;
		keyboard_cmd_byte cmd_byte;
		uint8_t sub;
		addr_t response_out;
	};
	class keyboard_scan_decoder
	{
		keyboard_event const* __scans;
		keyboard_event const* __escaped_scans;
		uint8_t const* __pause_sequence;
		size_t __pause_sequence_length;
		keyboard_event __decode_one(uint8_t scan, byte_queue& rem) const;
		keyboard_event __decode_one_escaped(uint8_t scan, byte_queue& rem) const;
		keyboard_event __decode_one_seq(uint8_t seq_bookend, byte_queue& rem) const;
	public:
		keyboard_vstate current_state;
		keyboard_lstate led_state;
		keyboard_scan_decoder(keyboard_scanset ss) noexcept;
		keyboard_scan_decoder() noexcept;
		void set_scanset(keyboard_scanset ss) noexcept;
		keyboard_event decode(byte_queue& scan_bytes);
	};
	class ps2_keyboard;
	class ps2_keyboard_controller
	{
		ps2_controller& __ps2_controller;
		circular_queue<keyboard_command> __cmd_queue;
		keyboard_cstate __state;
		bool __state_valid;
		void __send_cmd_byte(keyboard_cmd_byte b);
		__nointerrupts void __execute_next() noexcept;
		__nointerrupts bool __initialize() noexcept;
		friend class ooos::ps2_keyboard;
	public:
		ps2_keyboard_controller(ps2_controller& ps2) noexcept;
		bool enqueue_command(keyboard_command&& cmd) noexcept;
		bool scanset(keyboard_scanset value) noexcept;
		bool typematic(typematic_byte value) noexcept;
		bool set_leds(keyboard_lstate lstate) noexcept;
		keyboard_scanset scanset() const noexcept;
		typematic_byte typematic() const noexcept;
		uint16_t id_word() const noexcept;
		operator bool() const noexcept;
	};
	class ps2_keyboard
	{
		ps2_keyboard_controller __controller;
		keyboard_scan_decoder __decoder;
		byte_queue __input_queue;
		keyboard_listener_registry __listeners;
		void __on_irq();
		void __init();
	public:
		ps2_keyboard(ps2_controller& ps2);
		keyboard_listener& listener_for(void* owner);
		keyboard_listener& create_listener(void* owner, keyboard_listener&& l);
		bool remove_listener(void* owner);
		bool has_listener(void* owner);
		uint16_t id_word() const noexcept;
		template<__internal::__callable<keyboard_event> FT>
		constexpr bool add_listener(void* owner, FT&& ft) { return __listeners.emplace(std::piecewise_construct, std::tuple<void*>(owner), std::forward_as_tuple<FT>(std::forward<FT>(ft))).second; }
		template<__internal::__callable<keyboard_event> FT>
		constexpr keyboard_listener replace_listener(void* owner, FT&& ft)
		{
			keyboard_listener l(std::forward<FT>(ft));
			__listeners[owner].swap(l);
			return std::move(l);
		}
	};
}
#endif