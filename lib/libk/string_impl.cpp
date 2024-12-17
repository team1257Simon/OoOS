#include "kernel/libk_decls.h"
#include "string.h"

extern "C"
{
    char* strstr (const char *hs, const char *ne)
    {
        char c = ne[0];
        if (!c) return (char*)hs;
        for (; hs[0]; hs++)
        {
            if (hs[0] != c) continue;
            size_t i;
            for (i = 1; ne[i]; i++) if (hs[i] != ne[i]) break;
            if (!ne[i]) return (char*)hs;
        }
        return NULL;
    }
}
