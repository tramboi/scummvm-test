/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BACKENDS_ELF_H
#define BACKENDS_ELF_H

/* ELF stuff */

typedef unsigned short Elf32_Half, Elf32_Section;
typedef unsigned int Elf32_Word, Elf32_Addr, Elf32_Off;
typedef signed int  Elf32_Sword;
typedef Elf32_Half Elf32_Versym;

#define EI_NIDENT (16)
#define SELFMAG         6

/* ELF File format structures. Look up ELF structure for more details */

// ELF header (contains info about the file)
typedef struct {
	unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
	Elf32_Half    e_type;                 /* Object file type */
	Elf32_Half    e_machine;              /* Architecture */
	Elf32_Word    e_version;              /* Object file version */
	Elf32_Addr    e_entry;                /* Entry point virtual address */
	Elf32_Off     e_phoff;                /* Program header table file offset */
	Elf32_Off     e_shoff;                /* Section header table file offset */
	Elf32_Word    e_flags;                /* Processor-specific flags */
	Elf32_Half    e_ehsize;               /* ELF header size in bytes */
	Elf32_Half    e_phentsize;            /* Program header table entry size */
	Elf32_Half    e_phnum;                /* Program header table entry count */
	Elf32_Half    e_shentsize;            /* Section header table entry size */
	Elf32_Half    e_shnum;                /* Section header table entry count */
	Elf32_Half    e_shstrndx;             /* Section header string table index */
} Elf32_Ehdr;

// Should be in e_ident
#define ELFMAG          "\177ELF\1\1"	/* ELF Magic number */

// e_type values
#define ET_NONE		0	/* no file type */
#define ET_REL		1	/* relocatable */
#define ET_EXEC		2	/* executable */
#define ET_DYN		3	/* shared object */
#define ET_CORE		4	/* core file */

// e_machine values
#define EM_ARM		40

// Program header (contains info about segment)
typedef struct {
	Elf32_Word    p_type;                 /* Segment type */
	Elf32_Off     p_offset;               /* Segment file offset */
	Elf32_Addr    p_vaddr;                /* Segment virtual address */
	Elf32_Addr    p_paddr;                /* Segment physical address */
	Elf32_Word    p_filesz;               /* Segment size in file */
	Elf32_Word    p_memsz;                /* Segment size in memory */
	Elf32_Word    p_flags;                /* Segment flags */
	Elf32_Word    p_align;                /* Segment alignment */
} Elf32_Phdr;

// p_type values
#define PT_NULL 		0	/* ignored */
#define PT_LOAD			1	/* loadable segment */
#define PT_DYNAMIC		2	/* dynamic linking info */
#define PT_INTERP		3	/* info about interpreter */
#define PT_NOTE			4	/* note segment */
#define PT_SHLIB		5	/* reserved */
#define PT_PHDR			6	/* Program header table */
#define PT_ARM_ARCHEXT 	0x70000000 /* Platform architecture compatibility information */
#define PT_ARM_EXIDX 	0x70000001 /* Exception unwind tables */

// p_flags value
#define PF_X	1	/* execute */
#define PF_W	2	/* write */
#define PF_R	4	/* read */

// Section header (contains info about section)
typedef struct {
	Elf32_Word    sh_name;                /* Section name (string tbl index) */
	Elf32_Word    sh_type;                /* Section type */
	Elf32_Word    sh_flags;               /* Section flags */
	Elf32_Addr    sh_addr;                /* Section virtual addr at execution */
	Elf32_Off     sh_offset;              /* Section file offset */
	Elf32_Word    sh_size;                /* Section size in bytes */
	Elf32_Word    sh_link;                /* Link to another section */
	Elf32_Word    sh_info;                /* Additional section information */
	Elf32_Word    sh_addralign;           /* Section alignment */
	Elf32_Word    sh_entsize;             /* Entry size if section holds table */
} Elf32_Shdr;

// sh_type values
#define SHT_NULL			0	/* Inactive section */
#define SHT_PROGBITS        1	/* Proprietary */
#define SHT_SYMTAB			2	/* Symbol table */
#define SHT_STRTAB			3	/* String table */
#define SHT_RELA			4	/* Relocation entries with addend */
#define SHT_HASH			5	/* Symbol hash table */
#define SHT_DYNAMIC			6	/* Info for dynamic linking */
#define SHT_NOTE			7	/* Note section */
#define SHT_NOBITS			8	/* Occupies no space */
#define SHT_REL				9	/* Relocation entries without addend */
#define SHT_SHLIB			10	/* Reserved */
#define SHT_DYNSYM			11	/* Minimal set of dynamic linking symbols */
#define SHT_ARM_EXIDX 		0x70000001	/* Exception Index table */
#define SHT_ARM_PREEMPTMAP 	0x70000002	/* BPABI DLL dynamic linking pre-emption map */
#define SHT_ARM_ATTRIBUTES 	0x70000003	/* Object file compatibility attributes */

// sh_flags values
#define SHF_WRITE		0	/* writable section */
#define SHF_ALLOC		2	/* section occupies memory */
#define SHF_EXECINSTR	        4	/* machine instructions */

// Symbol entry (contain info about a symbol)
typedef struct {
	Elf32_Word    st_name;                /* Symbol name (string tbl index) */
	Elf32_Addr    st_value;               /* Symbol value */
	Elf32_Word    st_size;                /* Symbol size */
	unsigned char st_info;                /* Symbol type and binding */
	unsigned char st_other;               /* Symbol visibility */
	Elf32_Section st_shndx;               /* Section index */
} Elf32_Sym;

// Extract from the st_info
#define SYM_TYPE(x)		((x)&0xF)
#define SYM_BIND(x)		((x)>>4)

// Symbol binding values from st_info
#define STB_LOCAL 	0	/* Symbol not visible outside object */
#define STB_GLOBAL 	1	/* Symbol visible to all object files */
#define STB_WEAK	2	/* Similar to STB_GLOBAL */

// Symbol type values from st_info
#define STT_NOTYPE	0	/* Not specified */
#define STT_OBJECT	1	/* Data object e.g. variable */
#define STT_FUNC	2	/* Function */
#define STT_SECTION	3	/* Section */
#define STT_FILE	4	/* Source file associated with object file */

// Special section header index values from st_shndex
#define SHN_UNDEF  		0
#define SHN_LOPROC 		0xFF00	/* Extended values */
#define SHN_ABS	   		0xFFF1	/* Absolute value: don't relocate */
#define SHN_COMMON 		0xFFF2	/* Common block. Not allocated yet */
#define SHN_HIPROC 		0xFF1F
#define SHN_HIRESERVE 	0xFFFF

// Relocation entry (info about how to relocate)
typedef struct {
	Elf32_Addr    r_offset;               /* Address */
	Elf32_Word    r_info;                 /* Relocation type and symbol index */
} Elf32_Rel;

// Access macros for the relocation info
#define REL_TYPE(x)		((unsigned char) (x))	/* Extract relocation type */
#define REL_INDEX(x)	((x)>>8)		/* Extract relocation index into symbol table */

// ARM relocation types
#define R_ARM_NONE			0
#define R_ARM_ABS32			2
#define R_ARM_THM_CALL      10
#define R_ARM_V4BX 			40

#endif /* BACKENDS_ELF_H */
