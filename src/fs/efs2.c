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
#include <fs/efs2.h>

dentry_t* efs2_mknode(dentry_t* __parent, u8_t* __name, u16_t __type)
{
	
	// efs2_mknode doesn't make a link file
	if(IS_LINK_FILE(__type) || IS_LINK_FILE(__parent->d_mode))
		return NULL;
	// check if __parent dentry is actually a
	// directory
	if(!IS_DIRECTORY_FILE(__parent->d_mode))
		return NULL; 
	// check if __parent dentry was opened
	efs2_open(__parent,READ_O|WRITE_O|APPEND_O);
	efs2_data_t* __data = __parent->d_fs;
	// check if dentry was already created
	dentry_t* __dentry = __data->d_hash.find(__parent,__name,&__data->d_hash);
	if(__dentry && (__dentry->d_mode & DTYPE_MASK) == __type)
		return __dentry;

	LOCK();
	efs2_inode_t* __inode = sys_alloc(sizeof(efs2_inode_t));
	local_time(&__inode->i_ctime);
	local_time(&__inode->i_atime);
	__inode->i_mode = __type & DTYPE_MASK;
	__inode->i_link = 0;
	__inode->i_lock = 0;
	__inode->i_status = INODE_PRESENT;
	__inode->i_dev = __data->dev->dev_id;
	__inode->i_size = 0;
	__inode->i_num = efs2_alloc_inum(__data);
	if(__inode->i_num == -1)
	{
		UNLOCK();
		sys_free(__inode);
		return NULL;
	}
	__inode->i_pnum = __parent->d_inode;
	strcpy(__inode->i_name,__name);
	static dirent_t __dirent;
	__dirent.inode = __inode->i_num;
	write_inode_metadata(__data,__inode);
	// __dentry building
	__dentry = __data->dcache->alloc_dentry(__data->dcache);	
	// check if __dentry is valid
	if(!__dentry)
	{
		UNLOCK();
		sys_free(__inode);
		return NULL;
	}
	if(IS_DEVICE_FILE(__type))
		__dentry->d_fs = NULL;
	else
		__dentry->d_fs = __parent->d_fs;

	__dentry->parent = __parent;
	strcpy(__dentry->d_name,__name);
	if(!IS_DEVICE_FILE(__type))
		efs2_inode_to_dentry(__inode,__dentry);
	list_insert(&__parent->d_childs,&__dentry->d_head);
	__data->d_hash.insert(__dentry,&__data->d_hash);
	sys_free(__inode);
	efs2_write(__parent,&__dirent,sizeof(dirent_t));
	UNLOCK();
	return __dentry;
}

int efs2_touch(dentry_t* __parent, u8_t* __name)
{
	// create an ordinary file dentry
	if(efs2_mknode(__parent,__name,ORDINARY_FILE) != NULL)
		return 1;
	else
		return 0;
}

dentry_t* efs2_mkdir(dentry_t* __parent, u8_t* __name)
{
	// create a directory file dentry
	return efs2_mknode(__parent,__name,DIRECTORY_FILE);
}

int efs2_stat(dentry_t* __dentry, stat_t* __stat)
{
	// check if dentry was opened
	efs2_open(__dentry,READ_O|WRITE_O);
	// file system data from dentry
	efs2_data_t*  __data = __dentry->d_fs;
	LOCK();
	__stat->st_dev = __data->dev->dev_id;
	__stat->st_inode = __dentry->d_inode;
	__stat->st_mode = __dentry->d_mode;
	__stat->st_size = __dentry->d_size;
	__stat->st_blksize = __data->superblock.s_blksize;
	__stat->st_atime = __dentry->atime;
	__stat->st_ctime = __dentry->ctime;
	// inode
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	__stat->st_link = __inode->i_link;
	UNLOCK();
	return 1;
}

int efs2_eof(dentry_t* __dentry)
{
	// dentry can be all file unless directory
	if(IS_DIRECTORY_FILE(__dentry->d_mode) || IS_DEVICE_FILE(__dentry->d_mode))
		return 0;
	// file system data from dentry
	efs2_data_t*  __data = __dentry->d_fs;
	LOCK();
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	if(!__inode)
	{
		UNLOCK();
		return 0;
	}
	UNLOCK();
	return (__dentry->d_offset >= __inode->i_size);
}

dentry_t* efs2_chmod(dentry_t* __dentry, u16_t __mode)
{	
		// efs2_chmod doesn't change link file mode
	if(IS_LINK_FILE(__dentry->d_mode))
		return NULL;
	// dentry can be all file unless directory
	if(IS_DIRECTORY_FILE(__dentry->d_mode))
		return NULL;
	// check if dentry was opened
	efs2_open(__dentry,READ_O|WRITE_O);
	// file system data from dentry
	efs2_data_t* __data = __dentry->d_fs;
	LOCK();
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	if(!__inode)
	{
		UNLOCK();
		return NULL;
	}
	// update fields
	__dentry->d_mode &= ~DRIGHT_MASK;
	__inode->i_mode &= ~DRIGHT_MASK;	
	__dentry->d_mode |= (__mode & DRIGHT_MASK);
	__inode->i_mode |= (__mode & DRIGHT_MASK);
	write_inode_metadata(__data,__inode);
	UNLOCK();
	return __dentry;
}

int efs2_close(dentry_t* __dentry)
{
	// dentry can be all file unless directory
	if(IS_DIRECTORY_FILE(__dentry->d_mode))
		return 0;	
	// check if dentry was opened
	if(!__dentry->opened)
		return 0;
	
	// file system data from dentry
	efs2_data_t* __data = __dentry->d_fs;
	LOCK();
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	write_inode_metadata(__data,__inode);
	__dentry->opened = 0;
	// remove inode from main memory
	__data->i_hash.remove(__inode,&__data->i_hash);
	sys_free(__inode);
	UNLOCK();
	return 1;	
}

