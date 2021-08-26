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

#ifndef __VFS_H__
#define __VFS_H__

#include <types.h>
#include <device.h>
#include <list.h>
#include <fs/fs.h>
#include <ipc/semaphore.h>
#include <string.h>
#include <process.h>
#include <memory.h>
#include <fs/efs2.h>

#define INIT_PATH(path) \
	(path)->size = 0;	\
	init_list(&(path)->e_list);
	
typedef struct{
	unsigned int semid;
	dentry_t* root;
	u8_t path[PATH_MAX_LEN];	
}__attribute__((packed)) vfs_t;

typedef struct{
	int fd;
	dentry_t* file;
	process_t* process;
	int index;
	struct list_head head;
}__attribute__((packed)) fd_t;

typedef struct{
	int index;
	u8_t* name;
	struct list_head head;	
}__attribute__((packed)) path_entry_t;

typedef struct{
	int size;
	struct list e_list;
}__attribute__((packed)) path_t;
int path_clean(u8_t* __path);
void path(u8_t* __path, path_t* __vpath);
void destroy_path(path_t* __path);
dentry_t* mount_point(u8_t* __path, u8_t** __lpath);

typedef struct fd_hashtable{
	#define FD_HASH_MAX	MAX_FILE
	int init;
	struct list fd_list[FD_HASH_MAX];
	void (*insert)(fd_t* __fd, struct fd_hashtable* __fd_hashtable);
	void (*remove)(fd_t* __fd, struct fd_hashtable* __fd_hashtable);
	fd_t* (*find)(int __id, struct fd_hashtable* __fd_hashtable);
}fd_hashtable_t;
static inline void fd_insert(fd_t* __fd, fd_hashtable_t* __fd_hashtable){
	int __key = __fd->fd % FD_HASH_MAX;
	list_insert(&__fd_hashtable->fd_list[__key],&__fd->head);
}

static inline void fd_remove(fd_t* __fd, fd_hashtable_t* __fd_hashtable){
	int __key = __fd->fd % FD_HASH_MAX;
	list_remove(&__fd_hashtable->fd_list[__key],&__fd->head);
}

static inline fd_t* fd_find(int __id, fd_hashtable_t* __fd_hashtable){
	struct list_head* __it = NULL;
	list_for_each(__it,&__fd_hashtable->fd_list[__id%FD_HASH_MAX].head)
	{
		fd_t* __fd = list_entry(__it,fd_t,head);
		if(__fd->fd = __id)
			return __fd;
	}
	return NULL;
}

static inline void init_fdhashtable(fd_hashtable_t* __fd_hashtable){
	if(__fd_hashtable->init == 0xff)
		return;
	for (int i = 0; i < FD_HASH_MAX; i++)
		init_list(&__fd_hashtable->fd_list[i]);
	__fd_hashtable->insert = fd_insert;
	__fd_hashtable->remove = fd_remove;
	__fd_hashtable->find = fd_find;
	__fd_hashtable->init = 0xff;
}

fd_t* fileid();
int init_vfs(dentry_t* __root, dev_t* __dev);
int vfs_mknode(u8_t* __path, u16_t __type);
int vfs_open(u8_t* __path, u16_t __mode);
int vfs_write(int __fd, void* __buf, u32_t __bytes);
int vfs_read(int __fd, void* __buf, u32_t __bytes);
int vfs_exist(u8_t* __path);
int vfs_mount(u8_t* __devpath, u8_t* __fpath, u16_t __mode);
int vfs_umount(u8_t* __path);
int vfs_mkdir(u8_t* __path);
int vfs_rndir(u8_t* __path, u8_t* __name);
int vfs_rmdir(u8_t* __path, u8_t __option);
int vfs_chdir(u8_t* __path);
int vfs_chmod(u8_t* __path, u16_t __mode);
int vfs_close(int __fd);
int vfs_eof(int __fd);
int vfs_lock(u8_t* __path, u8_t __flags);
int vfs_unlock(u8_t* __path);
int vfs_link(u8_t* __fpath, u8_t* __lpath);
int vfs_unlink(u8_t* __lpath);
int vfs_touch(u8_t* __path);
int vfs_stat(u32_t __fd, stat_t* __stat);
int vfs_seek(u32_t __fd, u32_t __off, u32_t __pos);
int vfs_flush(u32_t __fd);
char* vfs_readfile(u8_t* __path);
dentry_t* vfs_lookup(u8_t* __path);
void vfs_sync();

#endif //__VFS_H__