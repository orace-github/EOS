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
#ifndef __ETHERNET_H__
#define __ETHERNET_H__
#include <net/net.h>
#define ETHERNET_TYPE_ARP	0x0806
#define ETHERNET_TYPE_IP	0x0800
typedef struct {
	mac_t dmac;
	mac_t smac;
	u16_t type;
	u8_t payload[];
}__attribute__((packed)) ethernet_header_t;
int ethernet_receive_packet(ethernet_header_t* __packet);
int ethernet_send_packet(mac_t* __dmac, u16_t __type, void* __payload, u32_t __len);
#endif //__ETHERNET_H__