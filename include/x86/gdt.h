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

#ifndef __GDT_H__
#define __GDT_H__
#include <types.h>

#define KERNEL_CS 0x08
#define KERNE_DS 0x10
#define KERNEL_ES 0x10
#define KERNEL_FS 0x10
#define KERNEL_GS 0x10
#define KERNEL_SS 0x10

#define USER_CS 0x1b
#define USER_DS 0x23
#define USER_ES 0x23
#define USER_FS 0x23
#define USER_GS 0x23
#define USER_SS 0x23
#define GDT_SIZE 0x06
#define TSS_SEGMENT	0x2b

#define DESCRIPTOR_FLAG 0x0c
#define DESCRIPTOR_PRESENT 0x80
#define DESCRIPTOR_DPL0 0x00
#define DESCRIPTOR_DPL1 0x20
#define DESCRIPTOR_DPL2 0x40
#define DESCRIPTOR_DPL3 0x60
#define SEGMENT_DESCRIPTOR 0x10
#define SYSTEM_DESCRIPTOR 0x00
#define CONFORMING_DESCRIPTOR 0x0c
#define NO_CONFORMING_DESCRIPTOR 0x08
#define READABLE_DESCRIPTOR 0x02
#define EXPANDING_DESCRIPTOR 0x04
#define NO_EXPANDING_DESCRIPTOR 0x00
#define WRITABLE_DESCRIPTOR 0x02

typedef struct
{
    u16_t limit_0_15;
    u16_t base_16_31;
    u8_t base_32_39;
    u8_t access;
    u8_t limit_48_51:4;
    u8_t flags:4;
    u8_t base_56_63;
}__attribute__((packed)) gdt_entry_t;


typedef struct sys_gdtr
{
    u16_t limit;
    u32_t base;
}__attribute__((packed)) gdtr_t ;


void init_gdt();
void load_gdtr();
void init_gdt_des(u32_t __base, u32_t __limit, u8_t __access,
 u8_t __flags, gdt_entry_t* __gdt_entry);

#endif //__GDT_H__
