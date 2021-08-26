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

#ifndef __SHM_H__
#define __SHM_H__

#include <ipc/ipc.h>
#include <memory.h>
#include <types.h>
#include <list.h>

typedef struct
{
	frame_t* __frame;
	u32_t __flags;
	u32_t __extern_id; // define by user process
	struct list_head head;
} __attribute__((packed)) shm_t;


#define SHM_HASH_MAX	10

typedef struct shm_hashtable
{
	struct list shm_list[SHM_HASH_MAX];
	shm_t* (*find)(u32_t, struct shm_hashtable*);
	void (*insert)(shm_t*, struct shm_hashtable*);
	void (*remove)(shm_t*, struct shm_hashtable*);	
} shm_hashtable_t;


shm_t* alloc_shm(u32_t __extern_id, u32_t size, u8_t __flags, u32_t __pid);
int free_shm(int __pid, shm_t* __shm);
void unmap_shm(u32_t __extern_id);
void kunmap_shm(shm_t* __shm);
void init_shm_hashtable_t();

#endif //__SHM_H__