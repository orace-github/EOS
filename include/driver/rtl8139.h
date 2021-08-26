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

#ifndef __RTL8139_H__
#define __RTL8139_H__
#include <device.h>
#include <types.h>
#include <fs/devfs.h>
#include <memory.h>
#include <x86/idt.h>
#include <net/ethernet.h>
#define RTL8139_VENDOR_ID	0x10EC
#define RTL8139_DEVICE_ID	0x8139
// ## transmit descriptor
typedef struct
{
	unsigned long tx_addr;
	struct
	{
		unsigned size	: 13;// ## packet size RW ##
		unsigned own	: 1; // ## set to 0 after size is written to start operation RD_O RW## 
		unsigned tun	: 1; // transmit underrun occurs: set to 1 RD_O ##
		unsigned tok 	: 1; // ## transmit ok(completed): set to 1  RD_O ##
		unsigned ertxth	: 6; // 
		unsigned 		: 2; // ## reserved ##
		unsigned ncc	: 4; // ## number of collision count RD_O ##
		unsigned cdh	: 1; // ## CD heart mode: cleared in 100 Mbps mode RD_0 ##
		unsigned owc	: 1; // ## out of windows collision RD_O ##
		unsigned tabt	: 1; // ## transmit abort RD_O ##
		unsigned crs 	: 1; // ## carrier sense lost : RD_O ##
	} tx_status;
} __attribute__((packed)) tx_desc_t;
// ## rtl8139 device structure ##
typedef struct
{
	u8_t bar_type;
	u16_t io_base;
	u32_t mem_base;
	mac_t mac;
	int eeprom_exist;
	volatile u8_t* rx_buf;
	u16_t tx_cur;
	u16_t rx_cur;
}__attribute__((packed)) rtl8139_dev_t;
void rtl8139_handler(void);
int rtl8139_receive(void);
int rtl8139_transmit(mac_t* __dmac, u32_t __type, u8_t* __data, u32_t __len);
void rtl8139_init(void);
void rtl8139_mac(mac_t* __mac, u8_t __mode);
#endif //__RTL8139_H__