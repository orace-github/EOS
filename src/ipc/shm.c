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


#include <ipc/shm.h>

shm_t* shm_hashtable_t_find(u32_t __extern_id, shm_hashtable_t* __hashtable)
{
	int __index = __extern_id % SHM_HASH_MAX;
	struct list_head* it = NULL;

	list_for_each(it,&__hashtable->shm_list[__index].head)
	{
		shm_t* __shm = list_entry(it,shm_t,head);
		if(__shm->__extern_id == __extern_id)
			return __shm;
	}

	return NULL;
}

void shm_hashtable_t_insert(shm_t* __shm, shm_hashtable_t* __hashtable)
{
	if(!__shm)
		return;

	int __index = __shm->__extern_id % SHM_HASH_MAX;
	list_insert(&__hashtable->shm_list[__index],&__shm->head);
}

void shm_hashtable_t_remove(shm_t* __shm, shm_hashtable_t* __hashtable)
{
	if(!__shm)
		return;

	int __index = __shm->__extern_id % SHM_HASH_MAX;
	list_remove(&__hashtable->shm_list[__index],&__shm->head);
}	

// kernel shared memory hashtable
static shm_hashtable_t __shmhashtable;

void init_shm_hashtable_t()
{
	for(int i = 0; i < SHM_HASH_MAX; i++)
		init_list(&__shmhashtable.shm_list[i]);

	__shmhashtable.find = shm_hashtable_t_find;
	__shmhashtable.insert = shm_hashtable_t_insert;
	__shmhashtable.remove = shm_hashtable_t_remove;
}

shm_t* alloc_shm(u32_t __extern_id, u32_t size, u8_t __flags, u32_t __pid)
{
	shm_t* __shm = __shmhashtable.find(__extern_id,&__shmhashtable);
	if(__shm)
		return __shm;

	if(!IS_IPC_CREATE(__flags))
		return NULL;

	int __page = size / 0x1000;
	if(size % 0x1000)
		__page++;

	frame_t* __frame = alloc_frame(__pid,__page);
	if(!__frame)
		return NULL;

	__shm = sys_alloc(sizeof(shm_t));
	if(!__shm)
	{
		free_frame(__pid,__frame);
		return NULL;
	}

	__shm->__frame = __frame;
	__shm->__flags = __flags;
	__shm->__extern_id = __extern_id;
	__shmhashtable.insert(__shm,&__shmhashtable);

	return __shm;	
}

int free_shm(int __pid, shm_t* __shm)
{
	if(free_frame(__pid,__shm->__frame))
	{
		__shmhashtable.remove(__shm,&__shmhashtable);
		sys_free(__shm);
		return 1;	
	}
	
	return 0;
}

void kunmap_shm(shm_t* __shm)
{
	if(!__shm)
		return;

	unmap_frame(__shm->__frame);
}

void unmap_shm(u32_t __extern_id)
{
	shm_t* __shm = __shmhashtable.find(__extern_id,&__shmhashtable);
	kunmap_shm(__shm);
}