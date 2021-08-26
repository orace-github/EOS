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
#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <net/net.h>
typedef int socklen_t;
typedef unsigned int sa_family_t;

/* Socket Address */
typedef struct{
	sa_family_t sa_family; // family address
	char sa_data[14];
}__attribute__((packed)) sockaddr_t;

/* Internet address */
typedef struct{
	ip_addr_t s_addr; // socket address in htonl
}__attribute__((packed)) in_addr_t;

typedef struct{
	sa_family_t sin_family; // family address
	unsigned short sin_port;	// port number in htons
	in_addr_t sin_addr; // internet address
}__attribute__((packed)) sockaddr_in_t;

struct socket;
typedef struct{
	u16_t portNo;
	#define MAX_PORT	0xffff
	u8_t used : 1; /* port No is allocate to process */
	u8_t rsvport : 1; /* reserved port <= 1024 */
	u8_t appport : 1; /* application port > 1024 && < 60000 */
	u8_t otherport: 1; /* the rest */
	u16_t unused : 12;
	#define RSV_PORT 0x01
	#define APP_PORT 0x02
	#define OTHER_PORT	0x04 
	struct socket* socket;
}__attribute__((packed)) port_t;
void map_all_port(void);
u16_t generate_port(u8_t __type);
void release_port(u16_t __portNo);
void hold_port(u16_t __portNo);
#define MAX_SOCKET	0x40
typedef struct socket{
	u16_t visible; /* socket is active */
	u32_t sockpid; /* socket process id */
	u16_t portNo; /* port number */
	ip_addr_t sockip; /* socket ip address */
	u8_t sockfamily; /* socket family */
	#define AF_INET		0x04 // ipv4
	#define AF_INET6	0x06 // ipv6 : Hint :: not implemented yet
	u8_t sockproto; /* socket transport protocol */
	#define SOCK_STREAM 	0x01 // TCP
	#define SOCK_DGRAM 		0x02 // UDP
	#define SOCK_RAW 		0x04 // other
	unsigned int io_lock; // semaphore for exclusive access
	int (*write)(struct socket* __s, void* __buffer, int __size);
	int (*read)(struct socket* __s, void* __buffer, int __size);
	int (*bind)(struct socket* __s, const sockaddr_t* __a, unsigned int __size);
	int (*listen)(struct socket* __s, int __backlog);
	int (*accept)(struct socket* __s, sockaddr_t* __addr, unsigned int __size);
	int (*connect)(struct socket* __s, const sockaddr_t* __a, unsigned int __size);
	int (*close)(struct socket* __s);
	void* pcb; /* protocol control block */
}__attribute__((packed)) socket_t;
/* socket: __protocol is set to 0 if socket type is udp or tcp */
socket_t* get_socket_context(unsigned int __sokfd);
void set_socket_context(unsigned int __sockfd, socket_t* __s);
socket_t* get_port_socket(unsigned short __portNo);
void set_port_socket(unsigned short __portNo, socket_t* __s);
int newsocket(int __domain, int __type, int __protocol);
int socket_bind(int __socketfd, const sockaddr_t* __addr, socklen_t __addrlen);
int socket_listen(int __socketfd, int __backlog);
int socket_accept(int __socketfd, sockaddr_t* __addr, socklen_t __addrlen);
int socket_connect(int __socketfd, const sockaddr_t* __addr, socklen_t __addrlen);
int socket_read(int __socketfd, void* __buf, int __len);
int socket_write(int __socketfd, void* __buf, int __len);
int socket_close(int __socketfd);
#endif //__SOCKET_H__