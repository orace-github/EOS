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


#include <process.h>
#include <fs/vfs.h>
// current process
process_t* current = NULL;
// kernel default process
process_t* __shell = NULL;
// kernel process hashtable
process_hashtable_t __processhashtable;
// kernel scheduler
schedule_t __scheduler;
// process table
static process_t __process[MAX_PROCESS] __attribute__((section(".data")));
// ## kernel task state segment ##
extern tss_t __tss;
extern int vfs_open(u8_t* __path, u16_t __mode);
extern int vfs_write(int __fd, void* __buf, u32_t __bytes);
extern int vfs_read(int __fd, void* __buf, u32_t __bytes);
extern int vfs_exist(u8_t* __path);
extern int vfs_mount(u8_t* __devpath, u8_t* __fpath, u16_t __mode);
extern int vfs_umount(u8_t* __path);
extern int vfs_mkdir(u8_t* __path);
extern int vfs_rndir(u8_t* __path, u8_t* __name);
extern int vfs_rmdir(u8_t* __path, u8_t __option);
extern int vfs_chdir(u8_t* __path);
extern int vfs_chmod(u8_t* __path, u16_t __mode);
extern int vfs_close(int __fd);
extern int vfs_eof(int __fd);
extern int vfs_link(u8_t* __fpath, u8_t* __lpath);
extern int vfs_unlink(u8_t* __lpath);
extern int vfs_touch(u8_t* __path);
extern int vfs_seek(u32_t __fd, u32_t __off, u32_t __pos);
extern int vfs_flush(u32_t __fd);
extern char* vfs_readfile(u8_t* __path);
extern dentry_t* vfs_lookup(u8_t* __path);
extern void vfs_sync();

void process_t_insert(process_t* __process, struct process_hashtable* __hashtable){
	int __key = __process->id % PROCESS_HASH_MAX;
	list_insert(&__hashtable->proc_list[__key],&__process->hhead);
}

void process_t_remove(process_t* __process, struct process_hashtable* __hashtable){
	int __key = __process->id % PROCESS_HASH_MAX;
	list_remove(&__hashtable->proc_list[__key],&__process->hhead);
}

process_t* process_t_find(u32_t __id, struct process_hashtable* __hashtable){
	int __key = __process->id % PROCESS_HASH_MAX;
	struct list_head* it = NULL;
	list_for_each(it,&__hashtable->proc_list[__key].head){
		process_t* __proc = list_entry(it,process_t,hhead);
		if(__proc->id == __id)
			return __proc;
	}
	return NULL;
}

process_t* process_lookup(unsigned int __pid){
	return __processhashtable.find(__pid,&__processhashtable);
}

void init_hashtable(){
	if(__processhashtable.init == 0xff)
		return;
	for(int i = 0; i < PROCESS_HASH_MAX; i++)
		init_list(&__processhashtable.proc_list[i]);
	__processhashtable.insert = process_t_insert;
	__processhashtable.remove = process_t_remove;
	__processhashtable.find = process_t_find;
	__processhashtable.init = 0xff;
}

void unmap_all(process_t* __proc){
	// .text + rodata
	unmap_frame(__proc->__text);
	// .data
	unmap_frame(__proc->__bss);
	// stack
	unmap_frame(__proc->__stack);
	// heap
	struct list_head* it = NULL;
	list_for_each(it,&__proc->heap_list.head){
		heap_frame_t* __heap_frame = list_entry(it,heap_frame_t,head);
		unmap_frame(__heap_frame->frame);
	}
	// shared memory
	list_for_each(it,&__proc->shm_list.head){
		shm_entry_t* __shm_entry = list_entry(it,shm_entry_t,head);
		unmap_frame(__shm_entry->shm->__frame);
	}
}

void map_all(process_t* __proc){
	// .text + rodata
	map_frame(__proc->__text,PAGE_PRESENT|PAGE_USER_SUPERUSER);
	// .data
	map_frame(__proc->__bss,PAGE_PRESENT|PAGE_READ_WRITE|PAGE_USER_SUPERUSER);
	// stack
	map_frame(__proc->__stack,PAGE_PRESENT|PAGE_READ_WRITE|PAGE_USER_SUPERUSER);
	// heap
	struct list_head* it = NULL;
	list_for_each(it,&__proc->heap_list.head){
		heap_frame_t* __heap_frame = list_entry(it,heap_frame_t,head);
		map_frame(__heap_frame->frame,PAGE_PRESENT|PAGE_READ_WRITE|PAGE_USER_SUPERUSER);
	}

	// shared memory
	list_for_each(it,&__proc->shm_list.head){
		shm_entry_t* __shm_entry = list_entry(it,shm_entry_t,head);
		map_frame(__shm_entry->shm->__frame,__shm_entry->shm->__flags);
	}	
}

