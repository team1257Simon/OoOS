#include "ld-ooos.hpp"
extern "C"
{
    void dlenter(void* phandle, int envc, char** env)
    {
        // The kernel will call this function to invoke the dynamic linker. The handle is the program handle to pass in syscalls as part of the setup.
        // envc is the number of environment variables, and env is the environment variable strings.
        // Here we will load all the dependencies of the program (per depends()) before calling all the initialization functions in the program.
        exit(0); // WIP
    }
    void dlexit(void* phandle)
    {
        // The kernel will call this function to invoke the dynamic linker. The handle is the program handle to pass in syscalls as part of the setup.
        // Here we will call the destructors for the object before returning to the kernel.
        exit(0); // WIP
    }
}