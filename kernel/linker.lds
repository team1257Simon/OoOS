ENTRY(_start)
KERNEL_VMA = 2M;
PHDRS
{
	headers PT_PHDR PHDRS;
	text PT_LOAD FILEHDR PHDRS FLAGS(0x5);
	data PT_LOAD;
}
SECTIONS
{
	. = KERNEL_VMA;
	__start = .;
	.data : AT(ADDR(.data))
	{
		__data 					= .;
		*(.data)
		. = ALIGN(4096);
		processor_local_data 	= .;
		*(.data.plocal)
		. = ALIGN(4096);
		__plocal_data_end		= .;
	} :data
	.bss : AT(ADDR(.bss))
	{
		. = ALIGN(4096);
		__bss = .;
		*(COMMON) *(.bss) *(.bss.*)
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
	.init (READONLY) : AT(ADDR(.init))
	{
		KEEP (*(SORT_NONE(.init)))
	} :text
	.text (READONLY) : AT(ADDR(.text))
	{
		__code = .;
		*(.text .text.*)
		*(.rodata*)
	} :text
	.fini (READONLY) : AT(ADDR(.fini))
	{
		KEEP (*(SORT_NONE(.fini)))
	} :text
	.eh_frame_hdr : AT(ADDR(.eh_frame_hdr))
	{
		*(.eh_frame_hdr)
		*(.eh_frame_entry .eh_frame_entry.*)
	}
	. = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
	.eh_frame : AT(ADDR(.eh_frame))
	{
		__ehframe = .;
		KEEP(*(.eh_frame))
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
	.exception_ranges   : AT(ADDR(.exception_ranges))
	{
		*(.exception_ranges*)
	}
	/DISCARD/ :
	{
		*(.comment)
	}
	. 					= ALIGN(4096);
	__end				= .;
	__kernel_pages		= (__end - __start) / 4096;
	__plocal_data_size	= __plocal_data_end - processor_local_data;
}
