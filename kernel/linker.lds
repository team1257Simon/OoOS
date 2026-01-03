ENTRY(_start)
KERNEL_VMA = 2M;
SECTIONS
{
	. = KERNEL_VMA;
	PROVIDE(__start = .);
	.data : ALIGN(CONSTANT(MAXPAGESIZE))
	{
		__data 					= .;
		*(.data)
		. = ALIGN(4096);
		processor_local_data 	= .;
		*(.data.plocal)
		. = ALIGN(4096);
		__plocal_data_end		= .;
	}
	.ctors :
	{
		KEEP (*crtbegin.o(.ctors))
		KEEP (*crtbegin?.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
	}
	.dtors :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*crtbegin?.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
	}
	.bss :
	{
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
	}
	.text : ALIGN(CONSTANT(MAXPAGESIZE))
	{
		*(.text.unlikely .text.*_unlikely .text.unlikely.*)
		*(.text .text.*)
	}
	.init :
	{
		PROVIDE(__code = .);
		KEEP (*(SORT_NONE(.init)))
	}
	.fini :
	{
		KEEP (*(SORT_NONE(.fini)))
	}
	PROVIDE(__code_end	= .);
	.rodata :
	{
		PROVIDE(__rodata	= .);
		*(.rodata .rodata.*)
	}
	.eh_frame_hdr :
	{
		*(.eh_frame_hdr)
		*(.eh_frame_entry .eh_frame_entry.*)
	}
	.eh_frame :
	{
		__ehframe = .;
		KEEP(*(.eh_frame))
		*(.eh_frame.*)
		QUAD(0)
	}
	.sframe :
	{
		*(.sframe)
		*(.sframe.*)
	}
	.gcc_except_table :
	{
		*(.gcc_except_table .gcc_except_table.*)
	}
	.gnu_extab : AT(ADDR(.gnu_extab))
	{
		*(.gnu_extab*)
	}
	.exception_ranges :
	{
		*(.exception_ranges*)
	}
	PROVIDE(__rodata_end		= .);
	.gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
	/DISCARD/ :
	{
		*(.comment) *(.debug*)
	}
	. 							= ALIGN(4096);
	PROVIDE(__end				= .);
	PROVIDE(__kernel_pages		= (__end - __start) / 4096);
	PROVIDE(__plocal_data_size	= __plocal_data_end - processor_local_data);
}
