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

#include <x86/tss.h>
#include <mutex.h>
#include <io.h>

// TSS data
tss_t __tss;
extern gdt_entry_t __gdt[];

void  load_tss()
{
    __asm__ __volatile("movw %0, %%ax;"
                        "ltr %%ax;"
                        ::"i"(TSS_SEGMENT));
}

void init_tss()
{
	kprintf("Installing TSS...");
    u32_t __base = (u32_t)&__tss;
    u32_t __limit = __base + sizeof(tss_t);
    init_gdt_des(__base,__limit,TSS_ACCESS,TSS_FLAG,&__gdt[0x05]); // tss entry in gdt 0x28 | 0x03 = 0x2b
    load_tss();
    kprintf("[ok]\n");
}
