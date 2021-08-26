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

#ifndef __BOOT_H__
#define __BOOT_H__
#include <types.h>

typedef struct
{
   u32_t flags;
   u32_t mem_lower;
   u32_t mem_upper;
   u32_t boot_device;
   u32_t cmdline;
   u32_t mods_count;
   u32_t mods_addr;
   u32_t num;
   u32_t size;
   u32_t addr;
   u32_t shndx;
   u32_t mmap_length;
   u32_t mmap_addr;
   u32_t drives_length;
   u32_t drives_addr;
   u32_t config_table;
   u32_t boot_loader_name;
   u32_t apm_table;
   u32_t vbe_control_info;
   u32_t vbe_mode_info;
   u32_t vbe_mode;
   u32_t vbe_interface_seg;
   u32_t vbe_interface_off;
   u32_t vbe_interface_len;
}__attribute__((packed)) multiboot_t;

#endif //__BOOT_H__