process_t* process(){
	// init process hashtable
	init_hashtable();
	// find an unused process entry
	// into process table
	for(int i = 0; i < MAX_PROCESS; i++)
		if(!__process[i].used){
			// make process entry used
			__process[i].used = 1;
			// assign an id to process
			__process[i].id = PROCESS_DFLT_ID + i*MAX_FILE;
			// init list
			init_list(&__process[i].child_list);
			init_list(&__process[i].shm_list);
			init_list(&__process[i].sem_list);
			init_list(&__process[i].heap_list);
			// clear all descriptor (file,semaphore,socket)
			memset32(__process[i].fd,-1,MAX_FILE);
			memset32(__process[i].semid,0,MAX_SEMAPHORE);
			memset(__process[i].socket,0,MAX_SOCKET*sizeof(socket_t*));
			// add to process hashtable
			__processhashtable.insert(&__process[i],&__processhashtable);
			// return process memory location
			return &__process[i];
		}
	return NULL;
}

int process_sbrk(){
	// grow up heap size of page size bytes
	frame_t* __frame = alloc_frame(current->id,1);
	// alloc heap frame entry
	heap_frame_t* __heap_frame = sys_alloc(sizeof(heap_frame_t));
	// check if both of memory location are safe
	if(!__frame || !__heap_frame || (u32_t)current->__e_heap >= (USER_STACK - USER_STACK_SIZE)){
		free_frame(current->id,__frame);
		sys_free(__heap_frame);
		return 0;
	}
	// setting heap frame
	__frame->v_addr = (u32_t)current->__e_heap;
	// update heap end memory location
	current->__e_heap += PAGE_SIZE;
	__heap_frame->frame = __frame;
	// add heap entry to process heap list
	list_insert(&current->heap_list,&__heap_frame->head);
	// now map frame as PRESENT USER/SUPER RW
	map_frame(__frame,PAGE_PRESENT|PAGE_USER_SUPERUSER|PAGE_READ_WRITE);
	// return 1
	return 1;
}

char* process_bheap(){
	// return base address of heap
	return current->__b_heap;
}

char* process_eheap(){
	// return end address of heap
	return current->__e_heap;
}

int process_fork(){
	// child process memory location
	process_t* __proc = process();
	// make sure child process entry is safe
	if(!__proc || current->regs.cs == KERNEL_CS)
		return -1;
	// copy current registers state 
	// into child process registers
	__proc->regs = current->regs;
	__proc->regs.eax = 0;
	__proc->kstack.esp0 = (u32_t)(__proc->__kstack + PAGE_SIZE);
	__proc->kstack.ss0 = KERNEL_SS;
	current->regs.eax = current->id;
	// copy all descriptor (file,semaphore,socket)
	memcpy(__proc->fd,current->fd,MAX_FILE*sizeof(int));
	memcpy(__proc->semid,current->semid,sizeof(int)*MAX_SEMAPHORE);
	memcpy(__proc->socket,current->socket,sizeof(socket_t*)*MAX_SOCKET);
	// realtime
	__proc->realtime = 0;
	// name
	strncpy(__proc->name,current->name,DNAME_MAX_LEN);
	// current working directory
	strncpy(__proc->cwd,current->cwd,PATH_MAX_LEN);
	// signal and sigset
	for(int i = 0; i < NUM_SIG; i++)
		__proc->signal[i] = current->signal[i];
	__proc->sigset = 0;
	// parent
	__proc->parent = current;
	list_insert(&current->child_list,&__proc->head);
	// state and status
	__proc->state = current->state;
	__proc->status = current->status;
	// add it to running process list
	list_insert(&__scheduler.running_list,&__proc->sched_head);
	__scheduler.running++;
	// shm - sem - heap
	__proc->shm_list = current->shm_list;
	__proc->sem_list = current->sem_list;
	__proc->heap_list = current->heap_list;
	// heap bound
	__proc->__b_heap = current->__b_heap;
	__proc->__e_heap = current->__e_heap;
	// memory frame
	__proc->__text = current->__text;
	__proc->__bss = current->__bss;
	__proc->__stack = current->__stack;
	// local time
	local_time(&__proc->time);
	__proc->clock = time_clock();
	// switch to child process
	unmap_all(current);
	switch_process(__proc);
}

