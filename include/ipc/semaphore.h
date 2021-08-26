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

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__
#include <types.h>
#include <ipc/ipc.h>
#define MAX_SEMAPHORE	0x1E
typedef struct{
	int value;	 				// semaphore value define by user process
	unsigned int  pid;			// creator process id
	unsigned short flags;		// semaphore flags
	unsigned int id;			// semaphore id
}__attribute__((packed)) sem_t;

typedef struct{
	sem_t semaphore; 	// semaphore 
	unsigned short used;
}__attribute__((packed)) sem_entry_t;
unsigned int semaphore_open(int __value, unsigned short __flags);
sem_t* semaphore_lookup(unsigned int __id);
void semaphore_wait(unsigned int __id);
void semaphore_signal(unsigned int __id);
void semaphore_close(unsigned int __id);
void semaphore_update(unsigned int __id, int __value);
#endif //__SEMAPHORE_H__