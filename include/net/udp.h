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
#ifndef __UDP_H__
#define __UDP_H__
#include <net/net.h>
typedef struct{
	u16_t sport;
	u16_t dport;
	u16_t length;
	u16_t checksum;
	u8_t payload[];
}__attribute__((packed)) udp_header_t;
u16_t udp_calculate_checksum(udp_header_t* __packet);
int udp_send_packet(ip_addr_t __dip, u16_t __sport, u16_t __dport, void* __payload, u32_t __len);
int udp_receive_packet(udp_header_t* __packet, u32_t __size);
#endif //__UDP_H__