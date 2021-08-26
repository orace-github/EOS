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
#include <net/socket.h>
#include <time.h>
#include <process.h>
#include <net/tcp.h>
#include <net/udp.h>
static port_t __port[MAX_PORT];
static socket_t __socket[MAX_PORT];
extern process_t* current;
int newsocket(int __domain, int __type, int __protocol){

}

int socket_bind(int __socketfd, const sockaddr_t* __addr, socklen_t __addrlen){
	socket_t* __s = get_socket_context(__socketfd);
	if(!__s)
		return 0;
	/* call apropriate function */
	return __s->bind(__s,__addr,__addrlen);
}

int socket_listen(int __socketfd, int __backlog){
	socket_t* __s = get_socket_context(__socketfd);
	if(!__s)
		return 0;
	/* call apropriate function */
	return __s->listen(__s,__backlog);
}

int socket_accept(int __socketfd, sockaddr_t* __addr, socklen_t __addrlen){
	socket_t* __s = get_socket_context(__socketfd);
	if(!__s)
		return 0;
	/* call apropriate function */
	return __s->accept(__s,__addr,__addrlen);
}

int socket_connect(int __socketfd, const sockaddr_t* __addr, socklen_t __addrlen){
	socket_t* __s = get_socket_context(__socketfd);
	if(!__s)
		return 0;
	/* call apropriate function */
	return __s->connect(__s,__addr,__addrlen);	
}

int socket_read(int __socketfd, void* __buf, int __len){
	socket_t* __s = get_socket_context(__socketfd);
	if(!__s)
		return 0;
	/* call apropriate function */
	return __s->read(__s,__buf,__len);
}

int socket_write(int __socketfd, void* __buf, int __len){
	socket_t* __s = get_socket_context(__socketfd);
	if(!__s)
		return 0;
	/* call apropriate function */
	return __s->write(__s,__buf,__len);
}

int socket_close(int __socketfd){
	socket_t* __s = get_socket_context(__socketfd);
	if(!__s)
		return 0;
	/* call apropriate function */
	return __s->close(__s);
}

socket_t* get_socket_context(unsigned int __sockfd){
	/* get socket context bind to current process */
	return current->socket[__sockfd]; // highly depend on current process
}

void set_socket_context(unsigned int __sockfd, socket_t* __s){
	/* set socket context to current process */
	current->socket[__sockfd] = __s; // highly depend on current process
}

socket_t* get_port_socket(unsigned short __portNo){
	return __port[__portNo].socket;
}

void set_port_socket(unsigned short __portNo, socket_t* __s){
	__port[__portNo].socket = __s;
}

void map_all_port(void){
	// map all port
	for(int i = 0; i <= 1024; i++){
		__port[i].portNo = i; /* port no */
		__port[i].rsvport = 1; /* reserved port mask */
		__port[i].used = 0; /* set not used */
	}

	for(int i = 1025; i < 60000; i++){
		__port[i].portNo = i; /* port no */
		__port[i].appport = 1; /* application port mask */
		__port[i].used = 0; /* set not used */
	}

	for(int i = 60001; i < MAX_PORT; i++){
		__port[i].portNo = i; /* port no */
		__port[i].otherport = 1; /* other port mask */
		__port[i].used = 0; /* set not used */
	}
}

u16_t generate_port(u8_t __type){
	// generate a port number
	unsigned short port = 0;
	srand(0); // initiate seed ot 0
	switch(__type){
		case RSV_PORT:
			// generate reserved port randomly
			for(;;){
				port = rand() % 1025;
				if(__port[port].used)
					continue;
				__port[port].used = 1;
					return port;
			}
			break;
		case APP_PORT:
			// generate app port randomly 
			for(;;){
				port = rand() % (60000 - 1025);
				if(__port[1025 + port].used)
					continue;
				__port[1025 + port].used = 1;
					return port;
			}
			break;
		case OTHER_PORT:
			// generate other port randomly
			for(;;){
				port = rand() % (MAX_PORT - 60001);
				if(__port[60001 + port].used)
					continue;
				__port[60001 + port].used = 1;
					return port;
			}
			break;
	}
}

void release_port(u16_t __portNo){
	if(__portNo < 0 || __portNo >= MAX_PORT)
		return;
	// reset portNo mask used
	__port[__portNo].used = 0;
}

void hold_port(u16_t __portNo){
	if(__portNo < 0 || __portNo >= MAX_PORT)
		return;
	// set portNo mask used
	__port[__portNo].used = 1;
}
