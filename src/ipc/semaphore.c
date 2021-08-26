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


#include <ipc/semaphore.h>
#include <process.h>
static sem_entry_t __semaphore_table[MAX_SEMAPHORE * MAX_PROCESS] = {0};
extern process_t* current;

unsigned int semaphore_open(int __value, unsigned short __flags){
	if(__value <= 0)
		return 0;
	unsigned int base = current->id - PROCESS_DFLT_ID;
	for(int i = base; i < MAX_SEMAPHORE; i++){
		if(__semaphore_table[i].used)
			continue;
		__semaphore_table[i].semaphore.value = __value;
		__semaphore_table[i].semaphore.pid = current->id;
		__semaphore_table[i].semaphore.id = PROCESS_DFLT_ID + i;
		__semaphore_table[i].semaphore.flags = __flags;
		__semaphore_table[i].used = 1;
		current->semid[i-base] = __semaphore_table[i].semaphore.id;
		return __semaphore_table[i].semaphore.id;
	}
	return 0; // if no entry founded
}

sem_t* semaphore_lookup(unsigned int __id){
	unsigned int key = __id - PROCESS_DFLT_ID;
	return &__semaphore_table[key].semaphore;
}

void semaphore_wait(unsigned int __id){
	if(!__id) return;
	sem_t* __semaphore = semaphore_lookup(__id);
	/* wait until semaphore value is set to 0 */
	while(!__semaphore->value);
	__semaphore->value--;
}

void semaphore_signal(unsigned int __id){
	if(!__id) return;
	sem_t* __semaphore = semaphore_lookup(__id);
	__semaphore->value++;
}

void semaphore_close(unsigned int __id){
	if(!__id) return;
	unsigned int key = __id - PROCESS_DFLT_ID;
	__semaphore_table[key].used = 0;
}

void semaphore_update(unsigned int __id, int __value){
	if(__value <= 0)
		return;
	sem_t* __semaphore = semaphore_lookup(__id);
	if(IS_IPC_WRITE(__semaphore->flags))
		__semaphore->value = __value;
}