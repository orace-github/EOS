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
#ifndef __NIC_H__
#define __NIC_H__
#include <types.h>
#include <net/net.h>
typedef struct nic{
	u8_t mode; // enable or desable
	#define NIC_ENABLE	0x01
	#define NIC_DESABLE	0x02
	mac_t mac;
	void (*read_mac)(mac_t* __mac, struct nic* __nic);
	void (*write_mac)(mac_t* __mac, struct nic* __nic);
	u8_t* tx_buffer;
	u8_t* rx_buffer;
	int (*send_data)(void* __data, u32_t __len, struct nic* __nic);
	int (*receive_data)(void* __data, u32_t __len, struct nic* __nic);
}__attribute__((packed)) nic_t;
/* set a given network interface card n the mode v */
#define NIC_MODE(n,v) ((nic_t*)n)->mode = v
#define IS_NIC_ENABLE(n) (((nic_t*)n)->mode & NIC_ENABLE)
#define IS_NIC_DESABLE(n) (((nic_t*)n)->mode & NIC_DESABLE)
#endif //__NIC_H__