int efs2_seek(dentry_t* __dentry, u32_t __offset, u32_t __ref)
{
	// dentry can be all file unless directory
	if(IS_DIRECTORY_FILE(__dentry->d_mode))
		return 0;
	// check if dentry was opened
	if(!__dentry->opened)
		return 0;
	if(IS_PIPE_FILE(__dentry->d_mode) || IS_ORDINARY_FILE(__dentry->d_mode) || IS_LINK_FILE(__dentry->d_mode))
	{
		LOCK();
		if(__ref == SEEK_SET)
			__dentry->d_offset = __offset;
		else if(__ref == SEEK_CUR)
			__dentry->d_offset += __offset;
		else if(__ref == SEEK_END)
			__dentry->d_offset = __dentry->d_size - __offset;
		UNLOCK();
		return 1;	
	}
	return 0;		
}

dentry_t* efs2_lookup(dentry_t* __dentry, u8_t* __name)
{
	// check if dentry is a directory file
	if(!IS_DIRECTORY_FILE(__dentry->d_mode))
		return NULL;
	// check if dentry was opened
	efs2_open(__dentry,READ_O|WRITE_O);
	// traverse dentry childs at the lookup
	// of a dentry named "__name"
	struct list_head* __it = NULL;
	list_for_each(__it,&__dentry->d_childs.head)
	{
		dentry_t* __child = list_entry(__it,dentry_t,d_head);
		if(!strcmp(__child->d_name,__name))
			return __child;	
	}
	return NULL;
}

dentry_t* efs2_rndir(dentry_t* __dentry, u8_t* __name)
{
	// check if dentry was opened
	efs2_open(__dentry,READ_O|WRITE_O);
	efs2_data_t* __data = __dentry->d_fs;
	LOCK();
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	strncpy(__inode->i_name,__name,DNAME_MAX_LEN);
	strncpy(__dentry->d_name,__name,DNAME_MAX_LEN);
	write_inode_metadata(__data,__inode);
	UNLOCK();
	return __dentry;	
}

dentry_t* efs2_link(dentry_t* __dentry, dentry_t* __parent, u8_t* __name)
{
	// efs2 doesn't support link file
	return NULL;
}

int efs2_unlink(dentry_t* __dentry)
{
	// efs2 doesn't support link file
	return 0;
}

int efs2_flush(dentry_t* __dentry)
{
	// efs2 doesn' t flush a dentry
	// actually is here to avoid GCC warnning
	return 0;
}

int efs2_lock(dentry_t* __dentry, u8_t __flags)
{
	// file system data
	efs2_data_t* __data = __dentry->d_fs;
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	if(!__inode)
		return 0;
	__dentry->d_lock = (LOCKED_O | (__flags & DRIGHT_MASK));
	__inode->i_lock = __dentry->d_lock;
	write_inode_metadata(__data,__inode);
	return 1;
}

int efs2_unlock(dentry_t* __dentry)
{
	// file system data
	if(!IS_LOCKED_O(__dentry->d_lock))
		return 0;
	efs2_data_t* __data = __dentry->d_fs;
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	if(!__inode)
		return 0;
	__dentry->d_lock = __inode->i_lock = 0;
	write_inode_metadata(__data,__inode);
	return 1;
}

int efs2_open(dentry_t* __dentry, u16_t __mode)
{
	// check if dentry was already exist
	if(__dentry->opened)
		return 1;
	// file system data from dentry
	efs2_data_t* __data = __dentry->d_fs;
	// load inode in main memory
	efs2_inode_t* __inode = sys_alloc(sizeof(efs2_inode_t));
	if(!__inode)
		return 0;
	LOCK();
	read_inode_metadata(__data,__dentry->d_inode,__inode);
	__data->i_hash.insert(__inode,&__data->i_hash);
	__dentry->d_mode &= ~DRIGHT_MASK;
	__inode->i_mode &= ~DRIGHT_MASK;	
	__dentry->d_mode |= (__mode & DRIGHT_MASK);
	__inode->i_mode |= (__mode & DRIGHT_MASK);
	write_inode_metadata(__data,__inode);
	if(IS_DIRECTORY_FILE(__dentry->d_mode))
		__dentry->d_mode |= (READ_O|WRITE_O|APPEND_O);
	// check if write flags is set
	if(IS_WR_O(__dentry->d_mode))
	{
		if(IS_APPEND_O(__dentry->d_mode))
			__dentry->d_offset = __inode->i_size;
		else
		{
			// update both dentry and inode size
			__dentry->d_offset = 0;
			__dentry->d_size = 0;
			__inode->i_size = 0;
		}
	}
	__dentry->opened = 1;
	if(IS_DIRECTORY_FILE(__dentry->d_mode))
		efs2_listdir(__dentry);
	UNLOCK();
	return 1;
}

