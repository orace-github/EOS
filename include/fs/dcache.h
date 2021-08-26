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

#ifndef __DCACHE_H__
#define __DCACHE_H__

#include <fs/fs.h>
#include <memory.h>
#include <types.h>
#include <list.h>

typedef struct dcache
{
	u8_t* dcache_beg; // dcache beg 
	u8_t* dcache_end; // dcache end
	struct list fdentry; // free dentry list
	int dentry; // total dentry
	dentry_t* (*alloc_dentry)(struct dcache*);
	void (*free_dentry)(dentry_t*, struct dcache*);
} __attribute__((packed)) dcache_t;
dentry_t* alloc_dentry(dcache_t* __dcache);
void free_dentry(dentry_t* __dentry, dcache_t* __dcache);
void init_dcache(frame_t* __dcache_frame);
dcache_t* dcache();
#endif //__DCACHE_H__