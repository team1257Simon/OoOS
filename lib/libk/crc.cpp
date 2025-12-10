#include <libk_decls.h>
#include <array>
// 0xA001 is the bit-reversed value of the polynomial 0x8005
static const std::array<uint16_t, 256UZ> crc16_table	= sequence_transform(crc16_table_val<0xA001US>, std::make_integer_sequence<uint16_t, 256US>());
uint16_t crc16_calc(const void* data, size_t len, uint16_t seed)
{
	uint8_t const*	cdata	= static_cast<uint8_t const*>(data);
	uint16_t		result	= seed;
	for(size_t i = 0UZ; i < len; i++) { result = (((result >> 8) & 0xFFUS) ^ crc16_table[(result ^ cdata[i]) & 0xFFUS]) & 0xFFFFUS; }
	return result;
}