int process_wait(int* __status){
	if(list_empty(&current->child_list.head))
		return current->id;
	struct list_head* it = NULL;
	while(1){
		// process find zombie process among his childs
		list_for_each(it,&current->child_list.head){
			process_t* __proc = list_entry(it,process_t,head);
			if(IS_ZOMBIE(__proc)){
				// get child's status
				if(__status)
					*__status = __proc->status;
				// remove child from current childs
				list_remove(&current->child_list,&__proc->head);
				// free child
				__proc->used = 0;
				return __proc->id;
			}
		}
	}	
	// avoid GCC warnning
	return current->id;
}

int process_waitpid(int __pid){
	if(current->id == __pid)
		return current->id;
	// lookup process with this __pid
	process_t* __proc = __processhashtable.find(__pid,&__processhashtable);
	if(!__proc)
		return current->id;
	process_t* __parent = __proc->parent;
	while(1){
		if(IS_ZOMBIE(__proc)){
			// remove child from current childs
			list_remove(&__parent->child_list,&__proc->head);
			// get child id
			int __pid = __proc->id;
			// free child
			__proc->used = 0;
			return __proc->id;
		}
	}
	// avoid GCC warnning
	return current->id;
}

unsigned int process_pid() {
	return current->id;
}

unsigned int process_ppid(){
	return current->parent->id;
}

int process_exit(int __status){
	if(current == __shell)
		return 0;
	// wait all child process before dead
	while(!list_empty(&current->child_list.head))
		process_wait(NULL);
	struct list_head* __it = NULL;
	struct list_head* __next = NULL;
	// remove process hashtable entry
	__processhashtable.remove(current,&__processhashtable);
	// unmap all process page
	unmap_frame(current->__text);
	unmap_frame(current->__bss);
	unmap_frame(current->__stack);
	// free all process page
	free_frame(current->id,current->__text);
	free_frame(current->id,current->__bss);
	free_frame(current->id,current->__stack);
	// free all process file descriptor
	for(int i = 0; i < MAX_FILE; i++)
		if(current->fd[i] != -1)
			vfs_close(current->fd[i]);
	// free schedule entry
	list_remove(&__scheduler.running_list,&current->sched_head);
	__scheduler.running--;
	// free shared memory
	list_for_each_safe(__it,__next,&current->shm_list.head){
		shm_entry_t* __shm_entry = list_entry(__it,shm_entry_t,head);
		// removing from list
		list_remove(&current->shm_list,&__shm_entry->head);
		// unmap page hardwired to shared memory
		unmap_frame(__shm_entry->shm->__frame);
		// if process create shared memory then remove it
		if(free_shm(current->id,__shm_entry->shm))
			sys_free(__shm_entry); 
	}
	// free semaphore
	for(int i = 0; i < MAX_SEMAPHORE; i++){
		semaphore_close(current->semid[i]);
	}
	// free heap frame
	list_for_each_safe(__it,__next,&current->heap_list.head){
		heap_frame_t* __heap_frame = list_entry(__it,heap_frame_t,head);
		// remove from list
		list_remove(&current->heap_list,&__heap_frame->head);
		// unmap frame
		unmap_frame(__heap_frame->frame);
		// if process create heap frame then remove it
		if(free_frame(current->id,__heap_frame->frame))
			sys_free(__heap_frame);	
	}
	current->state = ZOMBIE;
	current->status = __status;
	// switch to __shell
	switch_process(__shell);
	// avoid GCC warnning
	return 1;
}

void process_sleep(int __sec){
	if(__sec < 0 || current == __shell)
		return;
	list_remove(&__scheduler.running_list,&current->sched_head);
	__scheduler.running--;
	// add to sleeping process
	list_insert(&__scheduler.sleeping_list,&current->sched_head); 
	__scheduler.sleeping++;
	// get local time
	local_time(&current->wake_up);
	// computing wake up time
	time_computing(&current->wake_up,__sec);
	// put process state to sleep
	current->state = SLEEP;
	// switch to __shell
	switch_process(__shell);
}

void process_alarm(int __sec){
	// make sure that second is great than 0 
	if(__sec <= 0)
		return;
	// alarm structure
	alarm_t __alarm;	
	// set rtc's pid
	// to current pid
	__alarm.pid = current->id;
	// now get local time 
	local_time(&__alarm.time);
	// compute alarm time
	time_computing(&__alarm.time,__sec);
	// now setting rtc alarm register
	alarm(&__alarm);
}

unsigned int process_clock(){
	return time_clock() - current->clock;
}

