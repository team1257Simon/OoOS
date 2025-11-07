#include <sys/types.h>
#include <sys/errno.h>
extern void *memalign(size_t alignment, size_t size);
extern int rand();
__attribute__((weak)) int posix_memalign(void** memptr, size_t alignment, size_t size)
{
	if(__builtin_expect(alignment % sizeof(void*) != 0 || __builtin_popcount(alignment) > 1, 0)) { *memptr = NULL; return EINVAL; }
	void* result = memalign(alignment, size);
	if(!result) return ENOMEM;
	*memptr = result;
	return 0;
}
__attribute__((weak)) int fcntl(int, int, ...)
{
	// NYI
	errno = ENOSYS;
	return -1;
}
__attribute__((weak)) int getentropy(void* buffer, unsigned long length)
{
	if(__builtin_expect(length > 256, 0)) { errno = EIO; return -1; }
	void* buf_end = ((unsigned char*)buffer) + length;
	while(buffer < buf_end) { *((unsigned char*)buffer++) = rand() % 256; }
	return 0;
}