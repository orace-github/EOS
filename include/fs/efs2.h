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

#ifndef __EFS2_H__
#define __EFS2_H__ 

#include <fs/dcache.h>
#include <x86/pic.h>
#include <x86/spinlock.h>
#include <fs/fs.h>
#include <device.h>
#include <time.h>
#include <ipc/semaphore.h>
#include <list.h>
#include <driver/ata.h>
#include <math.h>
#include <mutex.h>

#define EFS2_SIGNATURE 		0xEF50
#define EFS2_CLEAN			0x0000
#define EFS2_DIRTY			0x0001
#define EFS2_VERSION		0x0002
#define EFS2_READ 			0x0004
#define EFS2_WRITE 			0x0008

#define INODE_HASH_MAX 				50
#define INODE_DIRECT_BLK			0x0A
#define INODE_DINDIRECT_BLK			0x03
#define INODE_PRESENT				0x0001
#define INODE_DELETE				0x0002
#define INODE_BTMPBLK_MAX 			32
#define DATA_BTMPBLK_MAX			(4096*4) // volume max size (2 To)

#define LOCK() desable_interrupt(PIC_IRQ0_MSK)
#define UNLOCK() enable_interrupt()

#define IS_EFS2_SIGNATURE(superblock)		\
	((superblock)->s_signature == EFS2_SIGNATURE)

#define IS_EFS2_CLEAN(superblock)			\
	((superblock)->s_stat == EFS2_CLEAN)

#define IS_EFS2_DIRTY(superblock)			\
	((superblock)->s_stat == EFS2_DIRTY)

#define IS_EFS2_VERSION(superblock)		\
	((superblock)->s_version == EFS2_VERSION)

#define IS_INODE_PRESENT(inode)					\
	((inode)->i_status & INODE_PRESENT)

#define IS_INODE_DELETE(inode)					\
	((inode)->i_status & INODE_DELETE)

#define IS_INODE_LOCKED(inode)					\
	((inode)->i_lock & LOCKED_O)

#define EFS2_DEBUG(__debug)	kprintf(__debug)

typedef struct
{
	time_t s_ctime;
	time_t s_atime;
	u32_t s_blksize;
	u32_t s_sec_per_blk;
	u32_t s_isize;
	u16_t s_mode;
	u32_t s_bds_off; // lba
	u32_t s_inum_off; // lba
	u32_t s_dnum_off; // lba
	u32_t s_i_off; // lba
	u32_t s_d_off; // lba
	u32_t s_i_free;
	u32_t s_d_free;
	u32_t s_version;
	u32_t s_stat;
	u32_t s_signature;
}__attribute__((packed)) efs2_superblock_t;

typedef struct
{
	u32_t block;
	u32_t free;	
}__attribute__((packed)) efs2_bds_entry_t;

typedef struct
{
	efs2_bds_entry_t inode_bitmap[INODE_BTMPBLK_MAX];
	efs2_bds_entry_t data_bitmap[DATA_BTMPBLK_MAX];
	u32_t free_block;
	u32_t free_inode;
	u8_t reserved[760]; // align on block size
} __attribute__((packed)) efs2_bds_t;

typedef struct
{
	time_t i_ctime;
	time_t i_atime;
	u16_t i_mode;
	u16_t i_lock;
	u32_t i_link; // ## efs2 doesn't yet support link file ##
	u8_t i_name[DNAME_MAX_LEN];
	int i_num;
	int i_pnum;
	int i_block[INODE_DIRECT_BLK+INODE_DINDIRECT_BLK]; 
	u32_t i_dev;
	u64_t i_size;
	u16_t i_status;
	struct list_head i_head;
} __attribute__((packed)) efs2_inode_t;

typedef struct inode_hashtable
{
	struct list i_list[INODE_HASH_MAX];
	void (*insert)(efs2_inode_t*, struct inode_hashtable*);
	void (*remove)(efs2_inode_t*, struct inode_hashtable*);
	efs2_inode_t* (*find)(u32_t, struct inode_hashtable*);
}inode_hashtable_t;

