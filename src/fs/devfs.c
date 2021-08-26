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
#include <fs/devfs.h>
extern int efs2_lock(dentry_t* __dentry, u8_t __flags);
extern int efs2_unlock(dentry_t* __dentry);
extern int efs2_touch(dentry_t* __parent, u8_t* __name);
extern dentry_t* efs2_lookup(dentry_t* __dentry, u8_t* __name);
extern int efs2_listdir(dentry_t* __dentry);
extern dentry_t* efs2_mkdir(dentry_t* __parent, u8_t* __name);
extern dentry_t* efs2_mknode(dentry_t* __parent, u8_t* __name, u16_t __type);
extern dentry_t* efs2_rndir(dentry_t* __dentry, u8_t* __name);
extern int efs2_rmdir(dentry_t* __dentry, u8_t __option);
extern dentry_t* efs2_chmod(dentry_t* __dentry, u16_t __mode);
extern dentry_t* efs2_link(dentry_t* __dentry, dentry_t* __parent, u8_t* __name);
extern int efs2_unlink(dentry_t* __dentry);
extern int efs2_open(dentry_t* __dentry, u16_t __mode);
extern int efs2_close(dentry_t* __dentry);
extern int efs2_eof(dentry_t* __dentry);
extern int efs2_stat(dentry_t* __dentry, stat_t* __stat);
extern int efs2_seek(dentry_t* __dentry, u32_t __offset, u32_t __ref);
extern int efs2_flush(dentry_t* __dentry);
extern int efs2_mount(dentry_t* __devdentry, dentry_t* __dentry,
	u16_t __mode);
extern int efs2_umount(dentry_t* __dentry);
extern int efs2_mkfs(dentry_t* __dentry);
extern char* efs2_readfile(dentry_t* __dentry);
extern void efs2_sync(dentry_t* __dentry);

int devfs_lock(dentry_t* __dentry, u8_t __flags)
{
	// ## call efs2_lock ##
	return efs2_lock(__dentry,__flags);
}

int devfs_unlock(dentry_t* __dentry)
{
	// ## call efs2_unlock ##
	return efs2_unlock(__dentry);
}

int devfs_touch(dentry_t* __parent, u8_t* __name)
{
	// ## not allowed for device file ##
	return 0;
}

dentry_t* devfs_lookup(dentry_t* __dentry, u8_t* __name)
{
	// ## not allowed for device file ##
	return 0;
}

int devfs_listdir(dentry_t* __dentry)
{
	// ## not allowed for device file ##
	return 0;
}

dentry_t* devfs_mkdir(dentry_t* __parent, u8_t* __name)
{
	// ## not allowed for device file ##
	return 0;
}

dentry_t* devfs_mknode(dentry_t* __parent, u8_t* __name, u16_t __type)
{
	// ## not allowed for device file ##
	return 0;
}

dentry_t* devfs_rndir(dentry_t* __dentry, u8_t* __name)
{
	// ## call efs2_rndir ##
	return efs2_rndir(__dentry,__name);
}

int devfs_rmdir(dentry_t* __dentry, u8_t __option)
{
	// ## call efs2_rmdir ##
	return efs2_rmdir(__dentry,__option);
}

dentry_t* devfs_chmod(dentry_t* __dentry, u16_t __mode)
{
	// ## call efs2_chmod ##
	return efs2_chmod(__dentry,__mode);
}

dentry_t* devfs_link(dentry_t* __dentry, dentry_t* __parent, u8_t* __name)
{
	// ## not allowed for device file ##
	return 0;
}

int devfs_unlink(dentry_t* __dentry)
{
	// ## not allowed for device file ##
	return 0;
}

int devfs_write(dentry_t* __dentry, void* __buf, u32_t __size)
{
	// ## not allowed for device file ##
	return 0;	
}

int devfs_read(dentry_t* __dentry, void* __buf, u32_t __size)
{
	// ## not allowed for device file ##
	return 0;
}

int devfs_open(dentry_t* __dentry, u16_t __mode)
{
	// ## call efs2_open ##
	return efs2_open(__dentry,__mode);
}

int devfs_close(dentry_t* __dentry)
{
	// ## call efs2_close ##
	return efs2_close(__dentry);
}

int devfs_eof(dentry_t* __dentry)
{
	// ## not allowed for device file ##
	return 0;
}

int devfs_stat(dentry_t* __dentry, stat_t* __stat)
{
	// ## call efs2_stat ##
	return efs2_stat(__dentry,__stat);
}

int devfs_seek(dentry_t* __dentry, u32_t __offset, u32_t __ref)
{
	// ## not allowed for device file ##
	return 0;
}

int devfs_flush(dentry_t* __dentry)
{
	// ## not allowed for device file ##
	return 0;
}

int devfs_mount(dentry_t* __devdentry, dentry_t* __dentry, u16_t __mode)
{
	// ## not allowed for device file ##
	return 0;
}

int devfs_umount(dentry_t* __dentry)
{
	// ## not allowed for device file ##
	return 0;
}

char* devfs_readfile(dentry_t* __dentry)
{
	// ## not implemented ##
	return NULL;
}

void devfs_sync(dentry_t* __dentry)
{
	// ## call efs2_sync ##
	return efs2_sync(__dentry);
}

int devfs_mkfs(dentry_t* __dentry)
{
	if(!IS_DEVICE_FILE(__dentry->d_mode))
		return 0;
	__dentry->lock = devfs_lock;
	__dentry->unlock = devfs_unlock;
	__dentry->touch = devfs_touch;
	__dentry->lookup = devfs_lookup;
	__dentry->listdir = devfs_listdir;
	__dentry->mkdir = devfs_mkdir;
	__dentry->mknode = devfs_mknode;
	__dentry->rndir = devfs_rndir;
	__dentry->rmdir = devfs_rmdir;
	__dentry->chmod = devfs_chmod;
	__dentry->link = devfs_link;
	__dentry->unlink = devfs_unlink;
	__dentry->open = devfs_open;
	__dentry->close = devfs_close;
	__dentry->eof = devfs_eof;
	__dentry->stat = devfs_stat;
	__dentry->seek = devfs_seek;
	__dentry->flush = devfs_flush;
	__dentry->mount = devfs_mount;
	__dentry->umount = devfs_umount;
	__dentry->mkfs = devfs_mkfs;
	__dentry->readfile = devfs_readfile;
	__dentry->sync = devfs_sync;
	__dentry->write = devfs_write;
	__dentry->read = devfs_read;
	return 1;
}
