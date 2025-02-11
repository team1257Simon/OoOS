#ifndef __IDT
#define __IDT
#include "kernel/arch/arch_amd64.h"
#ifdef __cplusplus
extern "C"
{
#define CXX_INI(val) { val }
#else 
#define CXX_INI(val)
#endif
typedef struct 
{
    uint16_t    isr_low;                 // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;               // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t	    ist;                     // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t     attributes;              // Type and attributes
    uint16_t    isr_mid;                 // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t    isr_high;                // The higher 32 bits of the ISR's address
    uint32_t    reserved CXX_INI(0);     // Set to zero
} __pack idt_entry_t;
void idt_init();
#ifdef __cplusplus
}
#endif
#endif