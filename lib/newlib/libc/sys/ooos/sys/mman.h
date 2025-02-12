#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H
#include <sys/types.h>
#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */
/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
/* Other flags.  */
#define MAP_FIXED	0x10
#define MAP_FILE	0
#define MAP_ANONYMOUS	0x20
void* mmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset);
int munmap(void* addr, size_t len);
#endif