int load_elf(process_t* __proc, u8_t* __buf, int __argc, char** __argv)
{
	// checking for elf file header
	if(!is_elf_x86(__buf))
		return 0;
	// copy program arguments inside kernel memory
	char** __kparam = NULL;
	if(__argc){
		__kparam = sys_alloc(sizeof(char*)*(__argc+1));
		for(int i = 0; i < __argc; i++){
			__kparam[i] = sys_alloc(strlen(__argv[i]) + 1);
			strcpy(__kparam[i],__argv[i]);
		}
		__kparam[__argc] = NULL;
	}

	u32_t __elf_size = 0;
	elf_header_t* __elf_header = (elf_header_t*)__buf;

	for(int i = 0; i < __elf_header->e_phnum; i++){
		elf_program_header_t* __elf_program_header = elf_ph(__buf,i);
		// checking for a loadable program header
		if(__elf_program_header->p_type == PT_LOAD){
			if(__elf_program_header->p_flags == PF_X + PF_R){
				/* .text + .rodata program header */
				// checking for program address and size
				if(__elf_program_header->p_vaddr != USER_OFFSET || 
					(__elf_program_header->p_vaddr + __elf_program_header->p_memsz) >= (USER_STACK - USER_STACK_SIZE))
					return 0;
				// computing needing page for program
				int __page = __elf_program_header->p_memsz / PAGE_SIZE;
				if(__elf_program_header->p_memsz % PAGE_SIZE)
					__page++;
				__elf_size += __page*PAGE_SIZE;
				// allocating physical page for program
				__proc->__text = alloc_frame(__proc->id,__page);
				if(!__proc->__text)
					return 0;
				__proc->__text->v_addr = USER_OFFSET;	
				// mapping physical page with flags (PRESENT + USER/SUPER)
				map_frame(__proc->__text,PAGE_PRESENT|PAGE_USER_SUPERUSER);				
				// copy program code at USER_OFFSET
				memcpy((void*)USER_OFFSET,__buf+__elf_program_header->p_offset,
					__elf_program_header->p_filesz);
				// fill with 0 if program memsize is greatter than filesize
				if(__elf_program_header->p_memsz > __elf_program_header->p_filesz){
					u8_t* __vaddr = (u8_t*)USER_OFFSET;
					for(int i = __elf_program_header->p_filesz; i < __elf_program_header->p_memsz;
						i++)
						__vaddr[i] = 0;
				}

			}
			else if(__elf_program_header->p_flags == PF_R + PF_W){
				/* .bss program header */				
				// computing needing page for program
				int __page = __elf_program_header->p_memsz / PAGE_SIZE;
				if(__elf_program_header->p_memsz % PAGE_SIZE)
					__page++;
				__elf_size += __page*PAGE_SIZE;
				// allocating physical page for program
				__proc->__bss = alloc_frame(__proc->id,__page);
				if(!__proc->__bss){
					unmap_frame(__proc->__text);
					free_frame(__proc->id,__proc->__text);
					return 0;
				}
				__proc->__bss->v_addr = __elf_program_header->p_vaddr;
				// mapping physical page with flags (PRESENT + USER/SUPER + RW)
				map_frame(__proc->__bss,PAGE_PRESENT|PAGE_USER_SUPERUSER|PAGE_READ_WRITE);
				// copy program code at USER_OFFSET
				memcpy((void*)__elf_program_header->p_vaddr,__buf+__elf_program_header->p_offset,__elf_program_header->p_filesz);
				// fill with 0 if program memsize is greatter than filesize
				if(__elf_program_header->p_memsz > __elf_program_header->p_filesz){
					u8_t* __vaddr = (u8_t*)__elf_program_header->p_vaddr;
					for(int i = __elf_program_header->p_filesz; i < __elf_program_header->p_memsz;
						i++)
						__vaddr[i] = 0;
				}				

			}
		}
	}
	// setting process kernel's stack registers
	__proc->kstack.esp0 = (u32_t)(__proc->__kstack + PAGE_SIZE);
	__proc->kstack.ss0 = KERNEL_SS;
	// allocating process's stack
	__proc->__stack = alloc_frame(__proc->id,USER_STACK_SIZE/PAGE_SIZE);
	if(!__proc->__stack){
		unmap_frame(__proc->__text);
		free_frame(__proc->id,__proc->__text);
		unmap_frame(__proc->__bss);
		free_frame(__proc->id,__proc->__bss);
		for(int i = 0; i <= __argc; i++)
			sys_free(__kparam[i]);
		sys_free(__kparam);
		return 0;
	}
	__proc->__stack->v_addr = USER_STACK - USER_STACK_SIZE;
	// mapping physical page with flags (PRESENT + USER/SUPER + RW)
	map_frame(__proc->__stack,PAGE_PRESENT|PAGE_READ_WRITE|PAGE_USER_SUPERUSER);
	// make READY process state
	__proc->state = READY;
	// fill process working directory
	if(__proc != __shell)
		strncpy(__proc->cwd,__proc->parent->cwd,PATH_MAX_LEN);
	// fill process file descriptor with -1
	// required by vfs
	for(int i = 0; i < MAX_FILE; i++)
		__proc->fd[i] = -1;
	// setting process registers
	__proc->regs.eax = 0;
	__proc->regs.ebx = 0;
	__proc->regs.ecx = 0;
	__proc->regs.edx = 0;
	__proc->regs.esi = 0;
	__proc->regs.edi = 0;
	__proc->regs.eip = USER_OFFSET;
	__proc->regs.eflags = 0x200;
	__proc->regs.cs = USER_CS;
	__proc->regs.ss = USER_SS;
	__proc->regs.ds = USER_DS;
	__proc->regs.es = USER_ES;
	__proc->regs.gs = USER_GS;
	__proc->regs.fs = USER_FS;
	// copy program parameters on stack
	u32_t __stackup = USER_STACK - 16;
	char** __uparam = NULL;
	if(__argc){
		__uparam = sys_alloc(sizeof(char*)*__argc);
		for(int i = 0; i < __argc; i++){
			__stackup -= (strlen(__kparam[i]) + 1);
			strcpy((char*)__stackup,__kparam[i]);
			__uparam[i] = (char*)__stackup;
		}
		// align on 16 bytes
		__stackup &= 0xfffffff0;
		// main() argument creating
		__stackup -= sizeof(char*);
		*((char**)__stackup) = 0;
		// __argv[0] to __argv[n]
		for(int i = __argc - 1; i >= 0; i--){
			__stackup -= sizeof(char*);
			*((char**)__stackup) = __uparam[i];
		}
		// __argv
		__stackup -= sizeof(char*);
		*((char**)__stackup) = (char*)(__stackup + sizeof(char*));		
		// __argc
		__stackup -= sizeof(char*);
		*((int*)__stackup) = __argc;
		__stackup -= sizeof(char*);		
		for(int i = 0; i <= __argc; i++)
			sys_free(__kparam[i]);
		sys_free(__kparam);
		sys_free(__uparam);
	}
	__proc->regs.esp = __stackup;
	__proc->regs.ebp = __stackup;
	// setting process heap memory
	__proc->__b_heap = NULL;
	__proc->__e_heap = NULL;
	frame_t* __hframe = alloc_frame(__proc->id,1);

	if((USER_OFFSET + PAGE_SIZE + __elf_size) < USER_STACK && __hframe){
		heap_frame_t* __heap_frame = sys_alloc(sizeof(heap_frame_t));
		__heap_frame->frame = __hframe;
		__hframe->v_addr = USER_OFFSET + __elf_size;
		// mapping heap memory with flags (PRESENT + USER/SUPER + RW)
		map_frame(__hframe,PAGE_PRESENT|PAGE_USER_SUPERUSER|PAGE_READ_WRITE);
		list_insert(&__proc->heap_list,&__heap_frame->head);
		__proc->__b_heap = (char*)(USER_OFFSET+__elf_size);
		__proc->__e_heap = __proc->__b_heap + PAGE_SIZE;
	}
	// setting process signal bitmap
	__proc->sigset = 0x0;
	// setiing process signal action 
	// no writable this SIGDEF + IPC_READ
	set_sigaction(&__proc->signal[SIGKILL-1],SIGDEF,IPC_READ);
	set_sigaction(&__proc->signal[SIGABRT-1],SIGDEF,IPC_READ);
	set_sigaction(&__proc->signal[SIGSEGV-1],SIGDEF,IPC_READ);
	set_sigaction(&__proc->signal[SIGFPE-1],SIGDEF,IPC_READ);
	set_sigaction(&__proc->signal[SIGPGE-1],SIGDEF,IPC_READ);
	set_sigaction(&__proc->signal[SIGCHLD-1],SIGDEF,IPC_READ);
	set_sigaction(&__proc->signal[SIGWKP-1],SIGDEF,IPC_READ);
	// setting process signal action writable with SIGIGN + IPC_READ + IPC_WRITE
	set_sigaction(&__proc->signal[SIGINT-1],SIGDEF,IPC_READ|IPC_WRITE);
	set_sigaction(&__proc->signal[SIGSTOP-1],SIGDEF,IPC_READ|IPC_WRITE);
	set_sigaction(&__proc->signal[SIGBUS-1],SIGDEF,IPC_READ|IPC_WRITE);
	set_sigaction(&__proc->signal[SIGALARM-1],SIGIGN,IPC_READ|IPC_WRITE);
	set_sigaction(&__proc->signal[SIGUSR1-1],SIGIGN,IPC_READ|IPC_WRITE);
	set_sigaction(&__proc->signal[SIGUSR2-1],SIGIGN,IPC_READ|IPC_WRITE);
	// set realtime to 0
	__proc->realtime = 0;
	// set mutex to 0
	__proc->mutex = 0;
	// flush standard IO
	vfs_flush(0);
	vfs_flush(1);
	vfs_flush(2);
	// init list
	init_list(&__proc->child_list);
	init_list(&__proc->shm_list);
	init_list(&__proc->sem_list);
	init_list(&__proc->heap_list);
	// get time that process has been created
	local_time(&__proc->time);
	__proc->clock = time_clock();
	return 1;
}