typedef struct dentry_hashtable
{
	#define DENTRY_HASH_MAX	50
	u8_t init;
	struct list d_list[DENTRY_HASH_MAX];
	void (*insert)(dentry_t* __dentry, struct dentry_hashtable* __hashtable);
	void (*remove)(dentry_t* __dentry, struct dentry_hashtable* __hashtable);
	dentry_t* (*find)(dentry_t* __parent, u8_t* __name,
		struct dentry_hashtable* __hashtable);
}dentry_hashtable_t;

typedef struct
{
	int init;
	int blksize;
	inode_hashtable_t i_hash;
	dentry_hashtable_t d_hash;
	dcache_t* dcache;
	efs2_bds_t* bds;
	dev_t* dev;
	int inode;
	struct mutex mutex;
	dev_manager_t* devmanager;
	efs2_superblock_t superblock;
} __attribute__((packed)) efs2_data_t;


static inline void inode_hashtable_insert(efs2_inode_t* __inode,
	inode_hashtable_t* __hashtable)
{
	list_insert(&__hashtable->i_list[__inode->i_num % INODE_HASH_MAX],&__inode->i_head);
}

static inline void inode_hashtable_remove(efs2_inode_t* __inode,
	inode_hashtable_t* __hashtable)
{
	list_remove(&__hashtable->i_list[__inode->i_num % INODE_HASH_MAX], &__inode->i_head);
}

static inline efs2_inode_t* inode_hashtable_find(u32_t __key, inode_hashtable_t* __hashtable)
{
	struct list_head* it = NULL;
	list_for_each(it,&__hashtable->i_list[__key % INODE_HASH_MAX].head)
	{
		efs2_inode_t* __inode = list_entry(it,efs2_inode_t,i_head);
		if(__inode->i_num == __key)
			return __inode;
	}	
	return NULL;
}

static inline void init_ihashtable(inode_hashtable_t* __hashtable)
{
	for(int i = 0; i < INODE_HASH_MAX; i++)
		init_list(&__hashtable->i_list[i]);
	__hashtable->insert = inode_hashtable_insert;
	__hashtable->remove = inode_hashtable_remove;
	__hashtable->find = inode_hashtable_find;
}

static inline void dentry_insert(dentry_t* __dentry, dentry_hashtable_t* __hashtable)
{
	int __key = 0;
	for(int i = 0; __dentry->d_name[i] != '\0'; i++)
		__key += __dentry->d_name[i];
	__key %= DENTRY_HASH_MAX;
	list_insert(&__hashtable->d_list[__key],&__dentry->d_hhead);
}

static inline void dentry_remove(dentry_t* __dentry, dentry_hashtable_t* __hashtable)
{
	int __key = 0;
	for(int i = 0; __dentry->d_name[i] != '\0'; i++)
		__key += __dentry->d_name[i];
	__key %= DENTRY_HASH_MAX;
	list_remove(&__hashtable->d_list[__key],&__dentry->d_hhead);	
}

static inline dentry_t* dentry_find(dentry_t* __parent, u8_t* __name,
	dentry_hashtable_t* __hashtable)
{
	int __key = 0;
	for(int i = 0; __name[i] != '\0'; i++)
		__key += __name[i];
	__key %= DENTRY_HASH_MAX;

	struct list_head* __it = NULL;
	list_for_each(__it,&__hashtable->d_list[__key].head)
	{
		dentry_t* __dentry = list_entry(__it,dentry_t,d_hhead);
		if(!strcmp(__dentry->d_name,__name) && __dentry->parent == __parent)
			return __dentry;
	}

	return NULL;
}

