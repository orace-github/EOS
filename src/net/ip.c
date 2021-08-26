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
#include <net/ip.h>
#include <net/arp.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <net/ethernet.h>
#include <fs/fs.h>
#include <memory.h>
static ip_t __localip;
static ip_t __loopbackip;

int ip_send_packet(ip_addr_t __dip, u8_t __protocol, void* __payload, u32_t __size){
	ip_header_t* __packet = sys_alloc(sizeof(ip_header_t) + __size);
	__packet->version = IP_IPv4;
	__packet->ihl = IP_IHL;
	__packet->tos = IP_NORMAL_SERVICE;
	__packet->length = sizeof(ip_header_t) + __size;
	__packet->id = 0x00; // only used when fragmentation is set
	__packet->flags = IP_DF; // do not fragment packet
	__packet->off = 0x00;
	__packet->ttl = IP_TTL;
	__packet->protocol = __protocol;
	__packet->sip = __localip.ip;
	__packet->dip = __dip;
	/* locate ip packet date memory location */
	void* __data = (void*)__packet + IP_DATA_START(__packet);
	memcpy(__data,__payload,__size);
	/* Fix packet data order */
	*((u8_t*)(&__packet->version_ihl_ptr)) = htonb(*((u8_t*)(&__packet->version_ihl_ptr)),4);
	*((u8_t*)(&__packet->flags_off_ptr)) = htonb(*((u8_t*)(&__packet->flags_off_ptr)),3);
	__packet->length = htons(__packet->length);
	/* Make sure that checksum is set to 0 before calculate it */
	__packet->checksum = 0;
	__packet->checksum = htons(checksum(__packet,sizeof(ip_header_t)));
	/* mac address */
	mac_t __dmac;
	/* We have to call arp protocol to resolve destination mac address */
	int __bytes = ethernet_send_packet(&__dmac,ETHERNET_TYPE_IP,__packet,ntohs(__packet->length));
	if(__bytes){
		sys_free(__packet); // free memory
		return __bytes;
	}
	return 0;
}

int ip_receive_packet(ip_header_t* __packet, u32_t __size){
	// Fix packet data order
	*((u8_t*)(&__packet->version_ihl_ptr)) = ntohb(*((u8_t*)(&__packet->version_ihl_ptr)),4);
	*((u8_t*)(&__packet->flags_off_ptr)) = ntohb(*((u8_t*)(&__packet->flags_off_ptr)),3);
	__packet->length = ntohs(__packet->length);
	
	if(__packet->version == IP_IPv4){
		u32_t __datalen = __packet->length - sizeof(ip_header_t);
		void* __data = (void*)__packet + IP_DATA_START(__packet);
		if(__packet->protocol == IP_PROTO_UDP){
			/* handle udp packet */
			udp_header_t* __udppacket = (udp_header_t*)__data;
			return udp_receive_packet(__udppacket,__datalen);
		}
		if(__packet->protocol == IP_PROTO_TCP){
			/* handle tcp packet */
			tcp_pseudo_header_t* __tcppacket = (tcp_pseudo_header_t*)__data;
			return tcp_receive_packet(__tcppacket,__datalen);
		}	
		if(__packet->protocol == IP_PROTO_ICMP){
			/* handle icmp packet */
		}
	}
	/* we only handle ipv4, tcp, udp, icmp */
	return 0;
}

void ip(ip_addr_t *__ip, unsigned int* __subnet, u8_t __mode){
	if(IS_WR_O(__mode)){
		IP(&__localip,*__ip,*__subnet); // change __localip 
	}
	else if(IS_RD_O(__mode)){
		*__ip = __localip.ip; *__subnet = __localip.subnet;
	}
}

ip_addr_t inet_addr(u32_t __x0, u32_t __x1, u32_t __x2, u32_t __x3){
	return ((__x3 << 24)|(__x2 << 16)|(__x1 << 8)|__x0);
}
