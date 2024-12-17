#include "kernel/kernel_defs.h"
#include "kernel/elf64.h"
#include "uefi/uefi.h"

#define OK 0
#define EMALLOC 1
#define ENOMMAP 2
#define EFOPEN 3
#define ENOELF 4
#define ENOGFX 6
#define ESETGFX 7
#define MMAP_MAX_PG 0x200000
#define PAGESIZE 0x1000
#define PT_LEN 0x200
#define K_ADDR 0x2000000

const char *types[] = 
{
    "EfiReservedMemoryType",        // 0
    "EfiLoaderCode",                // 1
    "EfiLoaderData",                // 2
    "EfiBootServicesCode",          // 3
    "EfiBootServicesData",          // 4
    "EfiRuntimeServicesCode",       // 5
    "EfiRuntimeServicesData",       // 6
    "EfiConventionalMemory",        // 7
    "EfiUnusableMemory",            // 8
    "EfiACPIReclaimMemory",         // 9
    "EfiACPIMemoryNVS",             // 10
    "EfiMemoryMappedIO",            // 11
    "EfiMemoryMappedIOPortSpace",   // 12
    "EfiPalCode"                    // 13
};

paging_table __boot_pml4 = NULL;
pagefile* __boot_pagefile = NULL;
framebuf_t* fb = NULL;
paging_table* pg_addrs;

inline static bool validate_elf(elf64_ehdr * elf)
{
    return (memcmp(elf->e_ident, ELFMAG, SELFMAG) == 0  /* magic match? */
        && elf->e_ident[EI_CLASS] == ELFCLASS64         /* 64 bit? */
        && elf->e_ident[EI_DATA] == ELFDATA2LSB         /* LSB? */
        && elf->e_type == ET_EXEC                       /* executable object? */
        && elf->e_machine == EM_MACH                    /* architecture match? */
        && elf->e_phnum > 0)                            /* has program headers? */
        ? 1 : 0;                            
}

inline static uintptr_t read_pt_entry_ptr(pt_entry ent) 
{
    return ent.physical_address << 12;
}

inline static size_t direct_table_idx(vaddr48_t idx) 
{
    return idx.page_idx + idx.pd_idx*0x200 + idx.pdp_idx*0x40000 + idx.pml4_idx*0x8000000;
}

inline static uint64_t div_roundup(uint64_t num, uint64_t denom)
{
    if (num % denom == 0) return num / denom;
    else return 1 + (num / denom);
}

inline static size_t required_tables(size_t num_pages_to_map)
{
    size_t num_page_tables = div_roundup(num_pages_to_map, PT_LEN);
    size_t num_page_dirs = div_roundup(num_page_tables, PT_LEN);
    size_t num_page_dir_tables = div_roundup(num_page_dirs, PT_LEN);
    return num_page_dir_tables + num_page_dirs + num_page_tables + 1; // Additional 1 for the PML4; each table itself fills one page of memory
}

size_t required_tables_postinit(size_t num_pages_to_map)
{
    size_t num_page_tables = num_pages_to_map / PT_LEN;
    size_t num_page_dirs = num_page_tables / PT_LEN;
    size_t num_page_dir_tables = num_page_dirs / PT_LEN;
    return num_page_dir_tables + num_page_dirs + num_page_tables;
}

void map_some_pages(uintptr_t vaddr_start, uintptr_t phys_start, size_t num_pages, paging_table tables_start)
{
    size_t total_mem = num_pages * PAGESIZE;
    size_t current_table_num = 0;
    vaddr_t current_idx = (vaddr_t)vaddr_start;
    size_t pt_num = direct_table_idx(current_idx.idx);
    size_t n = pt_num + num_pages / PT_LEN;
    uintptr_t current_phys = phys_start;
    uintptr_t end_vaddr = total_mem + vaddr_start;
    paging_table pdpt = NULL, pd = NULL, pt = NULL;
    for(size_t i = 0; i < num_pages; i++)
    {
        if(pdpt == NULL || !__boot_pml4[current_idx.idx.pml4_idx].present) 
        {
            pdpt = tables_start + current_table_num*512;
            __boot_pml4[current_idx.idx.pml4_idx].present = 1;
            __boot_pml4[current_idx.idx.pml4_idx].write = 1;
            __boot_pml4[current_idx.idx.pml4_idx].physical_address = (uintptr_t)pdpt >> 12;
            current_table_num++;
        }
        if(pd == NULL || !pdpt[current_idx.idx.pdp_idx].present)
        {
            pd = tables_start + current_table_num*512;
            pdpt[current_idx.idx.pdp_idx].present = 1;
            pdpt[current_idx.idx.pdp_idx].write = 1;
            pdpt[current_idx.idx.pdp_idx].physical_address = (uintptr_t)pd >> 12;
            current_table_num++;
        }
        if(pt == NULL || !pd[current_idx.idx.pd_idx].present)
        {
            pt = tables_start + current_table_num*512;
            pd[current_idx.idx.pd_idx].present = 1;
            pd[current_idx.idx.pd_idx].write = 1;
            pd[current_idx.idx.pd_idx].physical_address = (uintptr_t)pt >> 12;
            __boot_pagefile->head->tables[pt_num] = pt;
            pt_num++;
            current_table_num++;
        }
        pt[current_idx.idx.page_idx].present = 1;
        pt[current_idx.idx.page_idx].write = 1;
        pt[current_idx.idx.page_idx].physical_address = current_phys >> 12;
        current_phys += PAGESIZE;
        current_idx.addr += PAGESIZE;
    }
}

