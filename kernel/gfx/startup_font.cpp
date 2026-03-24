#include <array>
template<size_t N>
struct psf2
{
	uint32_t magic;
	uint32_t version;
	uint32_t headersize;
	uint32_t flags;
	uint32_t numglyph;
	uint32_t bpg;
	uint32_t height;
	uint32_t width;
	uint8_t glyph_data[N - 32];
};
constexpr static size_t file_size = std::array
{
	//	first get the number of bytes
	//	annoyingly, #embed only allows for int literals
	#embed "startup_font.psf"
}.size();
constexpr static std::array<uint8_t, file_size> arr
{
	//	this time we get the data for real; technically this could be the symbol
	//	however, doing it this way lets us use intellisense tooltips to inspect the data in the psf2 header via some bit_cast abuse
	#embed "startup_font.psf"
};
extern "C" { constinit psf2<file_size> __startup_font_data = std::bit_cast<psf2<file_size>>(arr); }