int process_exec(process_t* __proc, u8_t* __path, int __argc, char** __argv){
	// check if process is shell
	if(__proc == __shell)
		return 0;
	// find vfs dentry
	dentry_t* __dentry = vfs_lookup(__path);
	if(!__dentry)
		return ENOENT; /* No such file or directory */
	if(!IS_EXEC_O(__dentry->d_mode))
		return ENOEXEC; /* exec format error */
	// copy dentry name in process context
	strncpy(__proc->name,__dentry->d_name,DNAME_MAX_LEN);
	// read file
	u8_t* __buf = vfs_readfile(__path);
	// check if file reading successful
	if(!__buf)
		return 0;
	// clear interrupt
	struct mutex __mutex;
	INIT_MUTEX(&__mutex);
	__mutex.mutex_lock();
	// load elf file
	if(!load_elf(__proc,__buf,__argc,__argv)){
		__mutex.mutex_unlock();// desable interrupt
		// free file memory buffer
		sys_free(__buf);
		return 0;
	}
	// free file memory buffer
	sys_free(__buf);
	// switch on process
	switch_process(__proc);
	// avoid GCC warnning
	return 1;
}

void process_cwd(u8_t* __cwd){
	strncpy(__cwd,current->cwd,PATH_MAX_LEN);
}

