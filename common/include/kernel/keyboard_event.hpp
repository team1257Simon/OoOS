#ifndef __KBD_EVENT
#define __KBD_EVENT
#include <libk_decls.h>
namespace ooos
{
	enum class keycode : wchar_t
	{
		KC_MAX_CHAR     = 0x10FFFFU,        // Anything greater than this is a special key, an escape byte, or an unknown scan
		KC_F1           = 0x110001U,
		KC_F2           = 0x110002U,
		KC_F3           = 0x110003U,
		KC_F4           = 0x110004U,
		KC_F5           = 0x110005U,
		KC_F6           = 0x110006U,
		KC_F7           = 0x110007U,
		KC_F8           = 0x110008U,
		KC_F9           = 0x110009U,
		KC_F10          = 0x11000AU,
		KC_F11          = 0x11000BU,
		KC_F12          = 0x11000CU,
		KC_LCTRL        = 0x120001U,        // Left Ctrl
		KC_LSHFT        = 0x120002U,        // Left Shift
		KC_LALT         = 0x120003U,        // Left Alt
		KC_RCTRL        = 0x120004U,        // Right Ctrl
		KC_RSHFT        = 0x120005U,        // Right Shift
		KC_RALT         = 0x120006U,        // Right Alt
		KC_LGUI         = 0x120007U,        // Left Windows/Super/GUI key
		KC_RGUI         = 0x120008U,        // Right Windows/Super/GUI key
		KC_CAPS         = 0x120009U,        // Caps Lock
		KC_NUM          = 0x12000AU,        // Num Lock
		KC_SCRLL        = 0x12000BU,        // Scroll Lock
		KC_INS          = 0x12000CU,        // Insert
		KC_PAUSE        = 0x130001U,        // Pause
		KC_PRTSC        = 0x130002U,        // Print Screen
		KC_END          = 0x140001U,
		KC_HOME         = 0x140002U,
		KC_PG_UP        = 0x140003U,        // Page Up
		KC_PG_DN        = 0x140004U,        // Page Down
		KC_LEFT         = 0x140005U,
		KC_RIGHT        = 0x140006U,
		KC_UP           = 0x140007U,
		KC_DOWN         = 0x140008U,
		KC_POWER        = 0x150001U,
		KC_SLEEP        = 0x150002U,
		KC_WAKE         = 0x150003U,
		KC_M1           = 0x160001U,        // Multimedia key 1 (Previous track)
		KC_M2           = 0x160002U,        // Multimedia key 2 (Next track)
		KC_M3           = 0x160003U,        // Multimedia key 3 (Mute)
		KC_M4           = 0x160004U,        // Multimedia key 4 (Calculator)
		KC_M5           = 0x160005U,        // Multimedia key 5 (Play)
		KC_M6           = 0x160006U,        // Multimedia key 6 (Stop)
		KC_M7           = 0x160007U,        // Multimedia key 7 (Volume down)
		KC_M8           = 0x160008U,        // Multimedia key 8 (Volume up)
		KC_M9           = 0x160009U,        // Multimedia key 9 (WWW home)
		KC_M10          = 0x16000AU,        // Multimedia key 10 (WWW search)
		KC_M11          = 0x16000BU,        // Multimedia key 11 (WWW favorites)
		KC_M12          = 0x16000CU,        // Multimedia key 12 (WWW refresh)
		KC_M13          = 0x16000DU,        // Multimedia key 13 (WWW stop)
		KC_M14          = 0x16000EU,        // Multimedia key 14 (WWW forward)
		KC_M15          = 0x16000FU,        // Multimedia key 15 (WWW back)
		KC_M16          = 0x160010U,        // Multimedia key 16 (My Computer)
		KC_M17          = 0x160011U,        // Multimedia key 17 (Email)
		KC_M18          = 0x160012U,        // Multimedia key 18 (Media Select)
		KC_APPS         = 0x160013U,        // "Apps" key
		KC_BRB          = 0x1700F0U,        // Placeholder for a break code escape byte in scancode tables (generally 0xF0)
		KC_EXB          = 0x1700E0U,        // Placeholder for a general escape byte in scancode tables (generally 0xE0)
		KC_SEQB         = 0x1700E1U,        // Placeholder for an atomic sequence (i.e. no separate break sequence) escape byte in scancode tables (generally 0xE1)
		KC_SEQ_UNKNOWN  = 0x1E0000U,        // Placeholder for an unrecognized scan sequence; the first 2 scancodes received will be a little-endian word in the low 16 bits
		KC_UNKNOWN      = 0x1F0000U,        // Placeholder for an unrecognized single-byte scan; the scancode will be in the lowest 8 bits
		KC_EXT_UNKNOWN  = 0x1FE000U,        // Placeholder for an unrecognized escaped scan; the unrecognized byte (i.e. after the escape byte) will be in the lowest 8 bits
		// Note: Multimedia keys are not present on all keyboards. I am also uncertain as to the mappings' consistency, hence the use of generic numbers.
		// The mappings given are the ones mentioned on OSDev Wiki.
	};
	// State-vector for the special modifier keys (shift, alt, etc.) on the keyboard.
	struct __pack keyboard_vstate
	{
		bool left_ctrl		: 1	= false;
		bool left_shift		: 1	= false;
		bool left_alt		: 1	= false;
		bool left_gui		: 1	= false;
		bool right_ctrl		: 1	= false;
		bool right_shift	: 1	= false;
		bool right_alt		: 1	= false;
		bool right_gui		: 1	= false;
		constexpr bool ctrl() const noexcept	{ return left_ctrl	|| right_ctrl; }
		constexpr bool alt() const noexcept		{ return left_alt	|| right_alt; }
		constexpr bool shift() const noexcept	{ return left_shift	|| right_shift; }
		constexpr bool gui() const noexcept		{ return left_gui	|| right_gui; }
	};
	struct __pack keyboard_event
	{
		// The unicode codepoint of the character, if any, or the keycode for a special key (arrow keys, shift, caps lock, Fn keys, etc.).
		// The value of a special key will be out of range for unicode, so any such value indicates a special key.
		wchar_t kv_code     : 21	= L'\0';
		// If this bit is set, this is a break (key-up) event; if it is clear, it is a make (key-down) event.
		bool kv_release     : 1		= false;
		// If this bit is set, the key is a num-pad numeral or symbol (the character code will remain the same).
		bool kv_numpad      : 1		= false;
		// If this bit is set, the key generated multiple scancodes (other than escape bytes; e.g. BREAK or PAUSE in sets 1 and 2).
		bool kv_multiscan   : 1		= false;
		// The state of the keyboard BEFORE the event — for example, if the key is right shift and the event is a key up, the right shift bit will still be set.
		// In prototype packets (the ones in the decode tables), the value will instead be an XOR mask to compute the state following the event.
		keyboard_vstate kv_vstate;
		// Extracting the character value from the packet requires some special handling to avoid accidental bit-field sign-extension.
		constexpr operator wchar_t() const noexcept { return static_cast<wchar_t>(kv_code & 0x1FFFFFU); }
		// Extracting the keycode value from the packet requires some special handling to avoid accidental bit-field sign-extension.
		constexpr operator keycode() const noexcept { return static_cast<keycode>(kv_code & 0x1FFFFFU); }
	};
	// State-vector for the locks (and their corresponding LEDs) on the keyboard.
	// Bits 3-7 might have different meanings on certain keyboards, but are not specified by the PS/2 standard for English QWERTY keyboards.
	// The three lowest bits are defined by the PS/2 standard (i.e. for their LEDs).
	// For convenience, the controller will store these locks in the same format.
	struct __pack keyboard_lstate
	{
		bool scroll 	: 1 = false;
		bool num		: 1 = false;
		bool caps		: 1	= false;
		bool l3			: 1 = false;
		bool l4			: 1 = false;
		bool l5			: 1 = false;
		bool l6			: 1 = false;
		bool l7			: 1 = false;
	};
}
#endif