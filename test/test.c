#include <stdio.h>
#include <time.h>
int main(int argc, char** argv)
{
    printf("Hello OoOS (from userland)! I'm %s!\n", argv[0]);
    uint64_t tm = time(NULL);
    printf("The timestamp is %lu\n", tm);
    return 0;
}