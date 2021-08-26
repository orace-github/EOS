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

#ifndef __ATA_H__
#define __ATA_H__

#include <pci.h>
#include <x86/idt.h>
#include <io.h>
#include <memory.h>
#include <list.h>

#define ATA_PRIMARY_IO 0x1f0
#define ATA_SECONDARY_IO 0x170
#define ATA_PRIMARY_CTRL 0x3f6
#define ATA_SECONDARY_CTRL 0x376
#define ATA_PRIMARY_ISR 14
#define ATA_SECONDARY_ISR 15

#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// Directions:
#define ATA_DMA_READ 0x08
#define ATA_DMA_WRITE 0x00

// Bus master register offset
#define ATA_BMS_CR_1 0x0
#define ATA_BMS_SR_1 0x2
#define ATA_BMS_CR_2 0x08
#define ATA_BMS_SR_2 0x0a

#define ATA_BMS_PRD_R_1 0x04
#define ATA_BMS_PRD_R_2 0x0c

#define ATA_PCI_CR 0x09

#define READ 0x01
#define WRITE 0x02

typedef struct
{
    u16_t io_base;
    u16_t bus;
    u16_t drive;
    u16_t bmr;
    pcidev_t* pcidev;
    struct list_head head;
} __attribute__((packed)) ata_t;

typedef struct
{
    u32_t phy_addr;
    u16_t bytes;
    u16_t reserved;
} __attribute__((packed)) prd_t;

void ata_reset(u16_t __io);
ata_t* new_ata(u16_t __bus, u16_t __drive,
 u16_t __subcode, u16_t __basecode);
ata_t* find_ata(u16_t __bus, u16_t __drive);
void ata_delay();
void __isr14(registers_t* __regs);
void __isr15(registers_t* __regs);
u8_t ide_identify(u16_t __bus, u16_t __drive);
void ide_400ns_delay(u16_t __io);
void ide_poll(u16_t __io);
int ide_read_write(char* __buf, u32_t __lba, u8_t __read);
int disk_read(u32_t __lba, u32_t __offset, void* __buf, u32_t __bytes);
int disk_write(u32_t __lba, u32_t __offset, void* __buf, u32_t __bytes);
void ata_init();

#endif // __ATA_H__
