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

#ifndef __PCI_H__
#define __PCI_H__
#include <types.h>
#include <io.h>
#include <memory.h>
#include <lib.h>
#include <string.h>
#define TOTAL_PCI_DEVICE 256
#define PCI_CONFIG_ADDR 0xcf8
#define PCI_DATA_ADDR 0xcfc
#define VENDOR_ID_REG_OFF 0x00
#define DEVICE_ID_REG_OFF 0x02
#define COMMAND_REG_OFF 0x04
#define DEVICE_STATUS_REG_OFF 0x06
#define REVISION_ID_REG_OFF 0x08
#define PROGRAMING_REG_OFF 0x09
#define SUBCLASS_REG_OFF 0x0a
#define BASECLASS_REG_OFF 0x0b
#define MASTER_LATENCY_REG_OFF 0x0d
#define PRIMARY_CMD_TASK_REG_OFF 0x10
#define PRIMARY_CTRL_TASK_REG_OFF 0x14
#define SECONDARY_CMD_TASK_REG_OFF 0x18
#define SECONDARY_CTRL_TASK_REG_OFF 0x1c
#define BUS_MASTER_REG_OFF 0x20
#define INTERRUPT_LINE_REG_OFF 0x3c
#define INTERRUPT_PIN_REG_OFF 0x3d
#define PCI_FIND_BY_CLASS	0x00
#define PCI_FIND_BY_ID		0x01
#define PCI_DEV_NAME	0x40
typedef struct{
    u16_t bus;
    u16_t device;
    u16_t function;
    u8_t basecode;
    u8_t subcode;
    u32_t bmr;
    u16_t vendorID;
    u16_t deviceID;
    u8_t  name[PCI_DEV_NAME];
} __attribute__((packed)) pcidev_t;
/* functions */
u32_t r_pcidev(u32_t __bus, u32_t __dev, u32_t __function, u32_t __offset);
void w_pcidev(u32_t __bus, u32_t __dev, u32_t __function, u32_t __offset, u32_t __data);
void lookup_pcidev();
void name_pcidev(pcidev_t* __pcidev, u8_t* __name);
u32_t read_pcidev(pcidev_t* __pcidev, u8_t __offset);
void write_pcidev(pcidev_t* __pcidev, u8_t __offset, u32_t __data);
pcidev_t* find_pcidev(u8_t __subclass, u8_t __baseclass, u16_t __vendorID, u16_t __deviceID, u8_t __type);
#endif // __PCI_H__