int efs2_listdir(dentry_t* __dentry)
{
	// file system data
	efs2_data_t* __data = __dentry->d_fs;
	LOCK();
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);	
	if(!__inode->i_size)
			return 0;
	char* __buf = efs2_inode_filedata(__data,__inode);
	efs2_inode_t __ibuf;
	dirent_t* __dirent  = (dirent_t*)__buf;
	u64_t __size = __inode->i_size;
	int __index = 0;

	while(__size)
	{
		read_inode_metadata(__data,__dirent[__index].inode,&__ibuf);	
		if(__ibuf.i_pnum == __dentry->d_inode && IS_INODE_PRESENT(&__ibuf))
		{
			dentry_t* __new = __data->dcache->alloc_dentry(__data->dcache);
			__new->parent = __dentry;
			__new->d_fs = __dentry->d_fs;
			list_insert(&__dentry->d_childs,&__new->d_head);
			efs2_inode_to_dentry(&__ibuf,__new);
			strcpy(__new->d_name,__ibuf.i_name);
			__data->d_hash.insert(__new,&__data->d_hash);
			//kprintf("#NEW DENTRY:%s --> %d\n",__ibuf.i_name,__ibuf.i_num);
		}
		__size -= sizeof(dirent_t);
		__index++;
	}
	sys_free(__buf);
	UNLOCK();
	return 1;
}

char* efs2_readfile(dentry_t* __dentry)
{
	// efs2_readfile only read link, ordinary
	// and pipe file
	if(IS_ORDINARY_FILE(__dentry->d_mode) || IS_LINK_FILE(__dentry->d_mode) || IS_PIPE_FILE(__dentry->d_mode))
	{
		// check if dentry was opened
		efs2_open(__dentry,READ_O);
		// file system data from dentry
		efs2_data_t* __data = __dentry->d_fs;
		LOCK();
		efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
		char* __membase = efs2_inode_filedata(__data,__inode);
		UNLOCK();
		return __membase;
	}
	return NULL;
}
	
int efs2_rmdir(dentry_t* __dentry, u8_t __option)
{
	// open __dentry
	efs2_open(__dentry,READ_O);
	// check if dentry child list is empty
	// actually not a directory file
	if(list_empty(&__dentry->d_childs.head))
	{
		// file system data from dentry
		efs2_data_t* __data = __dentry->d_fs;
		efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
		if(IS_INODE_LOCKED(__inode))
			return 0;	
		// remove dentry from both hashtable and memory
		LOCK();
		__data->d_hash.remove(__dentry,&__data->d_hash);
		list_remove(&__dentry->parent->d_childs,&__dentry->d_head);
		__data->dcache->free_dentry(__dentry,__data->dcache);
		// remove inode from both memory and mark INODE NOT PRESENT
		__data->i_hash.remove(__inode,&__data->i_hash);
		efs2_free_inode(__data,__inode);
		sys_free(__inode);
		UNLOCK();
		return 1; 
	}
	else
	{
		// check if recursion is enable
		// option:0 --> no recursion
		// option:1 --> recursion is set
		if(!__option)
			return 0;
		struct list_head* __it = NULL, *__next = NULL;
		list_for_each_safe(__it,__next,&__dentry->d_childs.head)
		{
			dentry_t* __child = list_entry(__it,dentry_t,d_head);
			efs2_rmdir(__child,__option);
		}
	}
	if(!list_empty(&__dentry->d_childs.head))
		return 0;
	else
		return efs2_rmdir(__dentry,__option);
	// avoid GCC warnning
	return 0;
}

void efs2_inode_to_dentry(efs2_inode_t* __inode, dentry_t* __dentry)
{
	__dentry->d_mode = __inode->i_mode;
	__dentry->d_inode = __inode->i_num;
	__dentry->opened = 0;
	__dentry->mounted = 0;
	__dentry->d_hide = 0;
	__dentry->d_offset = 0;
	__dentry->d_lock = __inode->i_lock;
	__dentry->d_size = __inode->i_size;
	__dentry->atime = __inode->i_atime;
	__dentry->ctime = __inode->i_ctime;
	__dentry->touch = efs2_touch;
	__dentry->write = efs2_write;
	__dentry->read = efs2_read;
	__dentry->lookup = efs2_lookup;
	__dentry->mkdir = efs2_mkdir;
	__dentry->mknode = efs2_mknode;
	__dentry->rndir = efs2_rndir;
	__dentry->rmdir = efs2_rmdir;
	__dentry->chmod = efs2_chmod;
	__dentry->lock = efs2_lock;
	__dentry->unlock = efs2_unlock;
	__dentry->link = efs2_link;
	__dentry->unlink = efs2_unlink;
	__dentry->open = efs2_open;
	__dentry->close = efs2_close;
	__dentry->eof = efs2_eof;
	__dentry->stat = efs2_stat;
	__dentry->seek = efs2_seek;
	__dentry->flush = efs2_flush;
	__dentry->mount = efs2_mount;
	__dentry->umount = efs2_umount;
	__dentry->readfile = efs2_readfile;
	__dentry->sync = efs2_sync;
	__dentry->mkfs = efs2_mkfs;
	__dentry->listdir = efs2_listdir;
}

char* efs2_inode_filedata(efs2_data_t*  __data, efs2_inode_t* __inode)
{
	// inode size
	u64_t __size = __inode->i_size;
	if(!__size)
		return NULL;
	LOCK();
	u64_t __offset = 0;
	char* __membase = sys_alloc(__size);
	efs2_inode_readwrite(__data,__inode,&__offset,__membase,__size,EFS2_READ);
	UNLOCK();
	return __membase;	
}

