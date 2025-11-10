#include "kernel/kernel_defs.h"
extern sysinfo_t* sysinfo;
bool checksum(acpi_header* h) { char* c = reinterpret_cast<char*>(h); signed char sum = 0; for(size_t i = 0; i < h->length; i++) sum += c[i]; return sum == 0; }
bool matches(acpi_header* h, const char* expected_sig) { return __builtin_memcmp(h->signature, expected_sig, 4) == 0; }
void* find_system_table(const char* expected_sig) 
{
    addr_t* ptrs 		= addr_t(sysinfo->xsdt).plus(sizeof(acpi_header));
    size_t total_len 	= ((sysinfo->xsdt->hdr.length - sizeof(acpi_header)) / sizeof(addr_t));
    for(size_t i = 0; i < total_len; i++) { if(matches(ptrs[i], expected_sig) && checksum(ptrs[i])) return ptrs[i]; } 
    return nullptr;
}