efi_status_t map_id_pages(size_t num_pages)
{
    size_t n = required_tables(num_pages);
    printf("map %i pages\n", n);
    // By setting the value passed in the address parameter, using AllocateMaxAddress we can control how high it's allowed to go
    paging_table tables_start;
    efi_status_t status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, n, (efi_physical_address_t*)&tables_start);
    if(EFI_ERROR(status)) 
    {
        printf("Unable to get pages for page tables: 0x%X\n", status);
        if(status == EFI_NOT_FOUND)
        {
            printf("EFI_NOT_FOUND\n");
        }
        else if(status == EFI_OUT_OF_RESOURCES)
        {
            printf("EFI_OUT_OF_RESOURCES\n");
        }
        else 
        {
            printf("EFI_INVALID_PARAMETER\n");
        }
        return status;
    }
    __boot_pml4 = tables_start;
    size_t pts_only = num_pages / PT_LEN;
    __boot_pagefile->head = (pagefile_entry*)malloc(sizeof(pagefile_entry) + pts_only * sizeof(paging_table));
    memset(__boot_pagefile->head, 0, sizeof(pagefile_entry) + pts_only * sizeof(paging_table));
    __boot_pagefile->head->num_tables = pts_only;
    __boot_pagefile->num_entries = 1;
    map_some_pages(0, 0, num_pages, tables_start + 512);
    return EFI_SUCCESS;
}

efi_status_t map_pages(uintptr_t vaddr_start, uintptr_t phys_start, size_t num_pages)
{
    size_t n = required_tables_postinit(num_pages);
    vaddr_t sv_start = {};
    sv_start.addr = vaddr_start;
    if(n < 1 && direct_table_idx(sv_start.idx) > __boot_pagefile->head->num_tables)
    {
        n = 1;
        size_t old_num = __boot_pagefile->head->num_tables;
        size_t new_num = direct_table_idx(sv_start.idx);
        __boot_pagefile->head = (pagefile_entry*)realloc(__boot_pagefile->head, sizeof(pagefile_entry) + new_num * sizeof(paging_table));
        size_t delta = (size_t)(new_num - old_num);
        memset(__boot_pagefile->head->tables + old_num, 0, delta * sizeof(paging_table));
    }
    paging_table tables_start;
    if (n == 0)
    {
        tables_start = __boot_pagefile->head->tables[direct_table_idx(sv_start.idx)];
        if(tables_start == NULL)
        {
            efi_status_t status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (efi_physical_address_t*)&tables_start);
            if(EFI_ERROR(status)) 
            {
                fprintf(stderr, "Unable to get pages for page tables\n");
                return status;
            }
        }
    }
    else 
    {
        efi_status_t status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, n, (efi_physical_address_t*)&tables_start);
        if(EFI_ERROR(status)) 
        {
            fprintf(stderr, "Unable to get pages for page tables\n");
            return status;
        }
    }
    map_some_pages(vaddr_start, phys_start, num_pages, tables_start);
    return EFI_SUCCESS;
}

