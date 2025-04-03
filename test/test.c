#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <errno.h>
int main(int argc, char** argv)
{
    printf("Hello OoOS (from userland)! I'm %s!\n", argv[0]);
    uint64_t tm = time(NULL);
    printf("The timestamp is %lu\n", tm);
    char* test_str = (char*)mmap(NULL, 4096, PROT_WRITE | PROT_READ, MAP_ANONYMOUS, 0, 0);
    if((void*)test_str != (void*)-1)
    {
        int len = snprintf(test_str, 4096, "Address of buffer: %p; number of arguments is %i\n", (void*)test_str, argc);
        char* copied = (char*)malloc(len + 1);
        strncpy(copied, test_str, len);
        copied[len] = 0;
        printf("%s", copied);
        free(copied);
        munmap(test_str, 4096);
    }
    else
    {
        printf("mmap failed; error code %i\n", errno);
        return errno;
    }
    return 0;
}