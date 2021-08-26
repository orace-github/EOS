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

#ifndef __TSS_H__
#define __TSS_H__

#include <types.h>
#include <lib.h>
#include <x86/gdt.h>


#define TSS_ACCESS 0xE9
#define TSS_FLAG 0x0

typedef struct
{
    u16_t link, unused_link;
    u32_t esp0;
    u16_t ss0, unused_ss0;
    u32_t esp1;
    u16_t ss1,unused_ss1;
    u32_t esp2;
    u16_t ss2, unused_ss2;
    u32_t cr3;
    u32_t eip;
    u32_t eflags;
    u32_t eax;
    u32_t ecx;
    u32_t edx;
    u32_t ebx;
    u32_t esp;
    u32_t ebp;
    u32_t esi;
    u32_t edi;
    u16_t es, unused_es;
    u16_t cs,unused_cs;
    u16_t ss ,unused_ss;
    u16_t ds,unused_ds;
    u16_t fs,unused_fs;
    u16_t gs, unused_gs;
    u16_t ldtr, unused_ldtr;
    u16_t unused_iomap, iomap;
} __attribute__((packed)) tss_t;

void init_tss();
void load_tss();

#endif //__TSS_H__
