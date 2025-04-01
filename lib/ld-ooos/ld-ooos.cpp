#include "ld-ooos.hpp"
#include <sys/errno.h>
const char path_var_str[] = "LD_LIBRARY_PATH=";
constexpr size_t name_str_size = sizeof(path_var_str);
extern "C"
{
    void dlbegin(void* phandle, char** argv, char** env)
    {
        // The kernel will call this function to invoke the dynamic linker. The handle is the program handle to pass in syscalls as part of the setup.
        // argv is the argument string vector, and env is the environment variable string vector. Both are null-terminated (argc is technically redundant)
        // Here we will load all the dependencies of the program (per depends()) before calling all the initialization functions in the program.
        exit(0); // WIP
    }
    void dlend(void* phandle)
    {
        // The kernel will call this function to invoke the dynamic linker. The handle is the program handle to pass in syscalls as part of the setup.
        // Here we will call the destructors for the object before returning to the kernel.
        exit(0); // WIP
    }
}