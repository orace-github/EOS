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
#include <fs/dcache.h>

dcache_t __dcache = {
	.alloc_dentry = alloc_dentry,
	.free_dentry = free_dentry,
};

dentry_t* alloc_dentry(dcache_t* __dcache)
{
	// find through free dentry list
	if(!list_empty(&__dcache->fdentry.head))
	{
		// get first entry of list
		// and then remove it from
		// free list
		dentry_t* __dentry = list_first_entry(&__dcache->fdentry.head,dentry_t,d_chead);
		list_remove(&__dcache->fdentry,&__dentry->d_chead);
		init_list(&__dentry->d_childs);
		return __dentry;
	}

	// find through dentry cache array
	if(__dcache->dentry < (int)(__dcache->dcache_end - __dcache->dcache_beg) / sizeof(dentry_t))
	{
		// get next dentry of array
		// and then return it after
		// increase dcache_dentry value
		// of 1		
		dentry_t* __dentry = (dentry_t*)(__dcache->dcache_beg);
		__dentry = &__dentry[__dcache->dentry];
		init_list(&__dentry->d_childs);
		__dcache->dentry++;
		return __dentry;
	}
	return NULL;
}

void free_dentry(dentry_t* __dentry, dcache_t* __dcache)
{
	// add dentry of dcache in free dentry list
	list_insert(&__dcache->fdentry,&__dentry->d_chead);
}

void init_dcache(frame_t* __dcache_frame)
{
	// ## initialize both memory location and list head ##
	init_list(&__dcache.fdentry);
	__dcache.dcache_beg = (u8_t*)__dcache_frame->p_addr;
	__dcache.dcache_end = (u8_t*)(__dcache.dcache_beg + __dcache_frame->page * PAGE_SIZE);
	__dcache.dentry = 0;
}

dcache_t* dcache()
{
	// ## simply return __dcache memory location ##
	return &__dcache;
}