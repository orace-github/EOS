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

#ifndef __SERIAL_H__
#define __SERIAL_H__
#include <types.h>
#include <device.h>
#include <io.h>
#define PORT_COM1	0x3f8
void serial_init(void);
u8_t serial_received(void);
u8_t serial_read_byte();
u8_t serial_transmit(void);
void serial_write_byte(u8_t __byte);
int serial_dev_write(dev_t* __dev, void* __buf, u32_t __bytes);
int serial_dev_read(dev_t* __dev, void* __buf, u32_t __bytes);
int serial_write(dentry_t* __dentry, void* __buf, u32_t __bytes);
int serial_read(dentry_t* __dentry, void* __buf, u32_t __bytes);
#endif //__SERIAL_H__