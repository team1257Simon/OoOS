#include "arch/keyboard.hpp"
#include "arch/arch_amd64.h"
#include "isr_table.hpp"
#include "array"
namespace ooos
{
	using enum keyboard_scanset;
	using enum keyboard_cmd_byte;
	using enum keycode;
	typedef keyboard_event const scancode_table[2][256];
	// Helper functions to build the prototype packets for the scancode tables.
	// Other than the unknown scan helpers and to_break, these should only be used in constant-evaluated context (i.e. in building the scancode tables).
#pragma region packet prototypes
	constexpr keyboard_event of_unicode(wchar_t unicode, bool is_break, bool is_numpad) noexcept
	{
		return keyboard_event
		{
			.kv_code		= unicode,
			.kv_release		= is_break,
			.kv_numpad		= is_numpad,
			.kv_multiscan	= false
		};
	}
	constexpr keyboard_event of_special(keycode keycode, bool is_break) noexcept
	{
		return keyboard_event
		{
			.kv_code			= static_cast<wchar_t>(keycode),
			.kv_release			= is_break,
			.kv_multiscan		= (!is_break && keycode == KC_SEQB),
			.kv_vstate
			{
				.left_ctrl		= (keycode == KC_LCTRL),
				.right_ctrl		= (keycode == KC_RCTRL),
				.left_alt		= (keycode == KC_LALT),
				.right_alt		= (keycode == KC_RALT),
				.left_shift		= (keycode == KC_LSHFT),
				.right_shift	= (keycode == KC_RSHFT),
				.caps_lock		= (!is_break && keycode == KC_CAPS),
				.num_lock		= (!is_break && keycode == KC_NUM)
			}
		};
	}
	// Converts a key-down packet into the equivalent key-up
	constexpr keyboard_event to_break(keyboard_event const& make) noexcept
	{
		// Everything except the values of kv_release, kv_vstate.caps_lock, and kv_vstate.num_lock remain identical.
		// Break events shouldn't toggle the locks, so we clear those bits in the XOR mask.
		keyboard_event result(make);
		result.kv_release 			= true;
		result.kv_vstate.caps_lock 	= false;
		result.kv_vstate.num_lock 	= false;
		return result;
	}
	// key down for a character key
	constexpr keyboard_event c_dn(wchar_t code) noexcept { return of_unicode(code, false, false); }
	// key down for a numpad character key
	constexpr keyboard_event n_dn(wchar_t code) noexcept { return of_unicode(code, false, true); }
	// key down for a special (i.e. non-character) key
	constexpr keyboard_event s_dn(keycode code) noexcept { return of_special(code, false); }
	// key up for a character key
	constexpr keyboard_event c_up(wchar_t code) noexcept { return of_unicode(code, true, false); }
	// key up for a numpad character key
	constexpr keyboard_event n_up(wchar_t code) noexcept { return of_unicode(code, true, true); }
	// key up for a special (i.e. non-character) key
	constexpr keyboard_event s_up(keycode code) noexcept { return of_special(code, true); }
	// unknown scancode
	constexpr keyboard_event uk_sc(uint8_t scan) noexcept { return keyboard_event(static_cast<wchar_t>(KC_UNKNOWN) | scan); }
	// unknown extended/escaped scancode
	constexpr keyboard_event uk_ex(uint8_t scan) noexcept { return keyboard_event(static_cast<wchar_t>(KC_EXT_UNKNOWN) | scan); }
	// unknown multibyte sequence, not counting escape bytes
	constexpr keyboard_event uk_seq(uint8_t first, uint8_t second) noexcept { return keyboard_event(static_cast<wchar_t>(KC_SEQ_UNKNOWN) | word(first, second)); }
	constexpr static keyboard_event prtscr_packet
	{
		.kv_code		= static_cast<wchar_t>(KC_PRTSC),
		.kv_numpad		= false,
		.kv_multiscan	= true
	};
	constexpr static keyboard_event prtscr_break
	{
		.kv_code		= static_cast<wchar_t>(KC_PRTSC),
		.kv_release		= true,
		.kv_numpad		= false,
		.kv_multiscan	= true
	};
	constexpr static keyboard_event pause_packet
	{
		.kv_code 		= static_cast<wchar_t>(KC_PAUSE),
		.kv_release		= false,
		.kv_numpad		= false,
		.kv_multiscan	= true,
	};
#pragma endregion
	// Scanset 1 decode tables
#pragma region scanset 1
	static scancode_table set1
	{
		{
			uk_sc(0x00UC),		c_dn('\033'),		c_dn('1'),			c_dn('2'),
			c_dn('3'),			c_dn('4'),			c_dn('5'),			c_dn('6'),
			c_dn('7'),			c_dn('8'),			c_dn('9'),			c_dn('0'),
			c_dn('-'),			c_dn('='),			c_dn('\b'),			c_dn('\t'),
			c_dn('q'),			c_dn('w'),			c_dn('e'),			c_dn('r'),
			c_dn('t'),			c_dn('y'),			c_dn('u'),			c_dn('i'),
			c_dn('o'),			c_dn('p'),			c_dn('['),			c_dn(']'),
			c_dn('\n'),			s_dn(KC_LCTRL),		c_dn('a'),			c_dn('s'),
			c_dn('d'),			c_dn('f'),			c_dn('g'),			c_dn('h'),
			c_dn('j'),			c_dn('k'),			c_dn('l'),			c_dn(';'),
			c_dn('\''),			c_dn('`'),			s_dn(KC_LSHFT),		c_dn('\\'),
			c_dn('z'),			c_dn('x'),			c_dn('c'),			c_dn('v'),
			c_dn('b'),			c_dn('n'),			c_dn('m'),			c_dn(','),
			c_dn('.'),			c_dn('/'),			s_dn(KC_RSHFT),		n_dn('*'),
			s_dn(KC_LALT),		c_dn(' '),			s_dn(KC_CAPS),		s_dn(KC_F1),
			s_dn(KC_F2),		s_dn(KC_F3),		s_dn(KC_F4),		s_dn(KC_F5),
			s_dn(KC_F6),		s_dn(KC_F7),		s_dn(KC_F8),		s_dn(KC_F9),
			s_dn(KC_F10),		s_dn(KC_NUM),		s_dn(KC_SCRLL),		n_dn('7'),
			n_dn('8'),			n_dn('9'),			n_dn('-'),			n_dn('4'),
			n_dn('5'),			n_dn('6'),			n_dn('+'),			n_dn('1'),
			n_dn('2'),			n_dn('3'),			n_dn('0'),			n_dn('.'),
			uk_sc(0x54UC),		uk_sc(0x55UC),		uk_sc(0x56UC),		s_dn(KC_F11),
			s_dn(KC_F12),		uk_sc(0x59UC),		uk_sc(0x5AUC),		uk_sc(0x5BUC),
			uk_sc(0x5CUC),		uk_sc(0x5DUC),		uk_sc(0x5EUC),		uk_sc(0x5FUC),
			uk_sc(0x60UC),		uk_sc(0x61UC),		uk_sc(0x62UC),		uk_sc(0x63UC),
			uk_sc(0x64UC),		uk_sc(0x65UC),		uk_sc(0x66UC),		uk_sc(0x67UC),
			uk_sc(0x68UC),		uk_sc(0x69UC),		uk_sc(0x6AUC),		uk_sc(0x6BUC),
			uk_sc(0x6CUC),		uk_sc(0x6DUC),		uk_sc(0x6EUC),		uk_sc(0x6FUC),
			uk_sc(0x70UC),		uk_sc(0x71UC),		uk_sc(0x72UC),		uk_sc(0x73UC),
			uk_sc(0x74UC),		uk_sc(0x75UC),		uk_sc(0x76UC),		uk_sc(0x77UC),
			uk_sc(0x78UC),		uk_sc(0x79UC),		uk_sc(0x7AUC),		uk_sc(0x7BUC),
			uk_sc(0x7CUC),		uk_sc(0x7DUC),		uk_sc(0x7EUC),		uk_sc(0x7FUC),
			uk_sc(0x80UC),		c_up('\033'),		c_up('1'),			c_up('2'),
			c_up('3'),			c_up('4'),			c_up('5'),			c_up('6'),
			c_up('7'),			c_up('8'),			c_up('9'),			c_up('0'),
			c_up('-'),			c_up('='),			c_up('\b'),			c_up('\t'),
			c_up('q'),			c_up('w'),			c_up('e'),			c_up('r'),
			c_up('t'),			c_up('y'),			c_up('u'),			c_up('i'),
			c_up('o'),			c_up('p'),			c_up('['),			c_up(']'),
			c_up('\n'),			s_up(KC_LCTRL),		c_up('a'),			c_up('s'),
			c_up('d'),			c_up('f'),			c_up('g'),			c_up('h'),
			c_up('j'),			c_up('k'),			c_up('l'),			c_up(';'),
			c_up('\''),			c_up('`'),			s_up(KC_LSHFT),		c_up('\\'),
			c_up('z'),			c_up('x'),			c_up('c'),			c_up('v'),
			c_up('b'),			c_up('n'),			c_up('m'),			c_up(','),
			c_up('.'),			c_up('/'),			s_up(KC_RSHFT),		n_up('*'),
			s_up(KC_LALT),		c_up(' '),			s_up(KC_CAPS),		s_up(KC_F1),
			s_up(KC_F2),		s_up(KC_F3),		s_up(KC_F4),		s_up(KC_F5),
			s_up(KC_F6),		s_up(KC_F7),		s_up(KC_F8),		s_up(KC_F9),
			s_up(KC_F10),		s_up(KC_NUM),		s_up(KC_SCRLL),		n_up('7'),
			n_up('8'),			n_up('9'),			n_up('-'),			n_up('4'),
			n_up('5'),			n_up('6'),			n_up('+'),			n_up('1'),
			n_up('2'),			n_up('3'),			n_up('0'),			n_up('.'),
			uk_sc(0xD4UC),		uk_sc(0xD5UC),		uk_sc(0xD6UC),		s_up(KC_F11),
			s_up(KC_F12),		uk_sc(0xD9UC),		uk_sc(0xDAUC),		uk_sc(0xDBUC),
			uk_sc(0xDCUC),		uk_sc(0xDDUC),		uk_sc(0xDEUC),		uk_sc(0xDFUC),
			s_dn(KC_EXB),		s_dn(KC_SEQB),		uk_sc(0xE2UC),		uk_sc(0xE3UC),
			uk_sc(0xE4UC),		uk_sc(0xE5UC),		uk_sc(0xE6UC),		uk_sc(0xE7UC),
			uk_sc(0xE8UC),		uk_sc(0xE9UC),		uk_sc(0xEAUC),		uk_sc(0xEBUC),
			uk_sc(0xECUC),		uk_sc(0xEDUC),		uk_sc(0xEEUC),		uk_sc(0xEFUC),
			uk_sc(0xF0UC),		uk_sc(0xF1UC),		uk_sc(0xF2UC),		uk_sc(0xF3UC),
			uk_sc(0xF4UC),		uk_sc(0xF5UC),		uk_sc(0xF6UC),		uk_sc(0xF7UC),
			uk_sc(0xF8UC),		uk_sc(0xF9UC),		uk_sc(0xFAUC),		uk_sc(0xFBUC),
			uk_sc(0xFCUC),		uk_sc(0xFDUC),		uk_sc(0xFEUC),		uk_sc(0xFFUC)
		},
		{
			uk_ex(0x00UC),		uk_ex(0x01UC),		uk_ex(0x02UC),		uk_ex(0x03UC),
			uk_ex(0x04UC),		uk_ex(0x05UC),		uk_ex(0x06UC),		uk_ex(0x07UC),
			uk_ex(0x08UC),		uk_ex(0x09UC),		uk_ex(0x0AUC),		uk_ex(0x0BUC),
			uk_ex(0x0CUC),		uk_ex(0x0DUC),		uk_ex(0x0EUC),		uk_ex(0x0FUC),
			s_dn(KC_M1),		uk_ex(0x11UC),		uk_ex(0x12UC),		uk_ex(0x13UC),
			uk_ex(0x14UC),		uk_ex(0x15UC),		uk_ex(0x16UC),		uk_ex(0x17UC),
			uk_ex(0x18UC),		s_dn(KC_M2),		uk_ex(0x1AUC),		uk_ex(0x1BUC),
			n_dn('\n'),			s_dn(KC_RCTRL),		uk_ex(0x1EUC),		uk_ex(0x1FUC),
			s_dn(KC_M3),		s_dn(KC_M4),		s_dn(KC_M5),		uk_ex(0x23UC),
			s_dn(KC_M6),		uk_ex(0x25UC),		uk_ex(0x26UC),		uk_ex(0x27UC),
			uk_ex(0x28UC),		uk_ex(0x29UC),		prtscr_packet,		uk_ex(0x2BUC),
			uk_ex(0x2CUC),		uk_ex(0x2DUC),		s_dn(KC_M7),		uk_ex(0x2FUC),
			s_dn(KC_M8),		uk_ex(0x31UC),		s_dn(KC_M9),		uk_ex(0x33UC),
			uk_ex(0x34UC),		n_dn('/'),			uk_ex(0x36UC),		prtscr_packet,
			s_dn(KC_RALT),		uk_ex(0x39UC),		uk_ex(0x3AUC),		uk_ex(0x3BUC),
			uk_ex(0x3CUC),		uk_ex(0x3DUC),		uk_ex(0x3EUC),		uk_ex(0x3FUC),
			uk_ex(0x40UC),		uk_ex(0x41UC),		uk_ex(0x42UC),		uk_ex(0x43UC),
			uk_ex(0x44UC),		uk_ex(0x45UC),		uk_ex(0x46UC),		s_dn(KC_HOME),
			s_dn(KC_UP),		s_dn(KC_PG_UP),		uk_ex(0x4AUC),		s_dn(KC_LEFT),
			uk_ex(0x4CUC),		s_dn(KC_RIGHT),		uk_ex(0x4EUC),		s_dn(KC_END),
			s_dn(KC_DOWN),		s_dn(KC_PG_DN),		s_dn(KC_INS),		c_dn('\177'),
			uk_ex(0x54UC),		uk_ex(0x55UC),		uk_ex(0x56UC),		uk_ex(0x57UC),
			uk_ex(0x58UC),		uk_ex(0x59UC),		uk_ex(0x5AUC),		s_dn(KC_LGUI),
			s_dn(KC_RGUI),		s_dn(KC_APPS),		s_dn(KC_POWER),		s_dn(KC_SLEEP),
			uk_ex(0x60UC),		uk_ex(0x61UC),		uk_ex(0x62UC),		s_dn(KC_WAKE),
			uk_ex(0x64UC),		s_dn(KC_M10),		s_dn(KC_M11),		s_dn(KC_M12),
			s_dn(KC_M13),		s_dn(KC_M14),		s_dn(KC_M15),		s_dn(KC_M16),
			s_dn(KC_M17),		s_dn(KC_M18),		uk_ex(0x6EUC),		uk_ex(0x6FUC),
			uk_ex(0x70UC),		uk_ex(0x71UC),		uk_ex(0x72UC),		uk_ex(0x73UC),
			uk_ex(0x74UC),		uk_ex(0x75UC),		uk_ex(0x76UC),		uk_ex(0x77UC),
			uk_ex(0x78UC),		uk_ex(0x79UC),		uk_ex(0x7AUC),		uk_ex(0x7BUC),
			uk_ex(0x7CUC),		uk_ex(0x7DUC),		uk_ex(0x7EUC),		uk_ex(0x7FUC),
			uk_ex(0x80UC),		uk_ex(0x81UC),		uk_ex(0x82UC),		uk_ex(0x83UC),
			uk_ex(0x84UC),		uk_ex(0x85UC),		uk_ex(0x86UC),		uk_ex(0x87UC),
			uk_ex(0x88UC),		uk_ex(0x89UC),		uk_ex(0x8AUC),		uk_ex(0x8BUC),
			uk_ex(0x8CUC),		uk_ex(0x8DUC),		uk_ex(0x8EUC),		uk_ex(0x8FUC),
			s_up(KC_M1),		uk_ex(0x91UC),		uk_ex(0x92UC),		uk_ex(0x93UC),
			uk_ex(0x94UC),		uk_ex(0x95UC),		uk_ex(0x96UC),		uk_ex(0x97UC),
			uk_ex(0x98UC),		s_up(KC_M2),		uk_ex(0x9AUC),		uk_ex(0x9BUC),
			n_up('\n'),			s_up(KC_RCTRL),		uk_ex(0x9EUC),		uk_ex(0x9FUC),
			s_up(KC_M3),		s_up(KC_M4),		s_up(KC_M5),		uk_ex(0xA3UC),
			s_up(KC_M6),		uk_ex(0xA5UC),		uk_ex(0xA6UC),		uk_ex(0xA7UC),
			uk_ex(0xA8UC),		uk_ex(0xA9UC),		prtscr_break,		uk_ex(0xABUC),
			uk_ex(0xACUC),		uk_ex(0xADUC),		s_up(KC_M7),		uk_ex(0xAFUC),
			s_up(KC_M8),		uk_ex(0xB1UC),		s_up(KC_M9),		uk_ex(0xB3UC),
			uk_ex(0xB4UC),		n_up('/'),			uk_ex(0xB6UC),		prtscr_break,
			s_up(KC_RALT),		uk_ex(0xB9UC),		uk_ex(0xBAUC),		uk_ex(0xBBUC),
			uk_ex(0xBCUC),		uk_ex(0xBDUC),		uk_ex(0xBEUC),		uk_ex(0xBFUC),
			uk_ex(0xC0UC),		uk_ex(0xC1UC),		uk_ex(0xC2UC),		uk_ex(0xC3UC),
			uk_ex(0xC4UC),		uk_ex(0xC5UC),		uk_ex(0xC6UC),		s_up(KC_HOME),
			s_up(KC_UP),		s_up(KC_PG_UP),		uk_ex(0xCAUC),		s_up(KC_LEFT),
			uk_ex(0xCCUC),		s_up(KC_RIGHT),		uk_ex(0xCEUC),		s_up(KC_END),
			s_up(KC_DOWN),		s_up(KC_PG_DN),		s_up(KC_INS),		c_up('\177'),
			uk_ex(0xD4UC),		uk_ex(0xD5UC),		uk_ex(0xD6UC),		uk_ex(0xD7UC),
			uk_ex(0xD8UC),		uk_ex(0xD9UC),		uk_ex(0xDAUC),		s_up(KC_LGUI),
			s_up(KC_RGUI),		s_up(KC_APPS),		s_up(KC_POWER),		s_up(KC_SLEEP),
			uk_ex(0xE0UC),		uk_ex(0xE1UC),		uk_ex(0xE2UC),		s_up(KC_WAKE),
			uk_ex(0xE4UC),		s_up(KC_M10),		s_up(KC_M11),		s_up(KC_M12),
			s_up(KC_M13),		s_up(KC_M14),		s_up(KC_M15),		s_up(KC_M16),
			s_up(KC_M17),		s_up(KC_M18),		uk_ex(0xEEUC),		uk_ex(0xEFUC),
			uk_ex(0xF0UC),		uk_ex(0xF1UC),		uk_ex(0xF2UC),		uk_ex(0xF3UC),
			uk_ex(0xF4UC),		uk_ex(0xF5UC),		uk_ex(0xF6UC),		uk_ex(0xF7UC),
			uk_ex(0xF8UC),		uk_ex(0xF9UC),		uk_ex(0xFAUC),		uk_ex(0xFBUC),
			uk_ex(0xFCUC),		uk_ex(0xFDUC),		uk_ex(0xFEUC),		uk_ex(0xFFUC)
		}
	};
	static uint8_t pause_set1[]{ 0xE1UC, 0x1DUC, 0x45UC, 0xE1UC, 0x9DUC, 0xC5UC };
#pragma endregion
	// Scanset 2 decode tables
#pragma region scanset 2
	static scancode_table set2
	{
		{
			uk_sc(0x00UC),		s_dn(KC_F9),		uk_sc(0x02UC),		s_dn(KC_F5),
			s_dn(KC_F3),		s_dn(KC_F1),		s_dn(KC_F2),		s_dn(KC_F12),
			uk_sc(0x08UC),		s_dn(KC_F10),		s_dn(KC_F8),		s_dn(KC_F6),
			s_dn(KC_F4),		c_dn('\t'),			c_dn('`'),			uk_sc(0x0FUC),
			uk_sc(0x10UC),		s_dn(KC_LALT),		s_dn(KC_LSHFT),		uk_sc(0x13UC),
			s_dn(KC_LCTRL),		c_dn('q'),			c_dn('1'),			uk_sc(0x17UC),
			uk_sc(0x18UC),		uk_sc(0x19UC),		c_dn('z'),			c_dn('s'),
			c_dn('a'),			c_dn('w'),			c_dn('2'),			uk_sc(0x1FUC),
			uk_sc(0x20UC),		c_dn('c'),			c_dn('x'),			c_dn('d'),
			c_dn('e'),			c_dn('4'),			c_dn('3'),			uk_sc(0x27UC),
			uk_sc(0x28UC),		c_dn(' '),			c_dn('v'),			c_dn('f'),
			c_dn('t'),			c_dn('r'),			c_dn('5'),			uk_sc(0x2FUC),
			uk_sc(0x30UC),		c_dn('n'),			c_dn('b'),			c_dn('h'),
			c_dn('g'),			c_dn('y'),			c_dn('6'),			uk_sc(0x37UC),
			uk_sc(0x38UC),		uk_sc(0x39UC),		c_dn('m'),			c_dn('j'),
			c_dn('u'),			c_dn('7'),			c_dn('8'),			uk_sc(0x3FUC),
			uk_sc(0x40UC),		c_dn(','),			c_dn('k'),			c_dn('i'),
			c_dn('o'),			c_dn('0'),			c_dn('9'),			uk_sc(0x47UC),
			uk_sc(0x48UC),		c_dn('.'),			c_dn('/'),			c_dn('l'),
			c_dn(';'),			c_dn('p'),			c_dn('-'),			uk_sc(0x4FUC),
			uk_sc(0x50UC),		uk_sc(0x51UC),		c_dn('\''),			uk_sc(0x53UC),
			c_dn('['),			c_dn('='),			uk_sc(0x56UC),		uk_sc(0x57UC),
			s_dn(KC_CAPS),		s_dn(KC_RSHFT),		c_dn('\n'),			c_dn(']'),
			uk_sc(0x5CUC),		c_dn('\\'),			uk_sc(0x5EUC),		uk_sc(0x5FUC),
			uk_sc(0x60UC),		uk_sc(0x61UC),		uk_sc(0x62UC),		uk_sc(0x63UC),
			uk_sc(0x64UC),		uk_sc(0x65UC),		c_dn('\b'),			uk_sc(0x67UC),
			uk_sc(0x68UC),		n_dn('1'),			uk_sc(0x6AUC),		n_dn('4'),
			n_dn('7'),			uk_sc(0x6DUC),		uk_sc(0x6EUC),		uk_sc(0x6FUC),
			n_dn('0'),			n_dn('.'),			n_dn('2'),			n_dn('5'),
			n_dn('6'),			n_dn('8'),			c_dn('\033'),		s_dn(KC_NUM),
			s_dn(KC_F11),		n_dn('+'),			n_dn('3'),			n_dn('-'),
			n_dn('*'),			n_dn('9'),			s_dn(KC_SCRLL),		uk_sc(0x7FUC),
			uk_sc(0x80UC),		uk_sc(0x81UC),		uk_sc(0x82UC),		s_dn(KC_F7),
			uk_sc(0x84UC),		uk_sc(0x85UC),		uk_sc(0x86UC),		uk_sc(0x87UC),
			uk_sc(0x88UC),		uk_sc(0x89UC),		uk_sc(0x8AUC),		uk_sc(0x8BUC),
			uk_sc(0x8CUC),		uk_sc(0x8DUC),		uk_sc(0x8EUC),		uk_sc(0x8FUC),
			uk_sc(0x90UC),		uk_sc(0x91UC),		uk_sc(0x92UC),		uk_sc(0x93UC),
			uk_sc(0x94UC),		uk_sc(0x95UC),		uk_sc(0x96UC),		uk_sc(0x97UC),
			uk_sc(0x98UC),		uk_sc(0x99UC),		uk_sc(0x9AUC),		uk_sc(0x9BUC),
			uk_sc(0x9CUC),		uk_sc(0x9DUC),		uk_sc(0x9EUC),		uk_sc(0x9FUC),
			uk_sc(0xA0UC),		uk_sc(0xA1UC),		uk_sc(0xA2UC),		uk_sc(0xA3UC),
			uk_sc(0xA4UC),		uk_sc(0xA5UC),		uk_sc(0xA6UC),		uk_sc(0xA7UC),
			uk_sc(0xA8UC),		uk_sc(0xA9UC),		uk_sc(0xAAUC),		uk_sc(0xABUC),
			uk_sc(0xACUC),		uk_sc(0xADUC),		uk_sc(0xAEUC),		uk_sc(0xAFUC),
			uk_sc(0xB0UC),		uk_sc(0xB1UC),		uk_sc(0xB2UC),		uk_sc(0xB3UC),
			uk_sc(0xB4UC),		uk_sc(0xB5UC),		uk_sc(0xB6UC),		uk_sc(0xB7UC),
			uk_sc(0xB8UC),		uk_sc(0xB9UC),		uk_sc(0xBAUC),		uk_sc(0xBBUC),
			uk_sc(0xBCUC),		uk_sc(0xBDUC),		uk_sc(0xBEUC),		uk_sc(0xBFUC),
			uk_sc(0xC0UC),		uk_sc(0xC1UC),		uk_sc(0xC2UC),		uk_sc(0xC3UC),
			uk_sc(0xC4UC),		uk_sc(0xC5UC),		uk_sc(0xC6UC),		uk_sc(0xC7UC),
			uk_sc(0xC8UC),		uk_sc(0xC9UC),		uk_sc(0xCAUC),		uk_sc(0xCBUC),
			uk_sc(0xCCUC),		uk_sc(0xCDUC),		uk_sc(0xCEUC),		uk_sc(0xCFUC),
			uk_sc(0xD0UC),		uk_sc(0xD1UC),		uk_sc(0xD2UC),		uk_sc(0xD3UC),
			uk_sc(0xD4UC),		uk_sc(0xD5UC),		uk_sc(0xD6UC),		uk_sc(0xD7UC),
			uk_sc(0xD8UC),		uk_sc(0xD9UC),		uk_sc(0xDAUC),		uk_sc(0xDBUC),
			uk_sc(0xDCUC),		uk_sc(0xDDUC),		uk_sc(0xDEUC),		uk_sc(0xDFUC),
			s_dn(KC_EXB),		s_dn(KC_SEQB),		uk_sc(0xE2UC),		uk_sc(0xE3UC),
			uk_sc(0xE4UC),		uk_sc(0xE5UC),		uk_sc(0xE6UC),		uk_sc(0xE7UC),
			uk_sc(0xE8UC),		uk_sc(0xE9UC),		uk_sc(0xEAUC),		uk_sc(0xEBUC),
			uk_sc(0xECUC),		uk_sc(0xEDUC),		uk_sc(0xEEUC),		uk_sc(0xEFUC),
			s_up(KC_BRB),		uk_sc(0xF1UC),		uk_sc(0xF2UC),		uk_sc(0xF3UC),
			uk_sc(0xF4UC),		uk_sc(0xF5UC),		uk_sc(0xF6UC),		uk_sc(0xF7UC),
			uk_sc(0xF8UC),		uk_sc(0xF9UC),		uk_sc(0xFAUC),		uk_sc(0xFBUC),
			uk_sc(0xFCUC),		uk_sc(0xFDUC),		uk_sc(0xFEUC),		uk_sc(0xFFUC)
		},
		{
			uk_ex(0x00UC),		uk_ex(0x01UC),		uk_ex(0x02UC),		uk_ex(0x03UC),
			uk_ex(0x04UC),		uk_ex(0x05UC),		uk_ex(0x06UC),		uk_ex(0x07UC),
			uk_ex(0x08UC),		uk_ex(0x09UC),		uk_ex(0x0AUC),		uk_ex(0x0BUC),
			uk_ex(0x0CUC),		uk_ex(0x0DUC),		uk_ex(0x0EUC),		uk_ex(0x0FUC),
			s_dn(KC_M10),		s_dn(KC_RALT),		prtscr_packet,		uk_ex(0x13UC),
			s_dn(KC_RCTRL),		s_dn(KC_M1),		uk_ex(0x16UC),		uk_ex(0x17UC),
			s_dn(KC_M11),		uk_sc(0x19UC),		uk_ex(0x1AUC),		uk_ex(0x1BUC),
			uk_ex(0x1CUC),		uk_ex(0x1DUC),		uk_ex(0x1EUC),		s_dn(KC_LGUI),
			s_dn(KC_M12),		s_dn(KC_M7),		uk_ex(0x22UC),		s_dn(KC_M3),
			uk_ex(0x24UC),		uk_ex(0x25UC),		uk_ex(0x26UC),		s_dn(KC_RGUI),
			s_dn(KC_M13),		uk_ex(0x29UC),		uk_ex(0x2AUC),		s_dn(KC_M4),
			uk_ex(0x2CUC),		uk_ex(0x2DUC),		uk_ex(0x2EUC),		s_dn(KC_APPS),
			s_dn(KC_M14),		uk_ex(0x31UC),		s_dn(KC_M8),		uk_ex(0x33UC),
			s_dn(KC_M5),		uk_ex(0x35UC),		uk_ex(0x36UC),		s_dn(KC_POWER),
			s_dn(KC_M15),		uk_ex(0x39UC),		s_dn(KC_M9),		s_dn(KC_M6),
			uk_ex(0x3CUC),		uk_ex(0x3DUC),		uk_ex(0x3EUC),		s_dn(KC_SLEEP),
			s_dn(KC_M16),		uk_ex(0x41UC),		uk_ex(0x42UC),		uk_ex(0x43UC),
			uk_ex(0x44UC),		uk_ex(0x45UC),		uk_ex(0x46UC),		uk_ex(0x47UC),
			s_dn(KC_M17),		uk_ex(0x49UC),		n_dn('/'),		uk_ex(0x4BUC),
			uk_ex(0x4CUC),		s_dn(KC_M2),		uk_ex(0x4EUC),		uk_ex(0x4FUC),
			s_dn(KC_M18),		uk_ex(0x51UC),		uk_ex(0x52UC),		uk_ex(0x53UC),
			uk_ex(0x54UC),		uk_ex(0x55UC),		uk_ex(0x56UC),		uk_ex(0x57UC),
			uk_ex(0x58UC),		uk_ex(0x59UC),		n_dn('\n'),		uk_ex(0x5BUC),
			uk_ex(0x5CUC),		uk_ex(0x5DUC),		s_dn(KC_WAKE),		uk_ex(0x5FUC),
			uk_ex(0x60UC),		uk_ex(0x61UC),		uk_ex(0x62UC),		uk_ex(0x63UC),
			uk_ex(0x64UC),		uk_ex(0x65UC),		uk_ex(0x66UC),		uk_ex(0x67UC),
			uk_ex(0x68UC),		s_dn(KC_END),		uk_ex(0x6AUC),		s_dn(KC_LEFT),
			s_dn(KC_HOME),		uk_ex(0x6DUC),		uk_ex(0x6EUC),		uk_ex(0x6FUC),
			s_dn(KC_INS),		c_dn('\177'),		s_dn(KC_DOWN),		uk_ex(0x73UC),
			s_dn(KC_RIGHT),		s_dn(KC_UP),		uk_ex(0x76UC),		uk_ex(0x77UC),
			uk_ex(0x78UC),		uk_ex(0x79UC),		s_dn(KC_PG_DN),		uk_ex(0x7BUC),
			prtscr_packet,		s_dn(KC_PG_UP),		uk_ex(0x7EUC),		uk_ex(0x7FUC),
			uk_ex(0x80UC),		uk_ex(0x81UC),		uk_ex(0x82UC),		uk_ex(0x83UC),
			uk_ex(0x84UC),		uk_ex(0x85UC),		uk_ex(0x86UC),		uk_ex(0x87UC),
			uk_ex(0x88UC),		uk_ex(0x89UC),		uk_ex(0x8AUC),		uk_ex(0x8BUC),
			uk_ex(0x8CUC),		uk_ex(0x8DUC),		uk_ex(0x8EUC),		uk_ex(0x8FUC),
			uk_ex(0x90UC),		uk_ex(0x91UC),		uk_ex(0x92UC),		uk_ex(0x93UC),
			uk_ex(0x94UC),		uk_ex(0x95UC),		uk_ex(0x96UC),		uk_ex(0x97UC),
			uk_ex(0x98UC),		uk_ex(0x99UC),		uk_ex(0x9AUC),		uk_ex(0x9BUC),
			uk_ex(0x9CUC),		uk_ex(0x9DUC),		uk_ex(0x9EUC),		uk_ex(0x9FUC),
			uk_ex(0xA0UC),		uk_ex(0xA1UC),		uk_ex(0xA2UC),		uk_ex(0xA3UC),
			uk_ex(0xA4UC),		uk_ex(0xA5UC),		uk_ex(0xA6UC),		uk_ex(0xA7UC),
			uk_ex(0xA8UC),		uk_ex(0xA9UC),		uk_ex(0xAAUC),		uk_ex(0xABUC),
			uk_ex(0xACUC),		uk_ex(0xADUC),		uk_ex(0xAEUC),		uk_ex(0xAFUC),
			uk_ex(0xB0UC),		uk_ex(0xB1UC),		uk_ex(0xB2UC),		uk_ex(0xB3UC),
			uk_ex(0xB4UC),		uk_ex(0xB5UC),		uk_ex(0xB6UC),		uk_ex(0xB7UC),
			uk_ex(0xB8UC),		uk_ex(0xB9UC),		uk_ex(0xBAUC),		uk_ex(0xBBUC),
			uk_ex(0xBCUC),		uk_ex(0xBDUC),		uk_ex(0xBEUC),		uk_ex(0xBFUC),
			uk_ex(0xC0UC),		uk_ex(0xC1UC),		uk_ex(0xC2UC),		uk_ex(0xC3UC),
			uk_ex(0xC4UC),		uk_ex(0xC5UC),		uk_ex(0xC6UC),		uk_ex(0xC7UC),
			uk_ex(0xC8UC),		uk_ex(0xC9UC),		uk_ex(0xCAUC),		uk_ex(0xCBUC),
			uk_ex(0xCCUC),		uk_ex(0xCDUC),		uk_ex(0xCEUC),		uk_ex(0xCFUC),
			uk_ex(0xD0UC),		uk_ex(0xD1UC),		uk_ex(0xD2UC),		uk_ex(0xD3UC),
			uk_ex(0xD4UC),		uk_ex(0xD5UC),		uk_ex(0xD6UC),		uk_ex(0xD7UC),
			uk_ex(0xD8UC),		uk_ex(0xD9UC),		uk_ex(0xDAUC),		uk_ex(0xDBUC),
			uk_ex(0xDCUC),		uk_ex(0xDDUC),		uk_ex(0xDEUC),		uk_ex(0xDFUC),
			uk_ex(0xE0UC),		uk_ex(0xE1UC),		uk_ex(0xE2UC),		uk_ex(0xE3UC),
			uk_ex(0xE4UC),		uk_ex(0xE5UC),		uk_ex(0xE6UC),		uk_ex(0xE7UC),
			uk_ex(0xE8UC),		uk_ex(0xE9UC),		uk_ex(0xEAUC),		uk_ex(0xEBUC),
			uk_ex(0xECUC),		uk_ex(0xEDUC),		uk_ex(0xEEUC),		uk_ex(0xEFUC),
			s_up(KC_BRB),		uk_ex(0xF1UC),		uk_ex(0xF2UC),		uk_ex(0xF3UC),
			uk_ex(0xF4UC),		uk_ex(0xF5UC),		uk_ex(0xF6UC),		uk_ex(0xF7UC),
			uk_ex(0xF8UC),		uk_ex(0xF9UC),		uk_ex(0xFAUC),		uk_ex(0xFBUC),
			uk_ex(0xFCUC),		uk_ex(0xFDUC),		uk_ex(0xFEUC),		uk_ex(0xFFUC)
		}
	};
	static uint8_t pause_set2[]{ 0xE1UC, 0x14UC, 0x77UC, 0xE1UC, 0xF0UC, 0x14UC, 0xF0UC, 0x77UC };
#pragma endregion
	// Scanset 3 decode table — just one, as set 3 doesn't use the extension byte like the other 2 sets do
#pragma region scanset 3
	static scancode_table set3
	{
		{
			uk_sc(0x00UC),		uk_sc(0x01UC),		uk_sc(0x02UC),		uk_sc(0x03UC),
			uk_sc(0x00UC),		uk_sc(0x01UC),		uk_sc(0x02UC),		s_dn(KC_F1),
			c_dn('\033'),		uk_sc(0x09UC),		uk_sc(0x0AUC),		uk_sc(0x0BUC),
			uk_sc(0x0CUC),		c_dn('\t'),			c_dn('`'),			s_dn(KC_F2),
			uk_sc(0x10UC),		s_dn(KC_LCTRL),		s_dn(KC_LSHFT),		uk_sc(0x13UC),
			s_dn(KC_CAPS),		c_dn('q'),			c_dn('1'),			s_dn(KC_F3),
			uk_sc(0x18UC),		s_dn(KC_LALT),		c_dn('z'),			c_dn('s'),
			c_dn('a'),			c_dn('w'),			c_dn('2'),			s_dn(KC_F4),
			uk_sc(0x20UC),		c_dn('c'),			c_dn('x'),			c_dn('d'),
			c_dn('e'),			c_dn('4'),			c_dn('3'),			s_dn(KC_F5),
			uk_sc(0x28UC),		c_dn(' '),			c_dn('v'),			c_dn('f'),
			c_dn('t'),			c_dn('r'),			c_dn('5'),			s_dn(KC_F6),
			uk_sc(0x30UC),		c_dn('n'),			c_dn('b'),			c_dn('h'),
			c_dn('g'),			c_dn('y'),			c_dn('6'),			s_dn(KC_F7),
			uk_sc(0x28UC),		s_dn(KC_RALT),		c_dn('m'),			c_dn('j'),
			c_dn('u'),			c_dn('7'),			c_dn('8'),			s_dn(KC_F8),
			uk_sc(0x40UC),		c_dn(','),			c_dn('k'),			c_dn('i'),
			c_dn('o'),			c_dn('0'),			c_dn('9'),			s_dn(KC_F9),
			uk_sc(0x48UC),		c_dn('.'),			c_dn('/'),			c_dn('l'),
			c_dn(';'),			c_dn('p'),			c_dn('-'),			s_dn(KC_F10),
			uk_sc(0x50UC),		uk_sc(0x51UC),		c_dn('\''),			uk_sc(0x53UC),
			c_dn('['),			c_dn('='),			s_dn(KC_F11),		s_dn(KC_PRTSC),
			s_dn(KC_RCTRL),		s_dn(KC_RSHFT),		c_dn('\n'),			c_dn(']'),
			c_dn('\\'),			uk_sc(0x5DUC),		s_dn(KC_F12),		s_dn(KC_SCRLL),
			s_dn(KC_DOWN),		s_dn(KC_LEFT),		s_dn(KC_PAUSE),		s_dn(KC_UP),
			c_dn('\177'),		s_dn(KC_END),		c_dn('\b'),			s_dn(KC_INS),
			uk_sc(0x68UC),		n_dn('1'),			s_dn(KC_RIGHT),			n_dn('4'),
			n_dn('7'),			s_dn(KC_PG_DN),		s_dn(KC_HOME),		s_dn(KC_PG_UP),
			n_dn('0'),			n_dn('.'),			n_dn('2'),			n_dn('5'),
			n_dn('6'),			n_dn('8'),			s_dn(KC_NUM),		uk_sc(0x77UC),
			uk_sc(0x78UC),		n_dn('\n'),			n_dn('3'),			uk_sc(0x7BUC),
			n_dn('+'),			n_dn('9'),			n_dn('*'),			uk_sc(0x7FUC),
			uk_sc(0x80UC),		uk_sc(0x81UC),		uk_sc(0x82UC),		uk_sc(0x83UC),
			uk_sc(0x84UC),		uk_sc(0x85UC),		uk_sc(0x86UC),		uk_sc(0x87UC),
			uk_sc(0x88UC),		uk_sc(0x89UC),		uk_sc(0x8AUC),		s_dn(KC_LGUI),
			s_dn(KC_RGUI),		s_dn(KC_APPS),		uk_sc(0x8EUC),		uk_sc(0x8FUC),
			uk_sc(0x90UC),		uk_sc(0x91UC),		uk_sc(0x92UC),		uk_sc(0x93UC),
			uk_sc(0x94UC),		uk_sc(0x95UC),		uk_sc(0x96UC),		uk_sc(0x97UC),
			uk_sc(0x98UC),		uk_sc(0x99UC),		uk_sc(0x9AUC),		uk_sc(0x9BUC),
			uk_sc(0x9CUC),		uk_sc(0x9DUC),		uk_sc(0x9EUC),		uk_sc(0x9FUC),
			uk_sc(0xA0UC),		uk_sc(0xA1UC),		uk_sc(0xA2UC),		uk_sc(0xA3UC),
			uk_sc(0xA4UC),		uk_sc(0xA5UC),		uk_sc(0xA6UC),		uk_sc(0xA7UC),
			uk_sc(0xA8UC),		uk_sc(0xA9UC),		uk_sc(0xAAUC),		uk_sc(0xABUC),
			uk_sc(0xACUC),		uk_sc(0xADUC),		uk_sc(0xAEUC),		uk_sc(0xAFUC),
			uk_sc(0xB0UC),		uk_sc(0xB1UC),		uk_sc(0xB2UC),		uk_sc(0xB3UC),
			uk_sc(0xB4UC),		uk_sc(0xB5UC),		uk_sc(0xB6UC),		uk_sc(0xB7UC),
			uk_sc(0xB8UC),		uk_sc(0xB9UC),		uk_sc(0xBAUC),		uk_sc(0xBBUC),
			uk_sc(0xBCUC),		uk_sc(0xBDUC),		uk_sc(0xBEUC),		uk_sc(0xBFUC),
			uk_sc(0xC0UC),		uk_sc(0xC1UC),		uk_sc(0xC2UC),		uk_sc(0xC3UC),
			uk_sc(0xC4UC),		uk_sc(0xC5UC),		uk_sc(0xC6UC),		uk_sc(0xC7UC),
			uk_sc(0xC8UC),		uk_sc(0xC9UC),		uk_sc(0xCAUC),		uk_sc(0xCBUC),
			uk_sc(0xCCUC),		uk_sc(0xCDUC),		uk_sc(0xCEUC),		uk_sc(0xCFUC),
			uk_sc(0xD0UC),		uk_sc(0xD1UC),		uk_sc(0xD2UC),		uk_sc(0xD3UC),
			uk_sc(0xD4UC),		uk_sc(0xD5UC),		uk_sc(0xD6UC),		uk_sc(0xD7UC),
			uk_sc(0xD8UC),		uk_sc(0xD9UC),		uk_sc(0xDAUC),		uk_sc(0xDBUC),
			uk_sc(0xDCUC),		uk_sc(0xDDUC),		uk_sc(0xDEUC),		uk_sc(0xDFUC),
			uk_sc(0xE0UC),		uk_sc(0xE1UC),		uk_sc(0xE2UC),		uk_sc(0xE3UC),
			uk_sc(0xE4UC),		uk_sc(0xE5UC),		uk_sc(0xE6UC),		uk_sc(0xE7UC),
			uk_sc(0xE8UC),		uk_sc(0xE9UC),		uk_sc(0xEAUC),		uk_sc(0xEBUC),
			uk_sc(0xECUC),		uk_sc(0xEDUC),		uk_sc(0xEEUC),		uk_sc(0xEFUC),
			s_up(KC_BRB),		uk_sc(0xF1UC),		uk_sc(0xF2UC),		uk_sc(0xF3UC),
			uk_sc(0xF4UC),		uk_sc(0xF5UC),		uk_sc(0xF6UC),		uk_sc(0xF7UC),
			uk_sc(0xF8UC),		uk_sc(0xF9UC),		uk_sc(0xFAUC),		uk_sc(0xFBUC),
			uk_sc(0xFCUC),		uk_sc(0xFDUC),		uk_sc(0xFEUC),		uk_sc(0xFFUC)
		},
		{ /** N/A for set 3 */ }
	};
#pragma endregion
	// Helper functions for handling the character value in the packets and the multiplexing for the scanset tables
#pragma region miscellaneous helpers
	// Case difference, for shifts and caps lock
	constexpr char case_diff 					= 'a' - 'A';
	static std::array<char, 127> shift_table	= std::ext::range_evaluate<char, '\177'>([](char value) -> char
	{
		if(value >= 'a' && value <= 'z') return value - case_diff;
		else if(value >= 'A' && value <= 'Z') return value + case_diff;
		switch(value)
		{
			case ',': 		return '<';
			case '.': 		return '>';
			case '/': 		return '?';
			case ';': 		return ':';
			case '\'':		return '"';
			case '[':		return '{';
			case ']':		return '}';
			case '\\':		return '|';
			case '`':		return '~';
			case '1':		return '!';
			case '2':		return '@';
			case '3':		return '#';
			case '4':		return '$';
			case '5':		return '%';
			case '6':		return '^';
			case '7':		return '&';
			case '8':		return '*';
			case '9':		return '(';
			case '0':		return ')';
			case '-':		return '_';
			case '=':		return '+';
			default:		return value;
		}
	});
	static keycode numpad_shift(wchar_t base)
	{
		switch(static_cast<char>(base))
		{
			case '0':		return KC_INS;
			case '1':		return KC_END;
			case '2':		return KC_DOWN;
			case '3':		return KC_PG_DN;
			case '4':		return KC_LEFT;
			case '6':		return KC_RIGHT;
			case '7':		return KC_HOME;
			case '8':		return KC_UP;
			case '9':		return KC_PG_UP;
			case '.':		return static_cast<keycode>(L'\177');
			default:		return static_cast<keycode>(base);
		}
	}
	static keycode compute_by_state(keyboard_event const& e)
	{
		wchar_t value	= e;
		bool is_shift	= e.kv_vstate.shift();
		if(e.kv_numpad && (e.kv_vstate.num_lock || is_shift)) return numpad_shift(value);
		if(e.kv_vstate.caps_lock && (value >= 'a' && value <= 'z')) value -= case_diff;
		if(value < 127 && is_shift) value = shift_table[value];
		return static_cast<keycode>(value);
	}
	static keyboard_event const* base_scan_table(keyboard_scanset ss) noexcept { return ss == SC_SET2 ? set2[0] : ss == SC_SET3 ? set3[0] : set1[0]; }
	static keyboard_event const* escape_table(keyboard_scanset ss) noexcept { return ss == SC_SET3 ? nullptr : ss == SC_SET2 ? set2[1] : set1[1]; }
	static uint8_t const* pause_sequence(keyboard_scanset ss) noexcept { return ss == SC_SET3 ? nullptr : ss == SC_SET2 ? pause_set2 : pause_set1; }
	static size_t pause_sequence_length(keyboard_scanset ss) noexcept { return ss == SC_SET3 ? 0UZ : ss == SC_SET2 ? sizeof(pause_set2) : sizeof(pause_set1); }
#pragma endregion
	uint16_t ps2_keyboard_controller::id_word() const noexcept { return __be16(__state.id_bytes); }
	typematic_byte ps2_keyboard_controller::typematic() const noexcept { return __state.typematic; }
	keyboard_scanset ps2_keyboard_controller::scanset() const noexcept { return __state.scanset; }
	ps2_keyboard_controller::operator bool() const noexcept { return __state_valid; }
	ps2_keyboard::ps2_keyboard(ps2_controller& ps2) : __controller(ps2), __decoder(), __input_queue(8UZ), __listeners() { __init(); }
	keyboard_event keyboard_scan_decoder::__decode_one(uint8_t scan, byte_queue& rem) const
	{
		keyboard_event e	= __scans[scan];
		if(e == KC_EXB) {
			if(__unlikely(!rem)) return uk_sc(scan);
			return __decode_one_escaped(rem.pop(), rem);
		}
		else if(e == KC_SEQB) {
			if(__unlikely(!rem)) return uk_sc(scan);
			return __decode_one_seq(scan, rem);
		}
		else if(e == KC_BRB) {
			if(__unlikely(!rem)) return uk_sc(scan);
			return to_break(__decode_one(rem.pop(), rem));
		}
		else return e;
	}
	keyboard_event keyboard_scan_decoder::__decode_one_escaped(uint8_t scan, byte_queue& rem) const
	{
		keyboard_event e	= __escaped_scans[scan];
		if(e == KC_BRB) {
			if(__unlikely(!rem)) return uk_ex(scan);
			e				= to_break(__decode_one_escaped(rem.pop(), rem));
		}
		if(e.kv_multiscan)
		{
			if(__unlikely(!rem)) return uk_ex(scan);
			uint8_t next		= rem.peek();
			keyboard_event f	= __decode_one(rem.pop(), rem);
			if(f.kv_code != e.kv_code || f.kv_release != e.kv_release) return uk_seq(scan, next);
		}
		return e;
	}
	keyboard_event keyboard_scan_decoder::__decode_one_seq(uint8_t seq_bookend, byte_queue& rem) const
	{
		if(__unlikely(!__pause_sequence || seq_bookend != __pause_sequence[0] || !rem)) return uk_sc(seq_bookend);
		uint8_t first = rem.peek(), second{}, curr{};
		for(size_t i = 1; i < __pause_sequence_length; i++)
		{
			if(__unlikely(!rem)) goto seq_fail;
			curr 					= rem.pop();
			if(i == 2) second 		= curr;
			if(curr != __pause_sequence[i]) goto seq_fail;
		}
		return pause_packet;
seq_fail:
		return uk_seq(first, second);
	}
	keyboard_scan_decoder::keyboard_scan_decoder(keyboard_scanset ss) noexcept :
		__scans(base_scan_table(ss)),
		__escaped_scans(escape_table(ss)),
		__pause_sequence(pause_sequence(ss)),
		__pause_sequence_length(pause_sequence_length(ss)),
		current_state(),
		led_state()
	{}
	keyboard_scan_decoder::keyboard_scan_decoder() noexcept :
		__scans(set2[0]),
		__escaped_scans(set2[1]),
		__pause_sequence(pause_set2),
		__pause_sequence_length(sizeof(pause_set2)),
		current_state(),
		led_state()
	{}
	void keyboard_scan_decoder::set_scanset(keyboard_scanset ss) noexcept
	{
		__scans 				= base_scan_table(ss);
		__escaped_scans 		= escape_table(ss);
		__pause_sequence 		= pause_sequence(ss);
		__pause_sequence_length	= pause_sequence_length(ss);
	}
	keyboard_event keyboard_scan_decoder::decode(byte_queue& scan_bytes)
	{
		if(__unlikely(!scan_bytes)) return uk_sc(0x00UC);
		keyboard_event e 		= __decode_one(scan_bytes.pop(), scan_bytes);
		if((dword(e).hi.hi & 0x0EUC) != 0x0EUC)
		{
			uint8_t mod_state	= std::bit_cast<uint8_t>(current_state) ^ std::bit_cast<uint8_t>(e.kv_vstate);
			e.kv_vstate			= current_state;
			current_state		= std::bit_cast<keyboard_vstate>(mod_state);
			if(!e.kv_release)
			{
				led_state.caps_lock		^= (e == KC_CAPS);
				led_state.num_lock		^= (e == KC_NUM);
				led_state.scroll_lock	^= (e == KC_SCRLL);
			}
			e.kv_code			= static_cast<wchar_t>(compute_by_state(e));
		}
		scan_bytes.flush();
		return e;
	}
	void ps2_keyboard_controller::__send_cmd_byte(keyboard_cmd_byte b)
	{
		uint8_t response_byte{};
		size_t attempts{};
		do {
			if(!(__ps2_controller << b).wait_for(response_byte)) throw std::runtime_error("[KBD] timeout waiting for controller");
			attempts++;
			if(attempts > 3UZ) throw std::runtime_error("[KBD] unsupported command or controller error");
		} while(response_byte != sig_keybd_ack);
	}
	void ps2_keyboard_controller::__execute_next() noexcept
	{
		keyboard_command cmd = __cmd_queue.pop();
		try
		{
			if(cmd.expects_response) __send_cmd_byte(KBC_SCDIS);
			__send_cmd_byte(cmd.cmd_byte);
			if(cmd.expects_response)
			{
				if(cmd.response_out)
					if(__unlikely(!__ps2_controller.wait_for(cmd.response_out.deref<uint8_t>())))
						cmd.response_out.deref<int8_t>()	= -1SC;
				__send_cmd_byte(KBC_SCEN);
			}
		}
		catch(std::exception& e)
		{
			panic(e.what());
			if(cmd.response_out)
				cmd.response_out.deref<int8_t>()			= -1SC;
		}
	}
	__nointerrupts bool ps2_keyboard_controller::__initialize() noexcept
	{
		if(__unlikely(!__ps2_controller.ready && !ps2_init(__ps2_controller))) return false;
		try
		{
			uint8_t echo_reply;
			size_t attempts{}, n{};
			do {
				if(attempts++ > 3UZ) throw std::runtime_error("[KBD] keyboard is absent or nonfunctional");
				__send_cmd_byte(KBC_RESET);
				(__ps2_controller << KBC_ECHO) >> echo_reply;
			} while(echo_reply != sig_keybd_ping);
			__send_cmd_byte(KBC_SCDIS);
			__send_cmd_byte(KBC_IDENTIFY);
			for(size_t i = 0UZ; i < 100UZ && n < 2UZ; i++)
				if(__ps2_controller >> __state.id_bytes[n])
					n++;
			__send_cmd_byte(KBC_PARAMRST);
			__send_cmd_byte(KBC_SCEN);
			interrupt_table::add_irq_handler(0UC, [this]() -> void { while(__cmd_queue) __execute_next(); });
			return true;
		}
		catch(std::exception& e) { panic(e.what()); }
		return false;
	}
	ps2_keyboard_controller::ps2_keyboard_controller(ps2_controller& ps2) noexcept :
		__ps2_controller(ps2),
		__cmd_queue(),
		__state(SC_SET2, typematic_byte()),
		__state_valid(__initialize())
	{}
	bool ps2_keyboard_controller::enqueue_command(keyboard_command&& cmd) noexcept
	{
		if(__unlikely(!__state_valid)) return false;
		try { __cmd_queue.push(std::move(cmd)); return true; }
		catch(std::exception& e) { panic(e.what()); }
		return false;
	}
	bool ps2_keyboard_controller::scanset(keyboard_scanset value) noexcept
	{
		// nothing to do if the value is already there
		if(__unlikely(__state.scanset == value)) return true;
		keyboard_command cmd(true, false, KBC_SCANSET);
		switch(value)
		{
		case SC_SET1:
			cmd.sub = 1UC;
			break;
		case SC_SET2:
			cmd.sub = 2UC;
			break;
		case SC_SET3:
			cmd.sub = 3UC;
			break;
		default:
			return false;
		}
		return enqueue_command(std::move(cmd));
	}
	bool ps2_keyboard_controller::typematic(typematic_byte value) noexcept
	{
		uint8_t as_uchar = std::bit_cast<uint8_t>(value);
		// nothing to do if the value is already there
		if(__unlikely(as_uchar == std::bit_cast<uint8_t>(__state.typematic))) return true;
		keyboard_command cmd
		{
			.has_sub	= true,
			.cmd_byte	= KBC_TYPEMATIC,
			.sub		= as_uchar
		};
		return enqueue_command(std::move(cmd));
	}
	bool ooos::ps2_keyboard_controller::set_leds(keyboard_lstate lstate) noexcept
	{
		uint8_t as_uchar = std::bit_cast<uint8_t>(lstate);
		keyboard_command cmd
		{
			.has_sub 	= true,
			.cmd_byte 	= KBC_SET_LEDS,
			.sub 		= as_uchar
		};
		return enqueue_command(std::move(cmd));
	}
	void ps2_keyboard::__init()
	{
		if(__unlikely(!__controller))
			return;
		interrupt_table::add_irq_handler(1UC, [this]() -> void { __on_irq(); });
	}
	void ps2_keyboard::__on_irq()
	{
		ps2_controller& ps2	= __controller.__ps2_controller;
		ps2_status_byte status{};
		uint8_t data_byte{};
		do {
			if(ps2 >> data_byte)
				__input_queue.push(data_byte);
			status			= __controller.__ps2_controller.status();
		} while(status.ps2_out_avail);
		if(__unlikely(!__input_queue)) return;
		uint8_t lights		= std::bit_cast<uint8_t>(__decoder.led_state);
		keyboard_event e	= __decoder.decode(__input_queue);
		if(uint8_t nlights 	= std::bit_cast<uint8_t>(__decoder.led_state); nlights != lights) __controller.set_leds(__decoder.led_state);
		for(keyboard_listener_registry::value_type const& p : __listeners) p.second(e);
	}
}