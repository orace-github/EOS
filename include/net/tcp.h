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
#ifndef __TCP_H__
#define __TCP_H__
#include <net/net.h>
#include <net/socket.h>
#define TH_FIN	0x01
#define TH_SYN	0x02
#define TH_RST 	0x04 /* you don't have to handle this */
#define TH_PUSH	0x08 /* ...or this */
#define TH_ACK 	0x10
#define TH_URG	0x20 /* ...or this */
#define TH_ECE	0x40
#define TH_CWR	0x80
#define TCP_WIN 	0x1000
#define TCP_MSS		0x536 // default value according to RFC

typedef struct{
	ip_addr_t sip; /* source ip */
	ip_addr_t dip; /* destination ip */
	u16_t protocol; /* TCP protocol: 0x06 */
	u16_t length; /* total length */
}__attribute__((packed)) tcp_pseudo_header_t;

typedef struct{
	u16_t th_sport; /* source port */
	u16_t th_dport; /* destination port */
	u32_t th_seq;	/* sequence number */
	u32_t th_ack; 	/* acknowledgement number */
	u8_t th_off:4;	/* data offset */
	#define TCP_OFF	0x05
	u8_t th_x2:4;	/* unused */
	u8_t th_flags;
	u16_t th_win;	/* windows */
	u16_t th_sum;	/* checksum */
	u16_t th_urp; 	/* urgent pointer */
	char payload[];	/* payload */
}__attribute__((packed)) tcp_header_t;

typedef struct{
	u32_t th_mss; /* maximum segment size */
}__attribute__((packed)) tcp_option_t;

enum{
	CLOSED,
	LISTEN,
	SYN_SENT,
	SYN_RECEIVED,
	ESTABLISHED,
	FIN_WAIT1,
	FIN_WAIT2,
	CLOSING,
	TIME_WAIT,
	CLOSE_WAIT,
	LAST_ACK,
};

typedef struct{
	u16_t tcp_sport; // src port
	u16_t tcp_dport; // dst port
	u32_t tcp_tx_segno; // transmission segment no
	u32_t tcp_rx_segno; // receiving segment no
	u32_t tcp_ack;	// acknowledge
	u16_t tcp_state; // tcp state
}__attribute__((packed)) tcp_ctrl_blk_t;

typedef struct{
	u32_t sequenceNumber;  // segment sequence number
	u32_t length;               // segment length
	u8_t acked;  // this segment has been ack (used for sender segment)
	u8_t fin;    // this is a fin segment
	u8_t* data;  // points to the data	
}__attribute__((packed)) segment_t;

/* starting byte position of data in TCP packet p*/
#define TCP_DATA_START(p) ((tcp_header_t*)p)->th_off * sizeof(u32_t)
/* length of options (in bytes) in TCP packet p */
#define TCP_OPTION_LEN(p) (TCP_DATA_START(p) - sizeof(tcp_header_t))
int tcp_send_packet(socket_t* __socket, void* __buf, int __size);
int tcp_receive_packet(tcp_pseudo_header_t* __packet, u32_t __size);
u16_t tcp_calculate_checksum(tcp_pseudo_header_t* __packet);
tcp_header_t* create_SYN_packet(unsigned int __seq, unsigned int __ack);
tcp_header_t* create_SYN_ACK_packet(unsigned int __seq, unsigned int __ack);
tcp_header_t* create_ACK_packet(unsigned int __seq, unsigned int __ack);
tcp_header_t* create_FIN_packet(unsigned int __seq, unsigned int __ack);
tcp_header_t* create_RST_packet(unsigned int __seq, unsigned int __ack);
int tcp_receive_SYN_packet(tcp_header_t* __packet, socket_t* __s);
int tcp_receive_SYN_ACK_packet(tcp_header_t* __packet, socket_t* __s);
int tcp_receive_ACK_packet(tcp_header_t* __packet, socket_t* __s);
int tcp_receive_FIN_packet(tcp_header_t* __packet, socket_t* __s);
int tcp_receive_RST_packet(tcp_header_t* __packet, socket_t* __s);
int tcp_receive_DATA_packet(tcp_header_t* __packet, socket_t* __s);
int tcp_send_SYN_packet(socket_t* __s);
int tcp_send_SYN_ACK_packet(socket_t* __s);
int tcp_send_ACK_packet(socket_t* __s);
int tcp_send_FIN_packet(socket_t* __s);
int tcp_send_RST_packet(socket_t* __s);
int tcp_send_DATA_packet(socket_t* __s, void* __buf, int __size);
int tcp_read(socket_t* __s, void* __buf, int __size);
int tcp_write(socket_t* __s, void* __buf, int __size);
int tcp_bind(socket_t* __s, const sockaddr_t* __addr, socklen_t __addrlen);
int tcp_listen(socket_t* __s, int __backlog);
int tcp_accept(socket_t* __s, sockaddr_t* __addr, socklen_t __addrlen);
int tcp_connect(socket_t* __s, const sockaddr_t* __addr, socklen_t __addrlen);
int tcp_close(socket_t* __s);
#endif //__TCP_H__