int efs2_inode_readwrite(efs2_data_t*  __data, efs2_inode_t* __inode, u64_t* __offset, void* __buf, u64_t __size, int __option)
{
	if(!__size)
		return 0;
	char* __memup = __buf;
	int __len = 0;
	// wait ata controller
	ata_delay();
	#define __blksize __data->blksize
	if((*__offset) < INODE_DIRECT_BLK*__blksize)
	{
		int __blkindex = (*__offset) / __blksize;
		int __blkoffset = (*__offset) % __blksize;
		// check if inode block already exist
		if(!__inode->i_block[__blkindex])
		{
			if(__option == EFS2_READ)
				return 0;
			__inode->i_block[__blkindex] = efs2_alloc_dnum(__data);
			//kprintf("inode block %d offset: %d\n",__inode->i_block[__blkindex],__blkoffset);
			if(!__inode->i_block[__blkindex])
				return 0;
		}
		
		int __msize = (__blksize - __blkoffset) > __size ? __size : (__blksize - __blkoffset);
		if(__option == EFS2_WRITE)
			__data->dev->write(block_to_lba(__data,__inode->i_block[__blkindex]),__blkoffset,__memup,__msize);
		else
			__data->dev->read(block_to_lba(__data,__inode->i_block[__blkindex]),__blkoffset,__memup,__msize);
		
		__memup += __msize;
		(*__offset) += __msize;
		__len = __msize;
		// check if d_offset is greater	than d_size
		if((*__offset) > __inode->i_size && __option == EFS2_WRITE)
			__inode->i_size = (*__offset);	

		// decrease size
		__size -= __msize;
		local_time(&__inode->i_atime);
		// write inode on disk
		write_inode_metadata(__data,__inode);
		__len += efs2_inode_readwrite(__data,__inode,__offset,__memup,__size,__option);
		return __len;
	}
	else
	{
		// new offset
		u64_t __noffset = (*__offset) - INODE_DIRECT_BLK*__blksize;
		u32_t* p = NULL;
		u32_t* pp = NULL;
		int __blkindex = __noffset / (__blksize*__blksize*__blksize/(sizeof(u32_t)*sizeof(u32_t)));
		int __blkoffset = (*__offset) % __blksize;

		u64_t __poffset = __noffset - __blkindex*(__blksize*__blksize*__blksize/(sizeof(u32_t)*sizeof(u32_t)));
		u32_t __pindex = __poffset / (__blksize*__blksize/(sizeof(u32_t)));
		u64_t __ppoffset = __poffset % (__blksize*__blksize/(sizeof(u32_t)));
		u32_t __ppindex = __ppoffset / __blksize;
		if(!__inode->i_block[INODE_DIRECT_BLK+__blkindex])
		{
			if(__option == EFS2_READ)
				return 0;
			__inode->i_block[INODE_DIRECT_BLK+__blkindex] = efs2_alloc_dnum(__data);
			if(!__inode->i_block[INODE_DIRECT_BLK+__blkindex])
				return 0;
		}
		
		p = sys_alloc(__blksize);
		efs2_read_block(__data,__inode->i_block[INODE_DIRECT_BLK+__blkindex],p);
		if(!p[__pindex])
		{
			if(__option == EFS2_READ)
				return 0;
			p[__pindex] = efs2_alloc_dnum(__data);
			if(!p[__pindex])
			{
				sys_free(p);
				return 0;
			}
		}

		pp = sys_alloc(__blksize);
		efs2_read_block(__data,p[__pindex],pp);
		if(!pp[__ppindex])
		{
			if(__option == EFS2_READ)
				return 0;
			pp[__ppindex] = efs2_alloc_dnum(__data);
			if(!pp[__ppindex])
			{
				sys_free(p);
				sys_free(pp);
				return 0;
			}
		}		

		int __msize = (__blksize - __blkoffset) > __size ? __size : (__blksize - __blkoffset);
		// write on disk
		if(__option == EFS2_WRITE)
			__data->dev->write(block_to_lba(__data,pp[__ppindex]),__blkoffset,__memup,__msize);
		else
			__data->dev->read(block_to_lba(__data,pp[__ppindex]),__blkoffset,__memup,__msize);

		__memup += __msize;
		(*__offset) += __msize;
		__len = __msize;
		// check if d_offset is greater	than d_size
		if((*__offset) > __inode->i_size && __option == EFS2_WRITE)
			__inode->i_size = (*__offset);	
		// decrease size
		__size -= __msize;
		local_time(&__inode->i_atime);
		// write inode on disk
		write_inode_metadata(__data,__inode);
		efs2_write_block(__data,__inode->i_block[INODE_DIRECT_BLK+__blkindex],p);
		efs2_write_block(__data,p[__pindex],pp);
		sys_free(p); sys_free(pp);
		__len += efs2_inode_readwrite(__data,__inode,__offset,__memup,__size,__option);
		return __len;	
	}

	return 0;
}

int efs2_write(dentry_t* __dentry, void* __buf, u32_t __size)
{
	// check if dentry was already opened
	if(!__dentry->opened || !__size || IS_LINK_FILE(__dentry->d_mode) || IS_DEVICE_FILE(__dentry->d_mode))
	{
		kprintf("ERROR [1]\n");
		return 0;
	}
	// check if write flag on dentry is set
	if(!IS_WR_O(__dentry->d_mode)  || (__dentry->d_lock & WRITE_O))
	{
		kprintf("ERROR [2]\n");
		return 0;
	}
	// file system from dentry
	efs2_data_t* __data = __dentry->d_fs;
	LOCK();
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	int __bytes = efs2_inode_readwrite(__data,__inode,&__dentry->d_offset,__buf,__size,EFS2_WRITE);
	if(__dentry->d_offset > __dentry->d_size)
		__dentry->d_size = __dentry->d_offset;
	UNLOCK();
	return __bytes;
}

