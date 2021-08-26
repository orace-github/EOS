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


#include <x86/gdt.h>
#include <lib.h>
#include <mutex.h>
#include <io.h>

gdt_entry_t __gdt[GDT_SIZE]; // system gdt table declaration
static gdtr_t __gdtr; // system gdt register


void init_gdt_des(u32_t __base, u32_t __limit, u8_t __access,
 u8_t __flags, gdt_entry_t* __gdt_entry)
{
    __gdt_entry->limit_0_15 = (u16_t)(__limit & 0x0000ffff);
    __gdt_entry->base_16_31 = (u16_t)(__base & 0x0000ffff);
    __gdt_entry->base_32_39 = ((u8_t)(__base >> 16) & 0x000000ff);
    __gdt_entry->access = __access;
    __gdt_entry->limit_48_51 = ((__limit >> 16) & 0x0000000f);
    __gdt_entry->flags = (__flags & 0x0f);
    __gdt_entry->base_56_63 = ((__base >> 24) & 0x000000ff );
}

void load_gdtr()
{
    __gdtr.limit = sizeof(gdt_entry_t)*GDT_SIZE - 1;
    __gdtr.base = (u32_t)(&__gdt);
    __asm__ __volatile__("lgdt %0"::"m"(__gdtr));
}

void init_gdt()
{
    kprintf("Installing GDT... ");
    init_gdt_des(0x0,0x0,0x0,0x0,&__gdt[0]); // null descriptor
    init_gdt_des(0x0,0xfffff,DESCRIPTOR_PRESENT|DESCRIPTOR_DPL0|SEGMENT_DESCRIPTOR|NO_CONFORMING_DESCRIPTOR|
    READABLE_DESCRIPTOR,DESCRIPTOR_FLAG,&__gdt[1]); // kernel code descriptor
    init_gdt_des(0x0,0xfffff,DESCRIPTOR_PRESENT|DESCRIPTOR_DPL0|SEGMENT_DESCRIPTOR|NO_EXPANDING_DESCRIPTOR|
    WRITABLE_DESCRIPTOR,DESCRIPTOR_FLAG,&__gdt[2]); // kernel data descriptor
    init_gdt_des(0x0,0xfffff,DESCRIPTOR_PRESENT|DESCRIPTOR_DPL3|SEGMENT_DESCRIPTOR|CONFORMING_DESCRIPTOR|
    READABLE_DESCRIPTOR,DESCRIPTOR_FLAG, &__gdt[3]); // user code descriptor
    init_gdt_des(0x0,0xfffff,DESCRIPTOR_PRESENT|DESCRIPTOR_DPL3|SEGMENT_DESCRIPTOR|CONFORMING_DESCRIPTOR|
    WRITABLE_DESCRIPTOR,DESCRIPTOR_FLAG,&__gdt[4]); // user data descriptor
    memset(&__gdt[5],0,sizeof(gdt_entry_t));
    load_gdtr();
    kprintf("[ok]\n");
}
