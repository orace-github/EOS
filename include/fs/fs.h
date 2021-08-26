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

#ifndef __FS_H__
#define __FS_H__
#include <list.h>
#include <time.h>
#include <error.h>
#define PATH_MAX_LEN   0x200
#define DNAME_MAX_LEN  0x40
#define READ_O         0x0001
#define WRITE_O        0x0002
#define READ_WRITE     0x0003
#define APPEND_O	   0x0004
#define EXEC_O		   0x0008
#define ORDINARY_FILE  0x0100
#define DIRECTORY_FILE 0x0200
#define PIPE_FILE      0x0400
#define SOCKET_FILE	   0x0800 
#define DEVICE_FILE    0x1000
#define LINK_FILE	   0x2000
#define DRIGHT_MASK	   0x00ff
#define DTYPE_MASK	   0xff00
#define LOCKED_O 	   0x0100
#define MAX_FILE	   0x1E

#define IS_LOCKED_O(lock) (lock & LOCKED_O)
#define IS_WR_O(mode) (mode & WRITE_O)
#define IS_RD_O(mode) (mode & READ_O)
#define IS_RD_WR(mode) (mode & READ_WRITE)
#define IS_APPEND_O(mode) (mode & APPEND_O)
#define IS_EXEC_O(mode)	(mode & EXEC_O)
#define IS_ORDINARY_FILE(mode) (mode & ORDINARY_FILE)
#define IS_DIRECTORY_FILE(mode) (mode & DIRECTORY_FILE)
#define IS_PIPE_FILE(mode) (mode & PIPE_FILE)
#define IS_SOCKET_FILE(mode) (mode & SOCKET_FILE)
#define IS_DEVICE_FILE(mode) (mode & DEVICE_FILE)
#define IS_LINK_FILE(mode) (mode & LINK_FILE)

#define SEEK_SET 0x0
#define SEEK_CUR 0x1
#define SEEK_END 0x2

typedef struct{
	u8_t bootable;		/* 0 = no, 0x80 = bootable */
	u8_t s_head;		/* Starting head */
	u16_t s_sector:6;	/* Starting sector */
	u16_t s_cyl:10;		/* Starting cylinder */
	u8_t id;			/* System ID */
	u8_t e_head;		/* Ending Head */
	u16_t e_sector:6;	/* Ending Sector */
	u16_t e_cyl:10;		/* Ending Cylinder */
	u32_t s_lba;		/* Starting LBA value */
	u32_t size;			/* Total Sectors in partition */
} __attribute__ ((packed)) partition_t;

typedef struct{
	u32_t st_dev;
	u32_t st_inode;
	u8_t st_mode;
	u32_t st_link;
	u32_t st_size;
	u32_t st_blksize;
	time_t st_ctime; // created time
	time_t st_atime; // last acess time
} __attribute__((packed)) stat_t;

typedef struct dentry{
	u8_t d_name[DNAME_MAX_LEN];
	void* d_fs;
	u16_t d_mode; // right + type
	u16_t d_fsmode; // used only when mounted is set
	u16_t d_lock;
	u8_t opened;
	u8_t mounted;
	u8_t d_hide;
	u32_t d_inode;
	u64_t d_offset;
	u64_t d_size;
	time_t ctime;
	time_t atime;
	time_t mtime;
	struct dentry* parent;
	struct list d_childs;
	struct list_head d_head;
	struct list_head d_chead; // dcache head
	struct list_head d_hhead; // hash table head
	struct list_head d_fhead; // open file head
	
	int (*lock)(struct dentry* __dentry, u8_t __flags);
	int (*unlock)(struct dentry* __dentry);
	int (*touch)(struct dentry* __parent, u8_t* __name);
	int (*write)(struct dentry* __dentry, void* __buf, u32_t __size);
	int (*read)(struct dentry* __dentry, void* __buf, u32_t __size);
	struct dentry* (*lookup)(struct dentry* __dentry, u8_t* __name);
	struct dentry* (*mkdir)(struct dentry* __parent, u8_t* __name);
	struct dentry* (*mknode)(struct dentry* __parent, u8_t* __name, u16_t __type);
	struct dentry* (*rndir)(struct dentry* __dentry, u8_t* __name);
	int (*rmdir)(struct dentry* __dentry, u8_t __option);
	struct dentry* (*chmod)(struct dentry* __dentry, u16_t __mode);
	struct dentry* (*link)(struct dentry* __dentry, struct dentry* __parent, u8_t* __name);
	int (*unlink)(struct dentry* __dentry);
	int (*open)(struct dentry* __dentry, u16_t __mode);
	int (*close)(struct dentry* __dentry);
	int (*eof)(struct dentry* __dentry);
	int (*stat)(struct dentry* __dentry, stat_t* __stat);
	int (*seek)(struct dentry* __dentry, u32_t __offset, u32_t __ref);
	int (*flush)(struct dentry* __dentry);
	int (*mount)(struct dentry* __devdentry, struct dentry* __dentry,
	 u16_t __mode);
	int (*umount)(struct dentry* __dentry);
	int (*mkfs)(struct dentry* __dentry);
	char* (*readfile)(struct dentry* __dentry);
	void (*sync)(struct dentry* __dentry);
	int (*listdir)(struct dentry* __dentry);
} __attribute__((packed)) dentry_t; // kernel space direntry

typedef struct dir{
	u8_t name[DNAME_MAX_LEN];
	u8_t mode;
	u8_t fsmode;
	u8_t opened;
	u8_t mounted;	
	u8_t hide;
	u8_t fd;
	u64_t offset;
	u64_t size;
	u32_t uid;
	u32_t gid;
	time_t ctime;
	time_t stime;
	time_t mtime;
	struct dir* parent;
	struct list_head head;
	struct list childs;
} __attribute__((packed)) dir_t; // user space direntry

#define DENTRY_TO_DIR(dentry,dir)	\
	strncpy((dir)->name,(dentry)->d_name,DNAME_MAX_LEN);	\
	(dir)->mode = (dentry)->d_mode;	\
	(dir)->opened = (dentry)->opened;	\
	(dir)->mounted = (dentry)->mounted;	\
	(dir)->hide = (dentry)->d_hide;	\
	(dir)->fd = (dentry)->d_fd;	\
	(dir)->offset = (dentry)->d_offset;	\
	(dir)->size = (dentry)->d_size;	\
	(dir)->uid = (dentry)->uid;	\
	(dir)->gid = (dentry)->gid;	\
	(dir)->ctime = (dentry)->ctime;	\
	(dir)->stime = (dentry)->stime;	\
	(dir)->mtime = (dentry)->mtime;	\
	(dir)->fsmode = (dentry)->d_fsmode;


typedef struct{
	u32_t inode; // inode number
} __attribute__((packed)) dirent_t;

#endif //__FS_H__