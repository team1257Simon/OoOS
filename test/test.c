#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
int	mkdir (const char *_path, mode_t __mode);
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
	FILE* f = fopen("/files/memes.txt", "a+");
	if(f)
	{
		fprintf(f, "blerple derple\n");
		fclose(f);
		f = fopen("/files/dreams.txt", "w+");
		if(f) { fprintf(f, "merple yerple\n"); fclose(f); }
		else printf("no file :(\n");
	}
	else printf("no file D:\n");
	if((mkdir("/files/fleedledeedle", 0774) < 0) && errno != EEXIST) printf("no directory creation q.q\n");
	else
	{
		FILE* g = fopen("/files/fleedledeedle/flaboogle.txt", "w+");
		if(g) { fprintf(g, "sha-dizzle!\n"); fclose(g); }
		else printf("no file ;-;\n");
	}
	long pid = fork();
	printf("forked\n");
	int rv = 0;
	switch(pid)
	{
	case -1:
		printf(":(\n");
		return -1;
	case 0:
		printf("In child\n");
		return 0;
	default:
		printf("Child PID is %li\n", pid);
		wait(&rv);
		return 0;
	}
}