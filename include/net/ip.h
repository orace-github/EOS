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
#ifndef __IP_H__
#define __IP_H__
#include <net/net.h>
#define IP_IPv4	0x04
#define IP_IHL	0x05
#define IP_TTL	0x40
#define IP_PROTO_X	0x00
#define IP_PROTO_ICMP	0x01
#define IP_PROTO_IGMP	0x02
#define IP_PROTO_GGP	0x03
#define IP_PROTO_IP 	0x04 // (IP encapsulation)
#define IP_PROTO_STREAM	0x05
#define IP_PROTO_TCP	0x06
#define IP_PROTO_EGP	0x08
#define IP_PROTO_PIRP	0x09 // (Private Interior Routing Protocol)
#define IP_PROTO_UDP	0x11
#define IP_PROTO_IPv6	0x29
#define IP_PROTO_ESPIPv6	0x32
#define IP_PROTO_AHIPv6	0x33
#define IP_PROTO_OSPF	0x59
typedef struct{
	u8_t version_ihl_ptr[0]; // memory of (version & ihl)
	u8_t version:4; /* ip packet version */
	u8_t ihl:4;		/* internet header length */
	u8_t tos; 		/* type of service */
	#define IP_NORMAL_SERVICE		0x00
	#define IP_MINIMIZE_MONEY		0x01
	#define IP_MAXIMIZE_RELIABILITY	0x02
	#define IP_MAXIMIZE_THROUGHPUT	0x04
	#define IP_MINIMIZE_DELAY		0x08
	u16_t length; 	/* total length */
	u16_t id;		/* identification */
	u16_t flags_off_ptr[0]; // memory of (flags & off)
	u8_t flags:3;	/* DF + MF */
	#define IP_DF 	0x02 /* do not fragment */
	#define IP_MF	0x04 /* more fragments */
	u16_t off:13; 	/* fragment offset */
	u8_t ttl;		/* time to live */
	u8_t protocol;	/* protocol */
	u16_t checksum; /* checksum */
	ip_addr_t sip;	/* source address */
	ip_addr_t dip;	/* destination address */
	u8_t payload[];	/* payload */
}__attribute__((packed)) ip_header_t;

typedef struct{
	ip_addr_t ip;
	unsigned int subnet;
}__attribute__((packed)) ip_t;
/* starting byte position of data in IP packet p */
#define IP_DATA_START(p) ((ip_header_t*)p)->ihl * sizeof(u32_t)
/* length of options (in bytes) in IP packet p */
#define IP_OPTION_LEN(p) (IP_DATA_START(p) - sizeof(ip_header_t))
/* update a given ip type p with ip address a and subnet s */
#define IP(p,a,s) \
((ip_t*)p)->ip = a; \
((ip_t*)p)->subnet = s;
int ip_send_packet(ip_addr_t __ip, u8_t __protocol, void* __payload, u32_t __size);
int ip_receive_packet(ip_header_t* __packet, u32_t __size);
static inline void ip(ip_addr_t* __ip, unsigned int* __subnet, u8_t __mode);
static inline ip_addr_t inet_addr(u32_t __x0, u32_t __x1, u32_t __x2, u32_t __x3);
#endif //__IP_H__