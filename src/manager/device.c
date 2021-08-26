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

#include <device.h>
#include <string.h>
#include <driver/rtc.h>
#include <driver/ata.h>
#include <driver/serial.h>
#include <driver/rtl8139.h>
// ## __dev_manger memory ##
dev_manager_t __dev_manager = {
	.free_dev = free_dev, 
	.alloc_dev = alloc_dev,
	.find_by_name = find_by_name,
	.find_by_dentry = find_by_dentry,
	.rename_dev = rename_dev,
};

void free_dev(dev_t* __dev, struct dev_manager* __dev_manager)
{
	// ## free __dev entry ##
	__dev->used = 0; __dev->read = NULL; __dev->write = NULL;
	__dev->dev_write = NULL; __dev->dev_read = NULL;
	__dev->initialize = NULL; __dev->reset = NULL;
	__dev_manager->devices--;
}

dev_t* alloc_dev(struct dev_manager* __dev_manager)
{
	// ## try to get a device ##
	if(__dev_manager->devices < MAX_DEV)
		for(int i = 0; i < MAX_DEV; i++)
			if(!__dev_manager->device[i].used)
			{
				__dev_manager->devices++;
				__dev_manager->device[i].used = 1;
				return &__dev_manager->device[i];
			}
		return NULL;
	return NULL;
}

dev_t* find_by_name(u8_t* __name, struct dev_manager* __dev_manager)
{
	// ## try to find the device with this name ##
	for(int i = 0; i < MAX_DEV; i++)
		if(!strcmp(__name,__dev_manager->device[i].dev_name))
			return &__dev_manager->device[i];
	return NULL;
}

dev_t* find_by_dentry(dentry_t* __dentry, struct dev_manager* __dev_manager)
{
	// ## try to find the device with this entry in VFS ##
	for(int i = 0; i < MAX_DEV; i++)
		if(__dev_manager->device[i].dev_dentry == __dentry)
			return &__dev_manager->device[i];
	return NULL;
}

void rename_dev(dev_t* __dev, u8_t* __name, struct dev_manager* __dev_manager)
{
	// ## Note that this function simply change device name in main memeory ##
	strcpy(__dev->dev_name,__name);
	// ##__dev_manager not used	
}

dev_manager_t* dev_manager()
{
	// ## return __devmanager memory location ##
	return &__dev_manager;
}

void init_devmanager()
{
	/* TODO */
	for(int i = 0; i < MAX_DEV; i++)
		__dev_manager.device[i].used = 0;
	INIT_MUTEX(&__dev_manager.mutex);
	 /* TODO */
	__dev_manager.mutex.mutex_lock();
	// ## RTC device ##
	dev_t* __rtc_dev = __dev_manager.alloc_dev(&__dev_manager);
	__rtc_dev->initialize = rtc_init;
	__rtc_dev->reset = NULL; __rtc_dev->write = NULL; __rtc_dev->read = NULL;
	__rtc_dev->dev_write = NULL; __rtc_dev->dev_read = NULL;
	__rtc_dev->activated = 1; strcpy(__rtc_dev->dev_name,"RTC");
	__rtc_dev->initialize();
	// ## ATA device ##
	dev_t* __ata_dev = __dev_manager.alloc_dev(&__dev_manager);
	__ata_dev->initialize = ata_init; __ata_dev->reset = NULL;
	__ata_dev->write = disk_write; __ata_dev->read = disk_read;
	__ata_dev->dev_read = NULL; __ata_dev->dev_write = NULL;
	__ata_dev->activated = 1; strcpy(__ata_dev->dev_name,"IDE/ATA");
	__ata_dev->dev_stat = FREE_DEVICE; __ata_dev->dev_type = BLOCK_DEVICE;
	__ata_dev->initialize();
	/* TODE */
}