int efs2_read(dentry_t* __dentry, void* __buf, u32_t __size)
{
	// check if dentry was already opened
	if(!__dentry->opened || !__size || IS_LINK_FILE(__dentry->d_mode) || IS_DEVICE_FILE(__dentry->d_mode))
		return 0;
	// check if read flag on dentry is set
	if(!IS_RD_O(__dentry->d_mode) || (__dentry->d_lock & READ_O))
		return 0;
	// file system from dentry
	efs2_data_t* __data = __dentry->d_fs;
	LOCK();
	efs2_inode_t* __inode = __data->i_hash.find(__dentry->d_inode,&__data->i_hash);
	int __bytes = efs2_inode_readwrite(__data,__inode,&__dentry->d_offset,__buf,__size,EFS2_READ);
	UNLOCK();
	return __bytes;
}

int efs2_mount(dentry_t* __devdentry, dentry_t* __dentry, u16_t __mode)
{
	// check if both __devdentry and __dentry are actually
	// device file and directory file
	if(!IS_DEVICE_FILE(__devdentry->d_mode) ||
		!IS_DIRECTORY_FILE(__dentry->d_mode))
	{
		#ifdef __DEBUG__
			kprintf("PANIC: MOUNT ERROR [1](DIRECTORY AND DEVICE REQUIRED)\n");
		#endif //__DEBUG__
		return 0;
	}
	
	// make sure that __dentry is empty
	if(!list_empty(&__dentry->d_childs.head))
	{
		#ifdef __DEBUG__
			kprintf("PANIC: MOUNT ERROR [2](DIRECTORY NO EMPTY)\n");
		#endif //__DEBUG__
		return 0;
	}

	// file system data from dentry
	efs2_data_t* __data = __devdentry->d_fs;
	if(__data->init != 0xff)
	{
		__data->blksize = 0x1000;
		__data->init = 0xff;
		init_ihashtable(&__data->i_hash);
		init_dhashtable(&__data->d_hash);
		INIT_MUTEX(&__data->mutex);
		__data->dev = __data->devmanager->find_by_dentry(__devdentry,
		__data->devmanager);
		__data->bds = sys_alloc(sizeof(efs2_bds_t));
	}
	LOCK();
	// check if both device are same
	if(!IS_FREE_DEVICE(__data->dev))
	{
		#ifdef __DEBUG__
			kprintf("PANIC: MOUNT ERROR [3](DEVICE ALREADY BUSY)\n");
		#endif //__DEBUG__
		UNLOCK();
		return 0;
	}

	__dentry->mounted = 1;
	__dentry->opened = 0;
	__dentry->d_fs = __devdentry->d_fs;
	__dentry->d_mode |= __mode & DRIGHT_MASK;
	__dentry->d_inode = __data->inode;
	__dentry->d_fsmode = __mode & DRIGHT_MASK;
	__data->dev->dev_stat |= BUSY_DEVICE|DIRTY_DEVICE;
	__data->dev->dev_fsname = "EFS2";
	// check if dentry is root of file system
	__dentry->touch = __devdentry->touch;
	__dentry->write = __devdentry->write;
	__dentry->read = __devdentry->read;
	__dentry->lookup = __devdentry->lookup;
	__dentry->mkdir = __devdentry->mkdir;
	__dentry->mknode = __devdentry->mknode;
	__dentry->rndir = __devdentry->rndir;
	__dentry->rmdir = __devdentry->rmdir;
	__dentry->chmod = __devdentry->chmod;
	__dentry->lock = __devdentry->lock;
	__dentry->unlock = __devdentry->unlock;
	__dentry->link = __devdentry->link;
	__dentry->unlink = __devdentry->unlink;
	__dentry->open = __devdentry->open;
	__dentry->close = __devdentry->close;
	__dentry->eof = __devdentry->eof;
	__dentry->stat = __devdentry->stat;
	__dentry->seek = __devdentry->seek;
	__dentry->flush = __devdentry->flush; 
	__dentry->mount = __devdentry->mount;
	__dentry->umount = __devdentry->umount;
	__dentry->readfile = __devdentry->readfile;
	__dentry->sync = __devdentry->sync;
	__dentry->mkfs = __devdentry->mkfs;
	__dentry->listdir = efs2_listdir;
	// reading superblock from disk
	__data->dev->read(__data->dev->dev_part->s_lba,0,&__data->superblock,
		sizeof(efs2_superblock_t));
	int __lba = __data->dev->dev_part->size;
	
	if(__data->superblock.s_signature != EFS2_SIGNATURE)
	{
		__data->superblock.s_signature = EFS2_SIGNATURE;
		__data->superblock.s_blksize = __data->blksize;
		__data->superblock.s_sec_per_blk = __data->blksize / 512;
		__data->superblock.s_isize = sizeof(efs2_inode_t);
		__data->superblock.s_mode = __mode & DRIGHT_MASK;
		__data->superblock.s_version = EFS2_VERSION;
		__data->superblock.s_stat = EFS2_DIRTY;
		local_time(&__data->superblock.s_ctime);
		local_time(&__data->superblock.s_atime);
		__data->superblock.s_bds_off = __data->dev->dev_part->s_lba + 1;
		__lba--;
		__data->superblock.s_inum_off = __data->superblock.s_bds_off + (sizeof(efs2_bds_t)/512);
		__lba -= (sizeof(efs2_bds_t)/512);
		if(__lba <= 0)
		{
			#ifdef __DEBUG__
				kprintf("PANIC: MOUNT ERROR [4](NO ENOUGH MEMORY)\n");
			#endif //__DEBUG__
			sys_free(__data->bds);
			__data->init = 0x0;
			UNLOCK();
			return 0;
		}
		__data->superblock.s_dnum_off = __data->superblock.s_inum_off + ((INODE_BTMPBLK_MAX*__data->blksize)/512);
		__lba -= ((INODE_BTMPBLK_MAX*__data->blksize)/512);
		if(__lba <= 0)
		{
			#ifdef __DEBUG__
				kprintf("PANIC: MOUNT ERROR [4](NO ENOUGH MEMORY)\n");
			#endif //__DEBUG__
			sys_free(__data->bds);
			__data->init = 0x0;
			UNLOCK();
			return 0;
		}
		__data->superblock.s_i_off = __data->superblock.s_dnum_off + ((DATA_BTMPBLK_MAX*__data->blksize)/512);
		__lba -= ((DATA_BTMPBLK_MAX*__data->blksize)/512);
		if(__lba <= 0)
		{
			#ifdef __DEBUG__
				kprintf("PANIC: MOUNT ERROR [4](NO ENOUGH MEMORY)\n");
			#endif //__DEBUG__
			sys_free(__data->bds);
			__data->init = 0x0;
			UNLOCK();
			return 0;
		}
		__data->superblock.s_d_off = __data->superblock.s_i_off + ((sizeof(efs2_inode_t)*8*__data->blksize*INODE_BTMPBLK_MAX)/512);
		__lba -= ((sizeof(efs2_inode_t)*8*__data->blksize*INODE_BTMPBLK_MAX) / 512);
		if(__lba <= 0)
		{
			#ifdef __DEBUG__
				kprintf("PANIC: MOUNT ERROR [4](NO ENOUGH MEMORY)\n");
			#endif //__DEBUG__
			sys_free(__data->bds);
			__data->init = 0x0;
			UNLOCK();
			return 0;
		}
		__data->bds->free_inode = 8*__data->blksize*INODE_BTMPBLK_MAX;
		__data->bds->free_block = __lba/__data->superblock.s_sec_per_blk;
		__data->superblock.s_i_free = __data->bds->free_inode;
		__data->superblock.s_d_free = __data->bds->free_block;
		// initialize inode bitmap block descriptor
		for(int i = 0; i < INODE_BTMPBLK_MAX; i++)
		{
			__data->bds->inode_bitmap[i].free = 1;
			__data->bds->inode_bitmap[i].block = __data->superblock.s_inum_off + i*__data->superblock.s_sec_per_blk;
		}
		// initialize data bitmap block descriptor
		for(int i = 0; i < DATA_BTMPBLK_MAX; i++)
		{
			__data->bds->data_bitmap[i].free = 1;
			__data->bds->data_bitmap[i].block = __data->superblock.s_dnum_off + i*__data->superblock.s_sec_per_blk;
		}

		// write superblock on disk
		__data->dev->write(__data->dev->dev_part->s_lba,0,&__data->superblock,
		sizeof(efs2_superblock_t));
		// write block descriptor on disk
		__data->dev->write(__data->superblock.s_bds_off,0,__data->bds,sizeof(efs2_bds_t));
		// for particular reason we hold block number 0
		efs2_hold_dnum(__data,0);
		efs2_hold_inum(__data,__data->inode);
		static efs2_inode_t __inode;
		__inode.i_num = 0;
		__inode.i_pnum = 0;
		strncpy(__inode.i_name,__dentry->d_name,DNAME_MAX_LEN);
		__inode.i_mode = DIRECTORY_FILE|(__mode & DRIGHT_MASK);
		write_inode_metadata(__data,&__inode);
	}
	else
	{
		// read block descriptor from disk
		__data->dev->read(__data->superblock.s_bds_off,0,__data->bds,sizeof(efs2_bds_t));
		__data->superblock.s_i_free = __data->bds->free_inode;
		__data->superblock.s_d_free = __data->bds->free_block;
		local_time(&__data->superblock.s_atime);
		// write superblock on disk
		__data->dev->write(__data->dev->dev_part->s_lba,0,&__data->superblock,
		sizeof(efs2_superblock_t));
	}
	// list dentry entry
	efs2_open(__dentry,READ_O|WRITE_O|APPEND_O);
	UNLOCK();
	return 1;
}

