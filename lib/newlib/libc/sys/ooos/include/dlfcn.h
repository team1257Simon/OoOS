#include <stdint.h>
/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY	0x00001	/* Lazy function call binding.  */
#define RTLD_NOW	0x00002	/* Immediate function call binding.  */
#define	RTLD_BINDING_MASK   0x3	/* Mask of binding time value.  */
#define RTLD_NOLOAD	    0x00004	/* Do not load the object.  */
#define RTLD_DEEPBIND	0x00008	/* Use deep binding.  */
/* If the following bit is set in the MODE argument to `dlopen', the symbols of the loaded object and its dependencies are made visible as if the object were linked directly into the program.  */
#define RTLD_GLOBAL	0x00100
/* Unix98 demands the following flag which is the inverse to RTLD_GLOBAL. The implementation does this by default and so we can define the value to zero.  */
#define RTLD_LOCAL	0
/* Do not delete object when closed.  */
#define RTLD_NODELETE	0x01000
struct link_map
{
    uintptr_t   l_addr;
    char*       l_name;
    uint64_t*   l_ld;
    struct link_map* l_next;
    struct link_map* l_prev;
};
typedef struct
{
    const char* dli_fname;	    /* File name of defining object.  */
    void* dli_fbase;		    /* Load address of that object.  */
    const char* dli_sname;	    /* Name of nearest symbol.  */
    void* dli_saddr;		    /* Exact value of nearest symbol.  */
} Dl_info;
enum
{
    RTLD_DI_LMID = 1,           /* Treat ARG as `lmid_t *'; store namespace ID for HANDLE there. (Currently this will always be the default namespace value of 0) */
    RTLD_DI_LINKMAP = 2,        /* Treat ARG as `struct link_map **'; store the `struct link_map *' for HANDLE there.  */
    RTLD_DI_CONFIGADDR = 3,	    /* Unsupported, defined by Solaris.  */
    RTLD_DI_SERINFO = 4,        /* NYI */
    RTLD_DI_SERINFOSIZE = 5,
    RTLD_DI_ORIGIN = 6,         /* Treat ARG as `char *', and store there the full path name of this object file (i.e. $ORIGIN/[object file name]).  */
    RTLD_DI_PROFILENAME = 7,	/* Unsupported, defined by Solaris.  */
    RTLD_DI_PROFILEOUT = 8,	    /* Unsupported, defined by Solaris.  */
    RTLD_DI_TLS_MODID = 9,      /* NYI */
    RTLD_DI_TLS_DATA = 10,      /* NYI */
    RTLD_DI_MAX = 10
};
extern void* dlopen(const char* __file, int __mode);
extern int dlclose(void* __handle);
extern void* dlsym(void* __handle, const char* __name);
extern char* dlerror(void);
int dladdr(const void* addr, void* dli);
int dlinfo(void* handle, int req, void* out);