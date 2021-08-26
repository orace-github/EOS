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
#include <net/tcp.h>
#include <memory.h>
#include <time.h>

int tcp_receive_packet(tcp_pseudo_header_t* __packet, u32_t __size){
	return 0;
}

tcp_header_t* create_SYN_packet(unsigned int __seq, unsigned int __ack){
	tcp_header_t* __packet = sys_alloc(sizeof(tcp_header_t));
	__packet->th_seq = htonl(__seq);
	__packet->th_ack = htonl(__ack);
	__packet->th_off = htons(TCP_OFF);
	__packet->th_flags = TH_SYN;
	__packet->th_win = htons(TCP_WIN);
	return __packet;
}

tcp_header_t* create_SYN_ACK_packet(unsigned int __seq, unsigned int __ack){
	tcp_header_t* __packet = sys_alloc(sizeof(tcp_header_t));
	__packet->th_seq = htonl(__seq);
	__packet->th_ack = htonl(__ack);
	__packet->th_off = htons(TCP_OFF);
	__packet->th_flags = TH_SYN | TH_ACK;
	__packet->th_win = htons(TCP_WIN);
	return __packet;
}

tcp_header_t* create_ACK_packet(unsigned int __seq, unsigned int __ack){
	tcp_header_t* __packet = sys_alloc(sizeof(tcp_header_t));
	__packet->th_seq = htonl(__seq);
	__packet->th_ack = htonl(__ack);
	__packet->th_off = htons(TCP_OFF);
	__packet->th_flags = TH_ACK;
	__packet->th_win = htons(TCP_WIN);
	return __packet;
}

tcp_header_t* create_FIN_packet(unsigned int __seq, unsigned int __ack){
	tcp_header_t* __packet = sys_alloc(sizeof(tcp_header_t));
	__packet->th_seq = htonl(__seq);
	__packet->th_ack = htonl(__ack);
	__packet->th_off = htons(TCP_OFF);
	__packet->th_flags = TH_FIN;
	__packet->th_win = htons(TCP_WIN);
	return __packet;
}

tcp_header_t* create_RST_packet(unsigned int __seq, unsigned int __ack){
	tcp_header_t* __packet = sys_alloc(sizeof(tcp_header_t));
	__packet->th_seq = htonl(__seq);
	__packet->th_ack = htonl(__ack);
	__packet->th_off = htons(TCP_OFF);
	__packet->th_flags = TH_RST;
	__packet->th_win = htons(TCP_WIN);
	return __packet;
}

int tcp_receive_SYN_packet(tcp_header_t* __packet, socket_t* __s){

}

int tcp_receive_SYN_ACK_packet(tcp_header_t* __packet, socket_t* __s){

}

int tcp_receive_ACK_packet(tcp_header_t* __packet, socket_t* __s){

}

int tcp_receive_FIN_packet(tcp_header_t* __packet, socket_t* __s){

}

int tcp_receive_RST_packet(tcp_header_t* __packet, socket_t* __s){

}

int tcp_receive_DATA_packet(tcp_header_t* __packet, socket_t* __s){

}

int tcp_send_SYN_packet(socket_t* __s){

}

int tcp_send_SYN_ACK_packet(socket_t* __s){

}

int tcp_send_ACK_packet(socket_t* __s){

}

int tcp_send_FIN_packet(socket_t* __s){

}

int tcp_send_RST_packet(socket_t* __s){

}

int tcp_send_DATA_packet(socket_t* __s, void* __buf, int __size){

}

int tcp_read(socket_t* __s, void* __buf, int __size){

}

int tcp_write(socket_t* __s, void* __buf, int __size){

}

int tcp_bind(socket_t* __s, const sockaddr_t* __addr, socklen_t __addrlen){

}

int tcp_listen(socket_t* __s, int __backlog){

}

int tcp_accept(socket_t* __s, sockaddr_t* __addr, socklen_t __addrlen){

}

int tcp_connect(socket_t* __s, const sockaddr_t* __addr, socklen_t __addrlen){

}

int tcp_close(socket_t* __s){

}