int efs2_umount(dentry_t* __dentry)
{
	// make sure that __dentry is actually
	// a directory file
	if(!IS_DIRECTORY_FILE(__dentry->d_mode) ||
		!IS_DEVICE_FILE(__dentry->d_mode))
		return 0;	

	// file system data from dentry
	efs2_data_t* __data = __dentry->d_fs;
	__data->init = 0x0;
	sys_free(__data->bds);
	// set __dev state on free
	__data->dev->dev_stat = FREE_DEVICE;
	__data->superblock.s_stat = EFS2_CLEAN;
	if(IS_DIRECTORY_FILE(__dentry->d_mode))
		__dentry->mounted = 0;
	
	struct list_head* __it = NULL, *__next = NULL;
	for(int i = 0; i < INODE_HASH_MAX; i++)
	{
		list_for_each_safe(__it,__next,&__data->i_hash.i_list[i].head)
		{
			efs2_inode_t* __inode = list_entry(__it,efs2_inode_t,i_head);
			list_del(&__inode->i_head);
			sys_free(__inode);
		}
	}

	for(int i = 0; i < DENTRY_HASH_MAX; i++)
	{
		list_for_each_safe(__it,__next,&__data->d_hash.d_list[i].head)
		{
			dentry_t* __dentry = list_entry(__it,dentry_t,d_hhead);
			list_remove(&__dentry->parent->d_childs,&__dentry->d_head);
			list_del(&__dentry->d_hhead);
			__data->dcache->free_dentry(__dentry,__data->dcache);
		}
	}
	sys_free(__data);
	return 1;
}

void efs2_sync(dentry_t* __dentry)
{
	// write block descriptor on disk
	efs2_data_t* __data = __dentry->d_fs;
	__data->dev->write(__data->superblock.s_bds_off,0,__data->bds,sizeof(efs2_bds_t));
}

