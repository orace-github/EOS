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
#ifndef __DEVFS_H__
#define __DEVFS_H__
#include <fs/fs.h>
int devfs_lock(dentry_t* __dentry, u8_t __flags);
int devfs_unlock(dentry_t* __dentry);
int devfs_touch(dentry_t* __parent, u8_t* __name);
dentry_t* devfs_lookup(dentry_t* __dentry, u8_t* __name);
int devfs_listdir(dentry_t* __dentry);
dentry_t* devfs_mkdir(dentry_t* __parent, u8_t* __name);
dentry_t* devfs_mknode(dentry_t* __parent, u8_t* __name, u16_t __type);
dentry_t* devfs_rndir(dentry_t* __dentry, u8_t* __name);
int devfs_rmdir(dentry_t* __dentry, u8_t __option);
dentry_t* devfs_chmod(dentry_t* __dentry, u16_t __mode);
dentry_t* devfs_link(dentry_t* __dentry, dentry_t* __parent, u8_t* __name);
int devfs_write(dentry_t* __dentry, void* __buf, u32_t __size);
int devfs_read(dentry_t* __dentry, void* __buf, u32_t __size);
int devfs_unlink(dentry_t* __dentry);
int devfs_open(dentry_t* __dentry, u16_t __mode);
int devfs_close(dentry_t* __dentry);
int devfs_eof(dentry_t* __dentry);
int devfs_stat(dentry_t* __dentry, stat_t* __stat);
int devfs_seek(dentry_t* __dentry, u32_t __offset, u32_t __ref);
int devfs_flush(dentry_t* __dentry);
int devfs_mount(dentry_t* __devdentry, dentry_t* __dentry, u16_t __mode);
int devfs_umount(dentry_t* __dentry);
int devfs_mkfs(dentry_t* __dentry);
char* devfs_readfile(dentry_t* __dentry);
void devfs_sync(dentry_t* __dentry);
#endif //__DEVFS_H__