static inline void init_dhashtable(dentry_hashtable_t* __dentry_hashtable)
{
	if(__dentry_hashtable->init == 0xff)
		return;
	__dentry_hashtable->insert = dentry_insert;
	__dentry_hashtable->remove = dentry_remove;
	__dentry_hashtable->find = dentry_find;
	for(int i = 0; i < DENTRY_HASH_MAX; i++)
		init_list(&__dentry_hashtable->d_list[i]);
	__dentry_hashtable->init = 0xff;
}

static inline u32_t lba_to_block(efs2_data_t*  __data, u32_t __lba)
{
	__lba -= __data->superblock.s_d_off;
	return __lba / __data->superblock.s_sec_per_blk;
}

static inline u32_t block_to_lba(efs2_data_t*  __data, u32_t __block)
{
	return (__data->superblock.s_d_off + __block * __data->superblock.s_sec_per_blk);
}


int efs2_alloc_inum(efs2_data_t* __data);
void efs2_free_inum(efs2_data_t* __data, int __inum);
void efs2_hold_inum(efs2_data_t* __data, int __inum);
u32_t efs2_alloc_dnum(efs2_data_t* __data);
void efs2_free_dnum(efs2_data_t* __data, u32_t __dnum);
void efs2_hold_dnum(efs2_data_t* __data, u32_t __dnum);
void write_inode_metadata(efs2_data_t* __data, efs2_inode_t* __inode);
void read_inode_metadata(efs2_data_t* __data, u32_t __inum, efs2_inode_t* __inode);
void efs2_write_block(efs2_data_t* __data, u32_t __block, void* __buf);
void efs2_read_block(efs2_data_t* __data, u32_t __block, void* __buf);
void efs2_free_inode(efs2_data_t* __data, efs2_inode_t* __inode);
char* efs2_inode_filedata(efs2_data_t* __data, efs2_inode_t* __inode);
int efs2_inode_readwrite(efs2_data_t* __data, efs2_inode_t* __inode, u64_t* __offset, void* __buf, u64_t __size, int __option);
void efs2_inode_to_dentry(efs2_inode_t* __inode, dentry_t* __dentry);
int efs2_lock(dentry_t* __dentry, u8_t __flags);
int efs2_unlock(dentry_t* __dentry);
int efs2_touch(dentry_t* __parent, u8_t* __name);
int efs2_write(dentry_t* __dentry, void* __buf, u32_t __size);
int efs2_read(dentry_t* __dentry, void* __buf, u32_t __size);
dentry_t* efs2_lookup(dentry_t* __dentry, u8_t* __name);
int efs2_listdir(dentry_t* __dentry);
dentry_t* efs2_mkdir(dentry_t* __parent, u8_t* __name);
dentry_t* efs2_mknode(dentry_t* __parent, u8_t* __name, u16_t __type);
dentry_t* efs2_rndir(dentry_t* __dentry, u8_t* __name);
int efs2_rmdir(dentry_t* __dentry, u8_t __option);
dentry_t* efs2_chmod(dentry_t* __dentry, u16_t __mode);
dentry_t* efs2_link(dentry_t* __dentry, dentry_t* __parent, u8_t* __name);
int efs2_unlink(dentry_t* __dentry);
int efs2_open(dentry_t* __dentry, u16_t __mode);
int efs2_close(dentry_t* __dentry);
int efs2_eof(dentry_t* __dentry);
int efs2_stat(dentry_t* __dentry, stat_t* __stat);
int efs2_seek(dentry_t* __dentry, u32_t __offset, u32_t __ref);
int efs2_flush(dentry_t* __dentry);
int efs2_mount(dentry_t* __devdentry, dentry_t* __dentry, u16_t __mode);
int efs2_umount(dentry_t* __dentry);
int efs2_mkfs(dentry_t* __dentry);
char* efs2_readfile(dentry_t* __dentry);
void efs2_sync(dentry_t* __dentry);
// Debug function
void efs2_dirent_db(dentry_t* __dentry);
#endif //__EFS2_H__