int efs2_mkfs(dentry_t* __dentry)
{
	if(!IS_DEVICE_FILE(__dentry->d_mode))
		return 0;

	if(__dentry->d_fs != NULL)
	{
		kprintf("efs2_mkfs ERROR [2]\n");
		return 0;
	}

	__dentry->d_fs = sys_alloc(sizeof(efs2_data_t));
	efs2_data_t* __data = __dentry->d_fs;
	__data->devmanager = dev_manager();
	__data->dcache = dcache();
	__data->inode = 0;
	__dentry->touch = efs2_touch; __dentry->write = efs2_write;
	__dentry->read = efs2_read; __dentry->lookup = efs2_lookup;
	__dentry->mkdir = efs2_mkdir; __dentry->mknode = efs2_mknode;
	__dentry->rmdir = efs2_rmdir; __dentry->chmod = efs2_chmod;
	__dentry->lock = efs2_lock; __dentry->unlock = efs2_unlock;
	__dentry->link = efs2_link; __dentry->unlink = efs2_unlink;
	__dentry->open = efs2_open; __dentry->close = efs2_close; 
	__dentry->eof = efs2_eof; __dentry->stat = efs2_stat; 
	__dentry->seek = efs2_seek; __dentry->flush = efs2_flush; 
	__dentry->mount = efs2_mount; __dentry->umount = efs2_umount; 
	__dentry->readfile = efs2_readfile; __dentry->sync = efs2_sync; 
	__dentry->mkfs = efs2_mkfs; __dentry->listdir = efs2_listdir;
	__dentry->rndir = efs2_rndir;
	return 1;
}

int efs2_alloc_inum(efs2_data_t* __data)
{
	// check if free block is empty
	if(!__data->bds->free_inode)
		return -1;	

	for(int i = 0; i < INODE_BTMPBLK_MAX; i++)
	{
		if(!__data->bds->inode_bitmap[i].free)
			continue;
		// read block from disk
		u32_t __block = __data->bds->inode_bitmap[i].block;
		u32_t* __blk = sys_alloc(__data->blksize);
		__data->dev->read(__block,0,__blk,__data->blksize);
		for(int j = 0; j < __data->blksize/4; j++)
		{
			if(__blk[j] == 0xffffffff)
				continue;
			for(int k = 0; k < 32; k++)
				if(!(__blk[j] & (1 << k)))
				{
					__blk[j] |= (1 << k);
					__data->dev->write(__block,0,__blk,__data->blksize);
					u32_t __inum = j*32 + k;
					if(__inum == __data->blksize*8-1)
						__data->bds->inode_bitmap[i].free = 0;
					sys_free(__blk);
					__inum += i*__data->blksize*8;
					__data->bds->free_inode--;
					return __inum;
				}
		}
	}
	return -1;
}

void efs2_free_inum(efs2_data_t* __data, int __inum)
{
	if(__inum == -1)
		return;
	// find __dnum block
	u32_t __blkindex = __inum / __data->blksize*8;
	u32_t __block = __data->bds->inode_bitmap[__blkindex].block;
	__inum = __inum % __data->blksize*8;
	u32_t __index = __inum / 32;
	u32_t __offset = __inum % 32;

	// read block from disk
	u32_t* __blk = sys_alloc(__data->blksize);
	__data->dev->read(__block,0,__blk,__data->blksize);
	// check if block was already full
	if(!__data->bds->inode_bitmap[__blkindex].free)
		__data->bds->inode_bitmap[__blkindex].free = 1;

	// free bitmap field
	__blk[__index] &= ~(1 << __offset);
	__data->bds->free_inode++;
	// write block on disk
	__data->dev->write(__block,0,__blk,__data->blksize);
	sys_free(__blk);	
}

void efs2_hold_inum(efs2_data_t*  __data, int __inum)
{
	if(__inum == -1)
		return;
	// find __dnum block
	u32_t __blkindex = __inum / __data->blksize*8;
	u32_t __block = __data->bds->inode_bitmap[__blkindex].block;
	__inum = __inum % __data->blksize*8;
	u32_t __index = __inum / 32;
	u32_t __offset = __inum % 32;

	// read block from disk
	u32_t* __blk = sys_alloc(__data->blksize);
	__data->dev->read(__block,0,__blk,__data->blksize);
	// check if block was already full
	if(__inum == __data->blksize*8-1)
		__data->bds->inode_bitmap[__blkindex].free = 0;

	// free bitmap field
	__blk[__index] |= (1 << __offset);
	__data->bds->free_inode--;
	// write block on disk
	__data->dev->write(__block,0,__blk,__data->blksize);
	sys_free(__blk);	
}

u32_t efs2_alloc_dnum(efs2_data_t*  __data)
{
	// check if free block is empty
	if(!__data->bds->free_block)
		return 0;	

	for(int i = 0; i < DATA_BTMPBLK_MAX; i++)
	{
		if(!__data->bds->data_bitmap[i].free)
			continue;
		// read block from disk
		u32_t __block = __data->bds->data_bitmap[i].block;
		u32_t* __blk = sys_alloc(__data->blksize);
		__data->dev->read(__block,0,__blk,__data->blksize);
		for(int j = 0; j < __data->blksize/4; j++)
		{
			if(__blk[j] == 0xffffffff)
				continue;
			for(int k = 0; k < 32; k++)
				if(!(__blk[j] & (1 << k)))
				{
					__blk[j] |= (1 << k);
					__data->dev->write(__block,0,__blk,__data->blksize);
					u32_t __dnum = j*32 + k;
					if(__dnum == __data->blksize*8-1)
						__data->bds->data_bitmap[i].free = 0;
					sys_free(__blk);
					__dnum += i*__data->blksize*8;
					__data->bds->free_block--;
					return __dnum;
				}
		}
	}
	return 0;
}

