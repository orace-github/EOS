/*
 * Copyright 2019 Orace KPAKPO </ orace.kpakpo@yahoo.fr >
 *
 * This file is part of EOS.
 *
 * EOS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __ELF_H__
#define __ELF_H__

#include <types.h>

#define ELF_IDENT	16
#define ET_NONE		0x0 // No file type
#define ET_REL		0x1 // Relocatable file
#define ET_EXEC		0x2 // Executable file
#define ET_DYN		0x3 // Shared Object file
#define ET_CORE		0x4 // Core file
#define ET_LOPROC	0xff00 // Processor specific
#define ET_HIPROC	0xffff // Processor specific

#define EM_NONE		0x0 // No machine
#define EM_M32		0x1 // AT&T WE  32100
#define EM_SPARC	0x2 // SPARC
#define EM_386		0x3 // Intel 80386
#define EM_68K		0x4 // Motorola 68000
#define EM_88K		0x5 // Motorola 88000
#define EM_860		0x7 // Intel 80860
#define EM_MIPS		0x8 // MIPS RS3000

#define EV_NONE		0x0 // Invalid version
#define EV_CURRENT	0x1 // Current version

#define EI_MAG0		0x0 // file identification
#define EI_MAG1		0x1 // file identification
#define EI_MAG2		0x2 // file identification
#define EI_MAG3		0x3 // file identification
#define EI_CLASS	0x4 // file class
#define EI_DATA		0x5 // file data encoding
#define EI_VERSION	0x6 // file version
#define EI_PAD		0x7 // padding offset

#define ELFCLASSNONE	0x0 // Invalid class
#define ELFCLASS32		0x1 // 32-bit objects
#define ELFCLASS64		0x2 // 64-bit objects

#define ELFDATANONE		0x0 // Invalid data encoding
#define ELFDATA2LSB		0x1 // LSB encoding
#define ELFDATA2MSB		0x2 // MSB encoding

#define PT_NULL 		0x0 // Invalid program header
#define PT_LOAD			0x1 // Loadable program header
#define PT_DYNAMIC		0x2 // 
#define PT_INTERP		0x3 // 
#define PT_NOTE			0x4 //
#define PT_SHLIB		0x5 //
#define PT_PHDR			0x6 //
#define PT_LOPROC		0x70000000 //
#define PT_HIPROC		0x7fffffff //

#define PF_X	0x1 // exec flag
#define PF_W	0x2 // write flag
#define PF_R	0x4 // read flag


typedef struct
{
	u8_t e_ident[ELF_IDENT];
	u16_t e_type;
	u16_t e_machine;
	u32_t e_version;
	u32_t e_entry;
	u32_t e_phoff;
	u32_t e_shoff;
	u32_t e_flags;
	u16_t e_ehsize;
	u16_t e_phentsize;
	u16_t e_phnum;
	u16_t e_shentsize;
	u16_t e_shnum;
	u16_t e_shstrndx;
} __attribute__((packed)) elf_header_t;

typedef struct
{
	u32_t sh_name;
	u32_t sh_type;
	u32_t sh_flags;
	u32_t sh_addr;
	u32_t sh_offset;
	u32_t sh_size;
	u32_t sh_link;
	u32_t sh_info;
	u32_t sh_addralign;
	u32_t sh_entsize;
} __attribute__((packed)) elf_section_header_t;

typedef struct
{
	u32_t p_type;
	u32_t p_offset;
	u32_t p_vaddr;
	u32_t p_paddr;
	u32_t p_filesz;
	u32_t p_memsz;
	u32_t p_flags;
	u32_t p_align;
} __attribute__((packed)) elf_program_header_t;

static inline int is_elf_x86(u8_t* __buf){
	elf_header_t* __elf_header = (elf_header_t*)__buf;
	// checking for first four characters
	if(__elf_header->e_ident[EI_MAG0] != 0x7f || __elf_header->e_ident[EI_MAG1] != 'E' ||
		__elf_header->e_ident[EI_MAG2] != 'L' || __elf_header->e_ident[EI_MAG3] != 'F')
		return 0;
	// checking for 32-bit binary code
	if(__elf_header->e_ident[EI_CLASS] != ELFCLASS32)
		return 0;
	// checking for data encoding
	if(__elf_header->e_ident[EI_DATA] != ELFDATA2LSB)
		return 0;
	// checking for version
	if(__elf_header->e_ident[EI_VERSION] != EV_CURRENT)
		return 0;
	// checking for executable binary
	if(__elf_header->e_type != ET_EXEC)
		return 0;
	// checking for 80386 machine
	if(__elf_header->e_machine != EM_386)
		return 0;
	return 1;
}

static inline elf_program_header_t* elf_ph(u8_t* __buf, int __index){
	elf_header_t* __elf_header = (elf_header_t*)__buf;
	elf_program_header_t* __elf_program_header = (elf_program_header_t*)(__buf + __elf_header->e_phoff);
	return &__elf_program_header[__index];
}

static inline elf_section_header_t* elf_sh(u8_t* __buf, int __index){
	elf_header_t* __elf_header = (elf_header_t*)__buf;
	elf_section_header_t* __elf_section_header = (elf_section_header_t*)(__buf + __elf_header->e_shoff);
	return &__elf_section_header[__index];
}

#endif //__ELF_H__