ENTRY(kmain)
KERNEL_VMA = 32M;
PHDRS
{
     headers PT_PHDR PHDRS;
     /* RX = 0x05 */
     text PT_LOAD FILEHDR PHDRS FLAGS(0x5);
     data PT_LOAD;
}
SECTIONS
{
     . = KERNEL_VMA;
     __start = .;

     .data : AT(ADDR(.data)) 
     {
          __data = .;
          *(.data)
     } :data

     .bss : AT(ADDR(.bss))
     {
          . = ALIGN(4096);
          __bss = .;
          *(COMMON)
     }
     
     
     .ctors : AT(ADDR(.ctors))
     {
           __CTOR_LIST__ = .;
          QUAD((__CTOR_END__ - __CTOR_LIST__) / 8 - 2)
          *(.ctors)
          QUAD(0)
          __CTOR_END__ = .;
     }
     
     .dtors : AT(ADDR(.dtors))
     {
          __DTOR_LIST__ = .;
          QUAD((__DTOR_END__ - __DTOR_LIST__) / 8 - 2)
          *(.dtors)
          QUAD(0)
          __DTOR_END__ = .;
     }

     .text (READONLY) : AT(ADDR(.text)) 
     {
          __code = .;
          *(.text .text.*)
          *(.rodata*)
     } :text

     .eh_frame_hdr : AT(ADDR(.eh_frame_hdr))
     {
          *(.eh_frame_hdr) 
          *(.eh_frame_entry .eh_frame_entry.*)
     }

     .eh_frame : AT(ADDR(.eh_frame))
     {
          . = ALIGN(4);   
          __ehframe = .;
          *(.eh_frame)
          *(.eh_frame.*)
          QUAD(0)
     }

     .sframe : AT(ADDR(.sframe))
     {
           *(.sframe) 
           *(.sframe.*) 
     }

     .gcc_except_table : AT(ADDR(.gcc_except_table))
     {
          *(.gcc_except_table .gcc_except_table.*) 
     }

     .gnu_extab : AT(ADDR(.gnu_extab))
     {
          *(.gnu_extab*)
     }

     /DISCARD/ :
     {
          *(.comment)
     }
     . = ALIGN(4096);
     __end = .;
}