void process_pause(){
	if(current == __shell)
		return;
	// make process state to stop
	current->state = STOPED;
	// remove current process from running process
	list_remove(&__scheduler.running_list,&current->sched_head);
	__scheduler.running--;
	//add process to stoped process list
	list_insert(&__scheduler.stoped_list,&current->sched_head);
	__scheduler.stoped++;
	// switch to __shell
	switch_process(__shell);
}

int process_wakeup(){
	struct list_head* __it = NULL;
	// wakeup a child process 
	list_for_each(__it,&current->child_list.head){
		// find a child process
		process_t* __proc = list_entry(__it,process_t,head);
		if(IS_STOPED(__proc)){
			// remove it from stoped process
			list_remove(&__scheduler.stoped_list,&__proc->sched_head);
			__scheduler.stoped--;
			// insert in running process
			list_insert(&__scheduler.running_list,&__proc->sched_head);
			__scheduler.running++;
			__proc->state = READY;
			return __proc->id;
		}
	}
	return current->id;
}

int process_wakeupid(int __pid){
	// find process with __pid in hashtable
	process_t* __proc = __processhashtable.find(__pid,&__processhashtable);
	if(!__proc || !IS_STOPED(__proc))
		return current->id;
	// remove it from stoped process
	list_remove(&__scheduler.stoped_list,&__proc->sched_head);
	__scheduler.stoped--;
	// insert in running process
	list_insert(&__scheduler.running_list,&__proc->sched_head);
	__scheduler.running++;
	__proc->state = READY;
	return __pid;
}

int process_io(u8_t* __input, u8_t* __output, u8_t* __errput){
	// ## change process std IO ##
	if(__input){
		// ## INPUT ##
		dentry_t* __dentry = vfs_lookup(__input);
		if(!__dentry)
			return 0;
		current->input = __dentry;
	}
	if(__output){
		// ## OUTPUT ##
		dentry_t* __dentry = vfs_lookup(__output);
		if(!__dentry)
			return 0;
		current->output = __dentry;
	}
	if(__errput){
		// ## ERRPUT ##
		dentry_t* __dentry = vfs_lookup(__errput);
		if(!__dentry)
			return 0;
		current->errput = __dentry;
	}
	return 1;
}

void process_hold(){
	// set process mutex to 1
	current->mutex = 1;
}

void process_release(){
	// set process mutex to 0
	current->mutex = 0;
}

void sleeping_process(){
	// check if sleeping process list is empty
	if(!__scheduler.sleeping)
		return;
	// current time
	time_t __time;
	local_time(&__time);
	// go through sleeping process list
	struct list_head* __it = NULL;
	struct list_head* __next = NULL;
	list_for_each_safe(__it,__next,&__scheduler.sleeping_list.head){
		process_t* __proc = list_entry(__it,process_t,sched_head);
		// compare wake up of process to current time
		int __test = memcmp(&__time,&__proc->wake_up,sizeof(time_t));
		// check if wake up time is great or equal to current time
		if(!__test || __test == 1){
			// remove process from sleeping process list
			list_remove(&__scheduler.sleeping_list,&__proc->sched_head);
			__scheduler.sleeping--;
			// make process state to READY and then add to running process list
			__proc->state = READY;
			list_insert(&__scheduler.running_list,&__proc->sched_head);
			__scheduler.running++;
		}
	}
}