void efs2_free_dnum(efs2_data_t*  __data, u32_t __dnum)
{
	// find __dnum block
	u32_t __blkindex = __dnum / __data->blksize*8;
	u32_t __block = __data->bds->data_bitmap[__blkindex].block;
	__dnum = __dnum % __data->blksize*8;
	u32_t __index = __dnum / 32;
	u32_t __offset = __dnum % 32;

	// read block from disk
	u32_t* __blk = sys_alloc(__data->blksize);
	__data->dev->read(__block,0,__blk,__data->blksize);
	// check if block was already full
	if(!__data->bds->data_bitmap[__blkindex].free)
		__data->bds->data_bitmap[__blkindex].free = 1;

	// free bitmap field
	__blk[__index] &= ~(1 << __offset);
	__data->bds->free_block++;
	// write block on disk
	__data->dev->write(__block,0,__blk,__data->blksize);
	sys_free(__blk);
}

void efs2_hold_dnum(efs2_data_t*  __data, u32_t __dnum)
{
	// find __dnum block
	u32_t __blkindex = __dnum / __data->blksize*8;
	u32_t __block = __data->bds->data_bitmap[__blkindex].block;
	__dnum = __dnum % __data->blksize*8;
	u32_t __index = __dnum / 32;
	u32_t __offset = __dnum % 32;

	// read block from disk
	u32_t* __blk = sys_alloc(__data->blksize);
	__data->dev->read(__block,0,__blk,__data->blksize);
	// check if block was already full
	if(__dnum == __data->blksize*8-1)
		__data->bds->data_bitmap[__blkindex].free = 0;

	// free bitmap field
	__blk[__index] |= (1 << __offset);
	__data->bds->free_block--;
	// write block on disk
	__data->dev->write(__block,0,__blk,__data->blksize);
	sys_free(__blk);	
}

void write_inode_metadata(efs2_data_t* __data, efs2_inode_t* __inode)
{
	LOCK();
	char* __buf = sys_alloc(__data->blksize);
	int __inode_in_block = __data->blksize / __data->superblock.s_isize;
	int __inode_block = __inode->i_num / __inode_in_block;
	int __inode_offset = __inode->i_num % __inode_in_block;
	int __lba = __data->superblock.s_i_off + (__inode_block * __data->superblock.s_sec_per_blk);
	__data->dev->read(__lba,0,__buf,__data->blksize);
	memcpy(__buf + __inode_offset * __data->superblock.s_isize,__inode,__data->superblock.s_isize);
	__data->dev->write(__lba,0,__buf,__data->blksize);
	sys_free(__buf);
	UNLOCK();
}

void read_inode_metadata(efs2_data_t* __data, u32_t __inum, efs2_inode_t* __inode)
{
	// ## inode size has been a divide of blksize ##
	LOCK();
	char* __buf = sys_alloc(__data->blksize);
	int __inode_in_block = __data->blksize / __data->superblock.s_isize;
	int __inode_block = __inum / __inode_in_block;
	int __inode_offset = __inum % __inode_in_block;
	int __lba = __data->superblock.s_i_off + (__inode_block * __data->superblock.s_sec_per_blk);
	__data->dev->read(__lba,0,__buf,__data->blksize);
	memcpy(__inode,__buf + __inode_offset * __data->superblock.s_isize,__data->superblock.s_isize);
	sys_free(__buf);
	UNLOCK();
}

void efs2_write_block(efs2_data_t* __data, u32_t __block, void* __buf)
{
	LOCK();
	__data->dev->write(block_to_lba(__data,__block),0,__buf,__data->superblock.s_blksize);
	UNLOCK();
}

void efs2_read_block(efs2_data_t* __data, u32_t __block, void* __buf)
{
	LOCK();
	__data->dev->read(block_to_lba(__data,__block),0,__buf,__data->superblock.s_blksize);
	UNLOCK();
}

void efs2_free_inode(efs2_data_t* __data, efs2_inode_t* __inode)
{
	// set inode status not present
	__inode->i_status &= INODE_DELETE;
	// free inode number
	efs2_free_inum(__data,__inode->i_num);
	// inode has no parent :)
	__inode->i_pnum = -1;
	// free inode data number
	// inode direct block 
	for(int i = 0; i < INODE_DIRECT_BLK; i++)
	{
		if(__inode->i_block[i])
			efs2_free_dnum(__data,__inode->i_block[i]);
		__inode->i_block[i] = 0;
	}

	u32_t* p = sys_alloc(__data->blksize);
	u32_t* pp = sys_alloc(__data->blksize);
	// inode double indirect block
	for(int i = INODE_DIRECT_BLK; i < (INODE_DIRECT_BLK+INODE_DINDIRECT_BLK);
		i++)
	{
		if(__inode->i_block[i])
		{
			// read indirect block
			efs2_read_block(__data,__inode->i_block[i],p);
			for(int j = 0; j < __data->blksize/4; j++)
			{	
				if(p[j])
				{
					// read double indirect block
					efs2_read_block(__data,p[j],pp);
					for(int k = 0; k < __data->blksize/4; k++)
					{
						if(pp[k])
							efs2_free_dnum(__data,pp[k]);
					}
				}		
			}
			efs2_free_dnum(__data,__inode->i_block[i]);
			__inode->i_block[i] = 0;
		}
	}
	write_inode_metadata(__data,__inode);
	sys_free(p);
	sys_free(pp);
}