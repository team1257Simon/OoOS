#ifndef __FONT_H
#define __FONT_H
#include <kernel_defs.h>
extern "C"
{
	extern const struct __pack psf2_t
	{
		uint32_t magic;
		uint32_t version;
		uint32_t headersize;
		uint32_t flags;
		uint32_t numglyph;
		uint32_t bpg;
		uint32_t height;
		uint32_t width;
		uint8_t glyph_data[];
	} __startup_font_data;
}
#endif