void process_kill(int __pid, int __signum){
	// send signal to process with __pid
	process_t* __proc = __processhashtable.find(__pid,&__processhashtable);
	if(!__proc || __signum < 1 || __signum > NUM_SIG)
		return;
	// release process mutex if it was set
	if(__proc->mutex)
		__proc->mutex = 0;
	// set signal bit in bitmap
	SET_SIGNAL(__proc->sigset,__signum);
}

void process_signal(int __signum, handler_t __handler){
	// change current process __signum handler
	if(__signum < 1 || __signum > NUM_SIG)
		return;
	sigaction(&current->signal[__signum-1],NULL,__handler);
}

void process_sigaction(int __pid, int __signum, sigaction_t* __sigaction, sigaction_t* __restorer){
	// change process with __pid __signum handler
	process_t* __proc = __processhashtable.find(__pid,&__processhashtable);
	if(!__proc || __signum < 1 || __signum > NUM_SIG)
		return;
	// checking for writable signal flag
	if(!IS_IPC_WRITE(__proc->signal[__signum-1].__flags))
		return;
	// saving process __signum sigaction before update it
	if(__restorer)
		memcpy(__restorer,&__proc->signal[__signum-1],sizeof(sigaction_t));
	// update process __signum sigaction
	if(__sigaction)
		__proc->signal[__signum-1].__handler = __sigaction->__handler;
}

void signal_handler(){
	int __signum = signal_dequeue(current->sigset);
	// checking signal number 
	if(!__signum)
		return;
	// clear signal
	CLEAR_SIGNAL(current->sigset,__signum);
	// get signal handler
	handler_t __handler = current->signal[__signum-1].__handler;
	if(__handler == SIGIGN)
		return;
	u32_t* __esp = NULL;
	if(__handler == SIGDEF){
		switch(__signum){
			// (KILL ABRT INT) signal kill with
			// status 0 
			case SIGKILL: case SIGABRT: case SIGINT:
				process_exit(0);
				break;
			// (FPE PGE SEGV) exception signal kill
			// with status -1
			case SIGFPE: case SIGPGE: case SIGSEGV:
				process_exit(-1);
				break;
			// CHLD signal not implemented
			// because process_wait() is locked 
			// system call. this may dangerous
			// for IRQ_0 (scheduler)
			case SIGCHLD:
				break;
			// STOP signal stop current
			// process and then switch
			// to shell
			case SIGSTOP:
				process_pause();
				break;
			// WKP signal wake up one 
			// current child process
			// stoped by process_pause()
			// system call	
			case SIGWKP:
				process_wakeup();
				break;
			// nothing is done for
			// other signal with
			// SIGDEF
			default:
				break;
		}
	}
	else{
		// make sure that signal handler
		// will be executing in user mode
		if(current->regs.cs == KERNEL_CS){
			SET_SIGNAL(current->sigset,__signum);
			return;
		}
		// save current registers on user stack
		__esp = (u32_t*)current->regs.esp - 21;
		// binary code which call sigreturn
		// mov eax,0
		// int 0x80
		__esp[20] = 0x0080cd00;
		__esp[19] = 0x000000b8;
		__esp[18] = current->regs.eax;
		__esp[17] = current->regs.ebx;
		__esp[16] = current->regs.ecx;
		__esp[15] = current->regs.edx;
		__esp[14] = current->regs.ebp;
		__esp[13] = current->regs.esp;
		__esp[12] = current->regs.esi;
		__esp[11] = current->regs.edi;
		__esp[10] = current->regs.eip;
		__esp[9] = current->regs.eflags;
		__esp[8] = current->regs.cs;
		__esp[7] = current->regs.ss;
		__esp[6] = current->regs.ds;
		__esp[5] = current->regs.es;
		__esp[4] = current->regs.gs;
		__esp[3] = current->regs.fs;
		__esp[2] = current->kstack.esp0;
		// signal handler default
		// parameter and return address
		__esp[1] = __signum;
		__esp[0] = (u32_t)&__esp[20];
		// replace eip and esp registers
		// of current process to jump to
		// signal handler
		current->regs.eip = (u32_t)__handler;
		current->regs.esp = (u32_t)__esp;
	}
}

