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

#ifndef __DEVICE_H__
#define __DEVICE_H__
#include <types.h>
#include <list.h>
#include <fs/fs.h>
#include <mutex.h>
#include <error.h>
#define MAX_DEV	1000
#define FREE_DEVICE   0x01
#define BUSY_DEVICE   0x02
#define DIRTY_DEVICE  0x04
#define UNKNOW_DEVICE 0x01
#define CHAR_DEVICE   0x02
#define BLOCK_DEVICE  0x04

#define IS_FREE_DEVICE(dev)  ((dev)->dev_stat & FREE_DEVICE)
#define IS_BUSY_DEVICE(dev)  ((dev)->dev_stat & BUSY_DEVICE)
#define IS_DIRTY_DEVICE(dev) ((dev)->dev_stat & DIRTY_DEVICE)
#define IS_CHAR_DEVICE(dev)  ((dev)->dev_type & CHAR_DEVICE)
#define IS_BLOCK_DEVICE(dev) ((dev)->dev_type & BLOCK_DEVICE)
#define IS_UNKNOW_DEVICE(dev) ((dev)->dev_type & UNKNOW_DEVICE)

typedef struct device
{
	// read - write drivers
	int (*read)(u32_t __pos, u32_t __off, void* __buf, u32_t __bytes);
	int (*write)(u32_t __pos, u32_t __off, void* __buf, u32_t __bytes);
	int (*dev_write)(struct device* __dev, void* __buf, u32_t __bytes);
	int (*dev_read)(struct device* __dev, void* __buf, u32_t __bytes);
	partition_t* dev_part; // device partition
	u32_t dev_id; // device id
	u8_t dev_name[DNAME_MAX_LEN]; // device name
	u8_t dev_path[DNAME_MAX_LEN]; // device path
	u8_t* dev_fsname; // device file system name
	u32_t dev_stat; // device stat
	u32_t dev_type; // device type
	dentry_t* dev_dentry; // device mount dentry
	void* dev_data;
	void (*initialize)(void);
	void (*reset)(void);
	u8_t activated;
	u8_t used;
} __attribute__((packed)) dev_t;

typedef struct dev_manager
{
	void (*free_dev)(dev_t* __dev, struct dev_manager* __dev_manager);
	dev_t* (*alloc_dev)(struct dev_manager* __dev_manager);
	dev_t* (*find_by_name)(u8_t* __name, struct dev_manager* __dev_manager);
	dev_t* (*find_by_dentry)(dentry_t* __dentry, struct dev_manager* __dev_manager);
	void (*rename_dev)(dev_t* __dev, u8_t* __name, struct dev_manager* __dev_manager);
	dev_t device[MAX_DEV];
	int devices;
	struct mutex mutex;
}__attribute__((packed)) dev_manager_t;

void free_dev(dev_t* __dev, struct dev_manager* __dev_manager);
dev_t* alloc_dev(struct dev_manager* __dev_manager);
dev_t* find_by_name(u8_t* __name, struct dev_manager* __dev_manager);
dev_t* find_by_dentry(dentry_t* __dentry, struct dev_manager* __dev_manager);
void rename_dev(dev_t* __dev, u8_t* __name, struct dev_manager* __dev_manager);
dev_manager_t* dev_manager();
void init_devmanager();
#endif //__DEVICE_H__