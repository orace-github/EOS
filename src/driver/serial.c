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
#include <driver/serial.h>

void serial_init(void)
{
	#ifdef __DEBUG__
		kprintf("Installing serial driver...");
	#endif //__DEBUG__
	outb(PORT_COM1 + 1, 0x00);
   	outb(PORT_COM1 + 3, 0x80);
   	outb(PORT_COM1 + 0, 0x03);
   	outb(PORT_COM1 + 1, 0x00);
   	outb(PORT_COM1 + 3, 0x03);
   	outb(PORT_COM1 + 2, 0xC7);
   	outb(PORT_COM1 + 4, 0x0B);
   	#ifdef __DEBUG__
		kprintf("[ok]\n");
	#endif //__DEBUG__
}

u8_t serial_received(void)
{
	return inb(PORT_COM1 + 5) & 1;
}

u8_t serial_read_byte()
{
	while(!serial_received());
	return inb(PORT_COM1);
}

u8_t serial_transmit(void)
{
	inb(PORT_COM1 + 5) & 0x20;
}

void serial_write_byte(u8_t __byte)
{
	while(!serial_transmit());
	outb(PORT_COM1,__byte);
}

int serial_dev_write(dev_t* __dev, void* __buf, u32_t __bytes)
{
	u8_t* __data = (u8_t*)__buf;
	for(int i = 0; i < __bytes; i++)
		serial_write_byte(__data[i]);
}

int serial_dev_read(dev_t* __dev, void* __buf, u32_t __bytes)
{
	u8_t* __data = (u8_t*)__buf;
	for(int i = 0; i < __bytes; i++)
		__data[i] = serial_read_byte();
}

int serial_write(dentry_t* __dentry, void* __buf, u32_t __bytes)
{
	// ## check __dentry mode ##
	if(!IS_WR_O(__dentry->d_mode) || !IS_DEVICE_FILE(__dentry->d_mode))
		return 0;
	// ## get device ##
	dev_manager_t* __devmanager = dev_manager();
	dev_t* __dev = __devmanager->find_by_dentry(__dentry,__devmanager);
	if(!__dev || !__dev->dev_write)
		return serial_dev_write(__dev,__buf,__bytes);
	return __dev->dev_write(__dev,__buf,__bytes);
}

int serial_read(dentry_t* __dentry, void* __buf, u32_t __bytes)
{
	// ## check __dentry mode ##
	if(!IS_RD_O(__dentry->d_mode) || !IS_DEVICE_FILE(__dentry->d_mode))
		return 0;
	// ## get device ##
	dev_manager_t* __devmanager = dev_manager();
	dev_t* __dev = __devmanager->find_by_dentry(__dentry,__devmanager);
	if(!__dev || !__dev->dev_read)
		return serial_dev_read(__dev,__buf,__bytes);
	return __dev->dev_read(__dev,__buf,__bytes);
}
