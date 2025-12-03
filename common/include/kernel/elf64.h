#ifndef __ELF64
#define __ELF64
#ifndef _GCC_STDINT_H
#include <stdint.h>
#endif
#include <stddef.h>
#define HAVE_SIZE_T 1
#define HAVE_STDINT 1
#ifdef __cplusplus
#include <sys/types.h>
#ifndef __may_alias
#define __may_alias [[gnu::may_alias]]
#endif
enum elf_class : uint8_t
{
	EC_NONE		= 0,
	EC_32		= 1,
	EC_64		= 2
};
enum elf_data_encoding : uint8_t
{
	ED_NONE		= 0,
	ED_LSB		= 1,
	ED_MSB		= 2
};
enum elf_machine_type : uint16_t
{
	EM_NONE		= 0,
	EM_M32		= 1,
	EM_SPARC	= 2,
	EM_I386		= 3,
	EM_68K		= 4,
	EM_88K		= 5,
	EM_I860		= 7,
	EM_MIPS		= 8,
	EM_PPC		= 0x14,
	EM_ARM		= 0x28,
	EM_SUPERH	= 0x2A,
	EM_ITANIUM	= 0x32,
	EM_AMD64	= 0x3E,
	EM_AARCH64	= 0xB7,
	EM_RISCV	= 0xF3
};
enum elf_object_type : uint16_t
{
	ET_NONE		= 0,
	ET_REL		= 1,
	ET_EXEC		= 2,
	ET_DYN		= 3,
	ET_CORE		= 4
	// PC-Specific = 0xFF00~0xFFFF
};
enum elf_segment_type : uint32_t
{
	PT_NULL		= 0U,
	PT_LOAD		= 1U,
	PT_DYNAMIC	= 2U,
	PT_INTERP	= 3U,
	PT_NOTE		= 4U,
	PT_SHLIB	= 5U,
	PT_PHDR		= 6U,
	PT_TLS		= 7U,
	// PC-Specific = 0x70000000~0x7FFFFFFF
};
enum elf_section_type : uint32_t
{
	SHT_NULL			= 0U,
	SHT_PROGBITS		= 1U,
	SHT_SYMTAB			= 2U,
	SHT_STRTAB			= 3U,
	SHT_RELA			= 4U,
	SHT_HASH			= 5U,
	SHT_DYNAMIC			= 6U,
	SHT_NOTE			= 7U,
	SHT_NOBITS			= 8U,
	SHT_REL				= 9U,
	SHT_SHLIB			= 10U,
	SHT_DYNSYM			= 11U,
	// OS-Specific		= 0x60000000~0x6FFFFFFF
	SHT_GNU_HASH		= 0x6FFFFFF6U,
	SHT_VERDEF			= 0x6FFFFFFDU,
	SHT_VERNEED			= 0x6FFFFFFEU,
	SHT_VERSYM			= 0xFFFFFFFFU,
	// PC-Specific		= 0x70000000~0x7FFFFFFF
	SHT_AMD64_UNWIND	= 0x70000001U
};
enum elf_sym_type : uint8_t
{
	ST_NOTYPE		= 0UC,
	ST_OBJECT		= 1UC,
	ST_FUNCTION		= 2UC,
	ST_SECTION		= 3UC,
	ST_FILE			= 4UC,
	ST_TLS			= 6UC,
};
enum elf_sym_bind : uint8_t
{
	SB_LOCAL	= 0UC,
	SB_GLOBAL	= 1UC,
	SB_WEAK		= 2UC
};
enum elf_special_section : uint16_t
{
	SHN_UNDEF	= 0US,		// undefined symbol
	SHN_ABS		= 0xFFF1US,	// absolute value
	SHN_COMMON	= 0xFFF2US,	// common block
};
enum elf_dyn_tag : uint64_t
{
	// NAME				VALUE			//		d_un		executable	shared
	DT_NULL				= 0UL,			//		N/A			req			req
	DT_NEEDED			= 1UL,			//		d_val		opt			opt
	DT_PLTRELSZ			= 2UL,			//		d_val		opt			opt
	DT_PLTGOT			= 3UL,			//		d_ptr		opt			opt
	DT_HASH				= 4UL,			//		d_ptr		req			req
	DT_STRTAB			= 5UL,			//		d_ptr		req			req
	DT_SYMTAB			= 6UL,			//		d_ptr		req			req
	DT_RELA				= 7UL,			//		d_ptr		req			opt
	DT_RELASZ			= 8UL,			//		d_val		req			opt
	DT_RELAENT			= 9UL,			//		d_val		req			opt
	DT_STRSZ			= 10UL,			//		d_val		req			req
	DT_SYMENT			= 11UL,			//		d_val		req			req
	DT_INIT				= 12UL,			//		d_ptr		opt			opt
	DT_FINI				= 13UL,			//		d_ptr		opt			opt
	DT_SONAME			= 14UL,			//		d_val		N/A			opt
	DT_RPATH			= 15UL,			//		d_val		opt			N/A
	DT_SYMBOLIC			= 16UL,			//		N/A			N/A			opt
	DT_REL				= 17UL,			//		d_ptr		req			opt
	DT_RELSZ			= 18UL,			//		d_val		req			opt
	DT_RELENT			= 19UL,			//		d_val		req			opt
	DT_PLTREL			= 20UL,			//		d_val		opt			opt
	DT_DEBUG			= 21UL,			//		d_ptr		opt			N/A
	DT_TEXTREL			= 22UL,			//		N/A			opt			opt
	DT_JMPREL			= 23UL,			//		d_ptr		opt			opt
	DT_BIND_NOW			= 24UL,			//		N/A			opt			opt
	DT_INIT_ARRAY		= 25UL,			//		d_ptr		opt			opt
	DT_FINI_ARRAY		= 26UL,			//		d_ptr		opt			opt
	DT_INIT_ARRAYSZ		= 27UL,			//		d_val		opt			opt
	DT_FINI_ARRAYSZ		= 28UL,			//		d_val		opt			opt
	DT_RUNPATH			= 29UL,			//		d_val		opt			opt
	DT_FLAGS			= 30UL,			//		d_val		opt			opt
	DT_ENCODING			= 31UL,			//		?			?			?
	DT_PREINIT_ARRAY	= 32UL,			//		d_ptr		opt			N/A
	DT_PREINIT_ARRAYSZ	= 33UL,			//		d_val		opt			N/A
	// OS 0x6000000D~0x6FFFFFFF			//		d_un		executable	shared
	DT_GNU_HASH			= 0x6FFFFEF5UL,	//		d_ptr		req			req
	DT_VERSYM			= 0x6FFFFFF0UL,	//		?			?			?
	DT_FLAGS_1			= 0x6FFFFFFBUL,	//		d_val		opt			opt
	DT_VERDEF			= 0x6FFFFFFCUL,	//		?			?			?
	DT_VERDEFNUM		= 0x6FFFFFFDUL,	//		?			?			?
	DT_VERNEED			= 0x6FFFFFFEUL,	//		?			?			?
	DT_VERNEEDNUM		= 0x6FFFFFFFUL,	//		?			?			?
	// PC 0x7000000D~0x7FFFFFFF			//		d_un		executable	shared
	// ...
};
enum elf_dyn_flags : uint64_t
{
	DF_ORIGIN		= 0x0001,		// Indicates that the object being loaded may make reference to the $ORIGIN substitution string
	DF_SYMBOLIC		= 0x0002,		// Indicates "symbolic" linking.
	DF_TEXTREL		= 0x0004,		// Indicates there may be relocations in non-writable segments.
	DF_BIND_NOW		= 0x0008,		// Indicates that the dynamic linker should process all relocations for the object containing this entry before transferring control to the program.
	DF_STATIC_TLS	= 0x0010,		// Indicates that the shared object or executable contains code using a static thread-local storage scheme.
};
enum elf_dyn_flags_1 : uint64_t
{
	DF_1_NOW		= 0x00000001,		// Indicates that all relocations for this object must be processed before returning control to the program.
	DF_1_GLOBAL		= 0x00000002,		// Unused.
	DF_1_GROUP		= 0x00000004,		// Indicates that the object is a member of a group.
	DF_1_NODELETE	= 0x00000008,		// Indicates that the object cannot be deleted from a process.
	DF_1_LOADFLTR	= 0x00000010,		// Meaningful only for filters. Indicates that all associated filtees be processed immediately.
	DF_1_INITFIRST	= 0x00000020,		// Indicates that this object's initialization section be run before any other objects loaded.
	DF_1_NOOPEN		= 0x00000040,		// Indicates that the object cannot be added to a running process with dlopen.
	DF_1_ORIGIN		= 0x00000080,		// Indicates the object requires $ORIGIN processing.
	DF_1_DIRECT		= 0x00000100,		// Indicates that the object should use direct binding information.
	DF_1_TRANS		= 0x00000200,		// Unused.
	DF_1_INTERPOSE	= 0x00000400,		// Indicates that the objects symbol table is to interpose before all symbols except the primary load object, which is typically the executable.
	DF_1_NODEFLIB	= 0x00000800,		// Indicates that the search for dependencies of this object ignores any default library search paths.
	DF_1_NODUMP		= 0x00001000,		// Indicates that this object is not dumped by dldump. Candidates are objects with no relocations that might get included when generating alternative objects using.
	DF_1_CONFALT	= 0x00002000,		// Identifies this object as a configuration alternative object generated by crle. Triggers the runtime linker to search for a configuration file $ORIGIN/ld.config.app-name.
	DF_1_ENDFILTEE	= 0x00004000,		// Meaningful only for filtees. Terminates a filters search for any further filtees.
	DF_1_DISPRELDNE = 0x00008000,		// Indicates that this object has displacement relocations applied.
	DF_1_DISPRELPND = 0x00010000,		// Indicates that this object has displacement relocations pending.
	DF_1_NODIRECT	= 0x00020000,		// Indicates that this object contains symbols that cannot be directly bound to.
	DF_1_IGNMULDEF	= 0x00040000,		// Reserved for internal use by the kernel runtime-linker.
	DF_1_NOKSYMS	= 0x00080000,		// Reserved for internal use by the kernel runtime-linker.
	DF_1_NOHDR		= 0x00100000,		// Reserved for internal use by the kernel runtime-linker.
	DF_1_EDITED		= 0x00200000,		// Indicates that this object has been edited or has been modified since the objects original construction by the link-editor.
	DF_1_NORELOC	= 0x00400000,		// Reserved for internal use by the kernel runtime-linker.
	DF_1_SYMINTPOSE	= 0x00800000,		// Indicates that the object contains individual symbols that should interpose before all symbols except the primary load object, which is typically the executable.
	DF_1_GLOBAUDIT	= 0x01000000,		// Indicates that the executable requires global auditing.
	DF_1_SINGLETON	= 0x02000000,		// Indicates that the object defines, or makes reference to singleton symbols.
	DF_1_STUB		= 0x04000000,		// Indicates that the object is a stub.
	DF_1_PIE		= 0x08000000,		// Indicates that the object is a position-independent executable.
	DF_1_KMOD		= 0x10000000,		// Indicates that the object is a kernel module.
	DF_1_WEAKFILTER	= 0x20000000,		// Indicates that the object is a weak standard filter.
	DF_1_NOCOMMON	= 0x40000000,		// Unused.
};
enum elf_rel_type : uint32_t
{
	// A:		Represents the addend used to compute the value of the relocatable field.
	// B:		Represents the base address at which a shared object has been loaded into memory during execution. Generally, a shared object is built with a 0 base virtual address, but the execution address will be different.
	// O:		Represents the offset into the global offset table at which the relocation entry’s symbol will reside during execution.
	// G:		Represents the address of the global offset table.
	// L:		Represents the place (section offset or address) of the Procedure Linkage Table entry for a symbol.
	// P:		Represents the place (section offset or address) of the storage unit being relocated (computed using r_offset).
	// S:		Represents the value of the symbol whose index resides in the relocation entry.
	// M:		Represents the DTV index of the module
	// T:		Represents the TLS block containing the symbol
	// Q:		Represents the initial TLS block
	// Z:		Represents the size of the symbol
	// I(...):	Represents a program-counter-relative offset
	// [...]:	Represents a table entry containing a pointer to something
	// Name 				Value	Field 	Calculation
	R_X86_64_NONE		=	0,	// 	none 	none
	R_X86_64_64			=	1,	// 	word64 	S + A
	R_X86_64_PC32		=	2,	// 	word32 	S + A - P
	R_X86_64_GOT32		=	3,	// 	word32 	O + A
	R_X86_64_PLT32		=	4,	// 	word32	L + A - P
	R_X86_64_COPY		=	5,	// 	none	none
	R_X86_64_GLOB_DAT	=	6,	// 	word64 	S
	R_X86_64_JUMP_SLOT	=	7,	// 	word64 	S
	R_X86_64_RELATIVE	=	8,	// 	word64 	B + A
	R_X86_64_GOTPCREL	=	9,	// 	word32 	O + G + A - P
	R_X86_64_32			=	10, // 	word32 	S + A
	R_X86_64_32S		=	11, // 	word32 	S + A
	R_X86_64_16			=	12, // 	word16	S + A
	R_X86_64_PC16		=	13, // 	word16 	S + A - P
	R_X86_64_8			=	14, // 	word8 	S + A
	R_X86_64_PC8		=	15, // 	word8 	S + A - P
	R_X86_64_DPTMOD64	=	16, // 	word64	M
	R_X86_64_DTPOFF64	=	17, // 	word64	S - T
	R_X86_64_TPOFF64	=	18, // 	word64	S - Q
	R_X86_64_TLSGD		=	19, // 	word32	I(G[S - T])
	R_X86_64_TLSLD		=	20, // 	word32	I(G[T])
	R_X86_64_DTPOFF32	=	21, // 	word32	S - T
	R_X86_64_GOTTPOFF	=	22, // 	word32	I(G[S - Q])
	R_X86_64_TPOFF32	=	23, // 	word32	S - Q
	R_X86_64_PC64		=	24, // 	word64 	S + A - P
	R_X86_64_GOTOFF64	=	25, // 	word64 	S + A - T
	R_X86_64_GOTPC32	=	26, // 	word32 	G + A - P
	R_X86_64_GOT64 		= 	27, //	word64 	O + A
	R_X86_64_GOTPCREL64 = 	28, //	word64 	O + T - P + A
	R_X86_64_GOTPC64 	= 	29, //	word64 	G - P + A
	R_X86_64_GOTPLT64 	=	30, //	word64 	O + A
	R_X86_64_PLTOFF64 	= 	31, //	word64 	L - T + A
	R_X86_64_SIZE32		=	32, //	word32	Z + A
	R_X86_64_SIZE64		=	33, //	word64	Z + A
	// Special dynamic relocations...
	R_X86_64_GOTPC32_TLSDESC	= 34,
	R_X86_64_TLSDESC_CALL		= 35,
	R_X86_64_TLSDESC			= 36,
	R_X86_64_IRELATIVE			= 37
};
constexpr unsigned elf_ident_class_idx	= 4U;
constexpr unsigned elf_ident_enc_idx	= 5U;
enum elf_ver_flag
{
	VER_FLG_NONE	= 0x0US,
	VER_FLG_BASE	= 0x1US,
	VER_FLG_WEAK	= 0x2US,
};
enum elf_segment_prot : uint8_t
{
	PV_NONE			= 0b000U,
	PV_READ			= 0b100U,
	PV_WRITE		= 0b010U,
	PV_EXEC			= 0b001U,
	PV_READ_EXEC	= 0b101U,
	PV_READ_WRITE	= 0b110U,
	PV_RWX			= 0b111U
};
constexpr bool is_write(elf_segment_prot p) { return (p & PV_WRITE) != 0; }
constexpr bool is_exec(elf_segment_prot p) { return (p & PV_EXEC) != 0; }
#else
typedef unsigned char elf_sym_bind;
typedef unsigned char elf_sym_type;
typedef unsigned int elf_rel_type;
#ifndef __may_alias
#define __may_alias
#endif
#endif
typedef struct
{
	uint8_t				e_ident[16];		/* Magic number and other info */
	uint16_t			e_type;				/* Object file type */
	uint16_t			e_machine;			/* Architecture */
	uint32_t			e_version;			/* Object file version */
	uint64_t			e_entry;			/* Entry point virtual address */
	uint64_t			e_phoff;			/* Program header table file offset */
	uint64_t			e_shoff;			/* Section header table file offset */
	uint32_t			e_flags;			/* Processor-specific flags */
	uint16_t			e_ehsize;			/* ELF header size in bytes */
	uint16_t			e_phentsize;		/* Program header table entry size */
	uint16_t			e_phnum;			/* Program header table entry count */
	uint16_t			e_shentsize;		/* Section header table entry size */
	uint16_t			e_shnum;			/* Section header table entry count */
	uint16_t			e_shstrndx;			/* Section header string table index */
} elf64_ehdr;
typedef struct
{
	uint32_t			p_type;				/* Segment type */
	uint32_t			p_flags;			/* Segment flags */
	uint64_t			p_offset;			/* Segment file offset */
	uint64_t			p_vaddr;			/* Segment virtual address */
	uint64_t			p_paddr;			/* Segment physical address */
	uint64_t			p_filesz;			/* Segment size in file */
	uint64_t			p_memsz;			/* Segment size in memory */
	uint64_t			p_align;			/* Segment alignment */
} elf64_phdr;
typedef struct
{
	uint32_t	sh_name;		/* Section name (string tbl index) */
	uint32_t	sh_type;		/* Section type */
	uint64_t	sh_flags;		/* Section flags */
	uint64_t	sh_addr;		/* Section virtual addr at execution */
	uint64_t	sh_offset;		/* Section file offset */
	uint64_t	sh_size;		/* Section size in bytes */
	uint32_t	sh_link;		/* Link to another section */
	uint32_t	sh_info;		/* Additional section information */
	uint64_t	sh_addralign;	/* Section alignment */
	uint64_t	sh_entsize;		/* Entry size if section holds table */
} elf64_shdr;
typedef struct
{
	uint32_t	st_name;				/* Symbol name (string tbl index) */
	struct
	{
		elf_sym_type type : 4;			/* Symbol type (object, function, etc) */
		elf_sym_bind bind : 4;			/* Symbol binding (local, global, or weak) */
	} __pack __align(1) st_info;
	uint8_t		st_other;				/* Symbol visibility */
	uint16_t	st_shndx;				/* Section index */
	uint64_t	st_value;				/* Symbol value */
	uint64_t	st_size;				/* Symbol size */
} elf64_sym;
typedef struct {
	elf_rel_type type;
	uint32_t sym_index;
} elf64_rel_info;
typedef struct {
	uintptr_t			r_offset;
	elf64_rel_info		r_info;
} elf64_rel;
typedef struct
{
	uintptr_t		r_offset;
	elf64_rel_info	r_info;
	int64_t			r_addend;
} elf64_rela;
typedef struct
{
	int64_t			d_tag;
	union __may_alias {
		uint64_t	d_val;
		uintptr_t	d_ptr;
	};
} elf64_dyn;
typedef struct
{
	uint16_t	vd_version;		//	Version revision. This field shall be set to 1.
	uint16_t	vd_flags;		//	Version information flag bitmask.
	uint16_t	vd_ndx;			//	Version index numeric value referencing the SHT_VERSYM section.
	uint16_t	vd_cnt;			//	Number of associated verdaux array entries.
	uint32_t	vd_hash;		//	Version name hash value (ELF hash function).
	uint32_t	vd_aux;			//	Offset in bytes to a corresponding entry in an array of elf64_verdaux structures.
	uint32_t	vd_next;		//	Offset to the next verdef entry, in bytes.
} elf64_verdef;
typedef struct {
	uint32_t		vda_name;	//	Offset to the version or dependency name string in the string table linked in the section header, in bytes.
	uint32_t		vda_next;	//	Offset to the next verdaux entry, in bytes.
} elf64_verdaux;
typedef struct
{
	uint16_t	vn_version;		//	Version of structure. This value is currently set to 1, and will be reset if the versioning implementation is incompatibly altered.
	uint16_t	vn_cnt;			//	Number of associated vernaux entries
	uint32_t	vn_file;		//	Offset to the file name string in the string table linked in the section header, in bytes.
	uint32_t	vn_aux;			//	Offset to a corresponding vernaux entry, in bytes.
	uint32_t	vn_next;		//	Offset to the next verneed entry, in bytes.
} elf64_verneed;
typedef struct
{
	uint32_t	vna_hash;		//	Dependency name hash value (ELF hash function).
	uint16_t	vna_flags;		//	Dependency information flag bitmask.
	uint16_t	vna_other;		//	Object file version identifier used in the .gnu.version symbol version array. Bit number 15 controls whether or not the object is hidden; if this bit is set, the object cannot be used and the static linker will ignore the symbol's presence in the object.
	uint32_t	vna_name;		//	Offset to the dependency name string in the string table linked in the section header, in bytes.
	uint32_t	vna_next;		//	Offset to the next vernaux entry, in bytes.
} elf64_vernaux;
// Contains the important details for loading an elf executable as a task.
typedef struct
{
	void* frame_ptr;
	void* prg_stack;
	size_t stack_size;
	void* prg_tls;
	size_t tls_size;
	size_t tls_align;
	void* entry;
	const char** ld_path;
	size_t ld_path_count;
	void* object_handle;
} elf64_program_descriptor;
typedef struct {
	uint64_t ti_module;
	uint64_t ti_offset;
} tls_index;
#ifdef __cplusplus
constexpr bool is_write(elf64_phdr const& seg) { return seg.p_flags & PV_WRITE; }
constexpr bool is_exec(elf64_phdr const& seg) { return seg.p_flags & PV_EXEC; }
constexpr bool is_tls(elf64_phdr const& seg) { return seg.p_type == PT_TLS; }
constexpr bool is_load(elf64_phdr const& seg) { return seg.p_type == PT_LOAD || is_tls(seg); }
constexpr bool is_dynamic(elf64_phdr const& seg) { return seg.p_type == PT_DYNAMIC; }
struct elf64_dlmap_entry
{
	uintptr_t vaddr_offset;
	char* absolute_pathname;
	elf64_dyn* dynamic_section;
	elf64_dlmap_entry* next;
	elf64_dlmap_entry* prev;
	size_t dynamic_section_length;
	void* object_handle;
	addr_t global_offset_table_start;
};
struct program_segment_descriptor
{
	addr_t absolute_addr;		// The global, physical address of the segment's start. If the segment is not loaded (e.g. not a loadable segment) this will be zero.
	addr_t virtual_addr;		// The p_vaddr value from the segment's program header modified based on where the segment was actually loaded if applicable.
	off_t obj_offset;			// The p_offset value from the segment's program header in the object file containing the segment's data.
	size_t size;				// The p_memsz from the segment's program header in the object file containing the segment's data.
	size_t seg_align;			// The p_align value from the segment's program header in the object file containing the segment's data.
	elf_segment_prot perms;		// The permission values as determined from the program header's flags (the three lowest bits only)
};
constexpr addr_t resegment_ptr(addr_t addr, program_segment_descriptor const& oseg, program_segment_descriptor const& nseg) { return nseg.absolute_addr.plus(addr - oseg.absolute_addr); }
constexpr addr_t to_segment_ptr(uint64_t off, program_segment_descriptor const& seg) { return seg.absolute_addr.plus(off - seg.obj_offset); }
#endif
#endif