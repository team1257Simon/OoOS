#ifndef __USER_INFO
#define __USER_INFO
#include "sys/types.h"
struct user_info
{
    uid_t   uid;
    gid_t   gid;
    char*   display_name;
    char*   home_directory;
    addr_t  permission_struct_ptr;
    addr_t  credentials_cache_ptr;
    size_t  process_count;
    size_t  process_quota;
    // ...
} __pack;
#endif