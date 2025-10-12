/*
 * Behaves similarly to the ldscript for shared libraries but omits some unnecessary bits.
*/
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
SEARCH_DIR("=/lib"); SEARCH_DIR("=/usr/lib");
ENTRY(_start)
SECTIONS
{
    PROVIDE (__executable_start = SEGMENT_START("text-segment", 0)); . = SEGMENT_START("text-segment", 0) + SIZEOF_HEADERS;
    .hash           : { *(.hash) }
    .gnu.hash       : { *(.gnu.hash) }
    .dynsym         : { *(.dynsym) }
    .dynstr         : { *(.dynstr) }
    .rela.dyn       :
    {
        *(.rela.init)
        *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
        *(.rela.fini)
        *(.rela.data.rel.ro .rela.data.rel.ro.* .rela.data.rel.ro.local.* .rela.gnu.linkonce.d.rel.ro.*) 
        *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
        *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
        *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
        *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
        *(.rela.ctors)
        *(.rela.dtors)
        *(.rela.got)
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
        *(.rela.ldata .rela.ldata.* .rela.gnu.linkonce.l.*)
        *(.rela.lbss .rela.lbss.* .rela.gnu.linkonce.lb.*)
        *(.rela.lrodata .rela.lrodata.* .rela.gnu.linkonce.lr.*)
        *(.rela.ifunc)
    }
    .rela.plt       :
    {
      *(.rela.plt)
      PROVIDE_HIDDEN (__rela_iplt_start = .);
      *(.rela.iplt)
      PROVIDE_HIDDEN (__rela_iplt_end = .);
    }
    .init           :
    {
        KEEP (*(SORT_NONE(.init)))
    }
    .plt            : { *(.plt) *(.iplt) }
    .plt.got        : { *(.plt.got) }
    .plt.sec        : { *(.plt.sec) }
    .text           :
    {
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
        *(.text.exit .text.exit.*)
        *(.text.startup .text.startup.*)
        *(.text.hot .text.hot.*)
        *(SORT(.text.sorted.*))
        *(.text .stub .text.* .gnu.linkonce.t.*)
        /* .gnu.warning sections are handled specially by elf.em.  */
        *(.gnu.warning)
    }
    .fini           :
    {
        KEEP (*(SORT_NONE(.fini)))
    }
    PROVIDE (__etext = .);
    PROVIDE (_etext = .);
    PROVIDE (etext = .);
    .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
    .rodata1        : { *(.rodata1) }
    .eh_frame_hdr   : { *(.eh_frame_hdr) *(.eh_frame_entry .eh_frame_entry.*) }
    .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) *(.eh_frame.*) }
    .sframe         : ONLY_IF_RO { *(.sframe) *(.sframe.*) }
    .gcc_except_table   : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }
    .gnu_extab   : ONLY_IF_RO { *(.gnu_extab*) }
    .exception_ranges   : ONLY_IF_RO { *(.exception_ranges*) }
    . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
    /* Exception handling  */
    .eh_frame       : ONLY_IF_RW { KEEP (*(.eh_frame)) *(.eh_frame.*) }
    .sframe         : ONLY_IF_RW { *(.sframe) *(.sframe.*) }
    .gnu_extab      : ONLY_IF_RW { *(.gnu_extab) }
    .gcc_except_table   : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }
    .exception_ranges   : ONLY_IF_RW { *(.exception_ranges*) }
    /* Thread Local Storage sections  */
    .tdata	  :
    {
        *(.tdata .tdata.* .gnu.linkonce.td.*)
    }
    .tbss		  : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
    .preinit_array    :
    {
        KEEP (*(.preinit_array))
    }
    .init_array    :
    {
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    }
    .fini_array    :
    {
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    }
    .ctors          :
    {
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    }
    .dtors          :
    {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
    }
    .jcr            : { KEEP (*(.jcr)) }
    .data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*) }
    .dynamic        : { *(.dynamic) }
    .got            : { *(.got) *(.igot) }
    . = DATA_SEGMENT_RELRO_END (SIZEOF (.got.plt) >= 24 ? 24 : 0, .);
    .got.plt        : { *(.got.plt) *(.igot.plt) }
    .data           :
    {
        *(.data .data.* .gnu.linkonce.d.*)
        SORT(CONSTRUCTORS)
    }
    .data1          : { *(.data1) }
    PROVIDE (_edata = .); PROVIDE (edata = .);
    . = ALIGN(ALIGNOF(NEXT_SECTION));
    PROVIDE (__bss_start = .);
    .bss            :
    {
        *(.dynbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(. != 0 ? 64 / 8 : 1);
    }
    .lbss   :
    {
        *(.dynlbss)
        *(.lbss .lbss.* .gnu.linkonce.lb.*)
        *(LARGE_COMMON)
    }
    . = ALIGN(64 / 8);
    . = SEGMENT_START("ldata-segment", .);
    .lrodata   ALIGN(CONSTANT (MAXPAGESIZE)) + (. & (CONSTANT (MAXPAGESIZE) - 1)) :
    {
        *(.lrodata .lrodata.* .gnu.linkonce.lr.*)
    }
    .ldata   ALIGN(CONSTANT (MAXPAGESIZE)) + (. & (CONSTANT (MAXPAGESIZE) - 1)) :
    {
        *(.ldata .ldata.* .gnu.linkonce.l.*)
        . = ALIGN(. != 0 ? 64 / 8 : 1);
    }
    . = ALIGN(64 / 8);
    PROVIDE (_end = .); PROVIDE (end = .);
    . = DATA_SEGMENT_END (.);
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment 0 (INFO) : { *(.comment); LINKER_VERSION; }
    .gnu.build.attributes : { *(.gnu.build.attributes .gnu.build.attributes.*) }
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions.  */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2.  */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2.  */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions.  */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
    /* DWARF 3.  */
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    .debug_ranges   0 : { *(.debug_ranges) }
    /* DWARF 5.  */
    .debug_addr     0 : { *(.debug_addr) }
    .debug_line_str 0 : { *(.debug_line_str) }
    .debug_loclists 0 : { *(.debug_loclists) }
    .debug_macro    0 : { *(.debug_macro) }
    .debug_names    0 : { *(.debug_names) }
    .debug_rnglists 0 : { *(.debug_rnglists) }
    .debug_str_offsets 0 : { *(.debug_str_offsets) }
    .debug_sup      0 : { *(.debug_sup) }
    .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
    /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) *(.interp) }
}