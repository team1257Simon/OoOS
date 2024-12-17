#include "kernel/libk_decls.h"
#include "kernel/font.h"
extern psf2_t *__startup_font;

extern "C"
{
    void kmain(framebuf_t* fb, mmap_t* mmap, pagefile* pg)
    {
        font_render render{__startup_font, fb->pitch};
        alset(fb->ptr, 0, fb->height * fb->width);
        const char* msg = "Hi there";
        for(unsigned int i = 0; i < sizeof(msg); i++) 
        {
            render(msg[i], fb->ptr, {i,0});
        }
        while(1);
    }
}