void switch_process(process_t* __proc){
	// update current process
	current = __proc;
	current->state = RUNNING;
	// if is shell process map it. other process are mapped by scheduler
	if(current == __shell)
		map_all(__shell);
	// call signal handler
	signal_handler();
	// setting tss
	__tss.esp0 = (u32_t)(current->__kstack + PAGE_SIZE);
	__tss.ss0 = KERNEL_SS;
	u16_t __cs = current->regs.cs;
	u16_t __ss = current->regs.ss;
	// jump to process context
	__asm__ __volatile__("cli");
	__asm__ __volatile__("mov %0,%%ax"::"m"(current->kstack.ss0));
	__asm__ __volatile__("mov %ax,%ss");
	__asm__ __volatile__("mov %0,%%esp"::"m"(current->kstack.esp0):"memory");	
	__asm__ __volatile__("cmp %[KMODE],%[mode];"
						"je next;"
						"push %0;"
						"push %1;"
						"next:"
						"push %2;"
						"push %3;"
						"push %4;"
						"push %5;"
						"ljmp $0x08,$do_switch;"
						::
						"m"(__ss),
						"m"(current->regs.esp),
						"m"(current->regs.eflags),
						"m"(__cs),
						"m"(current->regs.eip),
						"m"(current),
						[KMODE] "i"(KERNEL_CS),
						[mode] "g"(current->regs.cs)
						);
}

void __isr0(registers_t* __regs){
	// copy registers
	memcpy(&current->regs,__regs,sizeof(registers_t));
	// realtime
	current->realtime = process_clock();
	// wake up sleeping process
	sleeping_process();
	// check if interrupt occurs in
	// user mode
	if(current->regs.cs != USER_CS){
		current->regs.esp = (u32_t)(&__regs->esp);
		current->regs.ss = KERNEL_SS;
		current->kstack.esp0 = current->regs.esp;
	}
	else{
		current->kstack.esp0 = (u32_t)(current->__kstack + PAGE_SIZE);
		// new kernel stack
	}
	// check if running is one or
	// whether current's mutex field 
	// is define
	if(__scheduler.running == 1 || !__scheduler.running || current->mutex)
		return;
	// make process state to ready
	current->state = READY;
	// select next process
	struct list_head* next = NULL;
	process_t* __proc = NULL;
	next = current->sched_head.next;
	// if end of list go back to __shell process
	if(!next){
		__proc = __shell;
		unmap_all(current);
		switch_process(__proc);
	}
	else
		__proc = list_entry(next,process_t,sched_head);
	// unmap current process
	unmap_all(current);
	// map next process
	map_all(__proc);	
	// swicth to next process
	switch_process(__proc);
}

char* shm_create(int __extern_id, int __vaddr, int __size, char __flags){
	// create a shared memory
	shm_t* __shm = alloc_shm(__extern_id,__size,__flags,current->id);
	if(!__shm)
		return NULL;

	// check if __vaddr is inside shared memory bound
	if(__vaddr < SHM_OFFSET || __vaddr >= SHM_UP){
		// free shared memory
		free_shm(current->id,__shm);
		return NULL;
	}
	// check if current process really create shared memory
	if(__shm->__frame->pid == current->id){
		// shared memory entry
		shm_entry_t* __shm_entry = sys_alloc(sizeof(shm_entry_t));
		// check if shared memory entry
		// memory location is safe
		if(!__shm_entry){
			free_shm(current->id,__shm);
			return NULL;
		}		
		__shm_entry->shm = __shm;
		// add it to process shared
		// memory list	
		list_insert(&current->shm_list,&__shm_entry->head);
	}
	// find shared memory mapping flags
	u8_t __shmflags = PAGE_PRESENT | PAGE_USER_SUPERUSER;
	if(IS_IPC_WRITE(__flags))
		__shmflags |= PAGE_READ_WRITE;
	// align on 4kb
	__vaddr &= ~0xfff;
	// map shared memory
	__shm->__frame->v_addr = __vaddr;
	map_frame(__shm->__frame,__shmflags);
	// return shared memory location
	return (char*)__vaddr;
}

void shm_release(int __extern_id){
	// unmap shared memory
	unmap_shm(__extern_id);	
}

void init_schedule(void){
	// init process hashtable
	init_hashtable();
	__shell = process();
	current = __shell;
	// __scheduler
	__scheduler.running = 0;
	__scheduler.sleeping = 0;
	__scheduler.stoped = 0;
	__processhashtable.insert(__shell,&__processhashtable);
	init_list(&__scheduler.running_list);
	init_list(&__scheduler.sleeping_list);
	init_list(&__scheduler.stoped_list);
	register_interrupt_handler(__isr0,0);
}