#define MALLOC_CK(ptr) if(!ptr) \
{\
    fprintf(stderr, "unable to allocate memory\n");\
    return EMALLOC;\
}\

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    efi_status_t status;
    efi_memory_descriptor_t *memory_map = NULL, *mement = NULL;
    uintn_t memory_map_size = 0, map_key = 0, desc_size = 0;
    /* get the memory map */
    status = BS->GetMemoryMap(&memory_map_size, NULL, &map_key, &desc_size, NULL);
    if(status != EFI_BUFFER_TOO_SMALL || !memory_map_size)
    {
        fprintf(stderr, "Unable to get memory map\n");
        return ENOMMAP;
    }
    memory_map_size += 4 * desc_size;
    memory_map = (efi_memory_descriptor_t*)malloc(memory_map_size);
    MALLOC_CK(memory_map);
    __boot_pagefile = (pagefile*)malloc(sizeof(pagefile));
    MALLOC_CK(__boot_pagefile);
    status = map_id_pages(MMAP_MAX_PG);
    if(EFI_ERROR(status))
    {
        fprintf(stderr, "unable to allocate memory\n");
        return EMALLOC;
    }
    status = BS->GetMemoryMap(&memory_map_size, memory_map, &map_key, &desc_size, NULL);
    if(EFI_ERROR(status)) 
    {
        fprintf(stderr, "Unable to get memory map\n");
        return ENOMMAP;
    }
    size_t n = 0;
    size_t k = 0;
    mmap_t* map = (mmap_t*)malloc(sizeof(mmap_t));
    efi_memory_descriptor_t* prev = NULL;
    printf("Address              Size Type\n");
    for(mement = memory_map; n < memory_map_size; mement = NextMemoryDescriptor(mement, desc_size)) 
    {        
        printf("%016x %8d %02x %s\n", mement->PhysicalStart, mement->NumberOfPages, mement->Type, types[mement->Type]);
        if (prev != NULL && mement->Type == prev->Type && prev->PhysicalStart + prev->NumberOfPages * 4096 == mement->PhysicalStart)
        {
            map->entries[k].len += mement->NumberOfPages;
        }
        else 
        {
            map = (mmap_t*)realloc(map, sizeof(mmap_t) + (k + 1) * sizeof(mmap_entry));
            map->entries[k].addr = mement->PhysicalStart;
            map->entries[n].len = mement->NumberOfPages;
            switch(mement->Type)
            {
            case 7:
                map->entries[k].type = AVAILABLE;
                break;
            case 8:
                map->entries[k].type = BADRAM;
                break;
            case 9:
                map->entries[k].type = ACPI_RECLAIMABLE;
                break;
            case 10:
                map->entries[k].type = NVS;
                break;
            case 11:
            case 12:
                map->entries[k].type = MMIO;
                break;
            default:
                map->entries[k].type = RESERVED;
                break;
            }
            k++;
        }
        prev = mement;
        n += desc_size;
    }
    fb = (framebuf_t*)malloc(sizeof(framebuf_t));
    MALLOC_CK(fb);
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    // Setup the framebuffer for the kernel
    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    if(!EFI_ERROR(status) && gop) 
    {
        status = gop->SetMode(gop, 0);
        ST->ConOut->Reset(ST->ConOut, 0);
        ST->StdErr->Reset(ST->StdErr, 0);
        if(EFI_ERROR(status)) 
        {
            fprintf(stderr, "unable to set video mode\n");
            return ESETGFX;
        }
        fb->ptr = (uint32_t*)gop->Mode->FrameBufferBase;
        fb->width = gop->Mode->Information->HorizontalResolution;
        fb->height = gop->Mode->Information->VerticalResolution;
        fb->pitch = sizeof(unsigned int) * gop->Mode->Information->PixelsPerScanLine;
    } 
    else 
    {
        fprintf(stderr, "unable to get graphics output protocol\n");
        return ENOGFX;
    }
    status = map_pages((uintptr_t)fb->ptr, (uintptr_t) fb->ptr, (4 * fb->height * fb->width * fb->pitch) / 4096);
    FILE *f;
    char *buff;
    long int size;
    elf64_ehdr *elf;
    elf64_phdr *phdr;
    uintptr_t entry;
    int i;
    /* load the file */
    if((f = fopen(KERNEL_FILENAME, "r"))) 
    {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        buff = malloc(size + 1);
        if(!buff) 
        {
            fprintf(stderr, "unable to allocate memory\n");
            return EMALLOC;
        }
        fread(buff, size, 1, f);
        fclose(f);
    } 
    else 
    {
        fprintf(stderr, "Unable to open file\n");
        return EFOPEN;
    }
    elf = (elf64_ehdr *)buff;
    /* is it a valid ELF executable for this architecture? */
    if(validate_elf(elf))
    {                         
        /* load segments */
        for(phdr = (elf64_phdr *)(buff + elf->e_phoff), i = 0; i < elf->e_phnum; i++, phdr = (elf64_phdr *)((uint8_t *)phdr + elf->e_phentsize)) 
        {
            if(phdr->p_type == PT_LOAD) 
            {
                printf("ELF segment %p %d bytes (bss %d bytes)\n", phdr->p_vaddr, phdr->p_filesz, phdr->p_memsz - phdr->p_filesz);
                memcpy((void*)phdr->p_vaddr, buff + phdr->p_offset, phdr->p_filesz);
                memset((void*)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
            }
        }
        entry = elf->e_entry;
    } 
    else 
    {
        fprintf(stderr, "not a valid ELF executable for this architecture\n");
        return ENOELF; // must be an orc
    }
    printf("ELF entry point %p\n", entry);
    kernel_entry_fn fn = (kernel_entry_fn)entry;
     /* free resources */
    free(buff);
    /* execute the "kernel" */
    exit_bs();
    // In order to be able to map pages in the kernel, we need to disable the WP bit so that ring 0 can write to the paging tables
    asm volatile("movq %%cr0, %%rax\n" "andq %0, %%rax\n" "movq %%rax, %%cr0" :: "i"(0xFFFEFFFF) : "%rax");
    // Put the new paging tables into cr3
    asm volatile("movq %0, %%rax\n" "movq %%rax, %%cr3" :: "r"(__boot_pml4) : "%rax");
    (*fn)(fb, map, __boot_pagefile);
    while (1);
    return OK;
}