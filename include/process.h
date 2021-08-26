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
#ifndef __PROCESS_H__
#define __PROCESS_H__
#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/tss.h>
#include <memory.h>
#include <ipc/semaphore.h>
#include <ipc/signal.h>
#include <ipc/shm.h>
#include <net/socket.h>
#include <fs/fs.h>
#include <types.h>
#include <time.h>
#include <elf.h>
#include <list.h>
#include <lib.h>
#include <mutex.h>
#include <error.h>
#define SLEEP       0x01
#define READY       0x02
#define RUNNING     0x03
#define ZOMBIE      0x04
#define STOPED      0x05
#define IS_SLEEPING(p)     (p->state & SLEEP)
#define IS_READY(p)        (p->state & READY)
#define IS_RUNNING(p)      (p->state & RUNNING)
#define IS_ZOMBIE(p)       (p->state & ZOMBIE)
#define IS_STOPED(p)       (p->state & STOPED)
#define PROCESS_HASH_MAX        10
#define MAX_PROCESS             0x400 // 1024
// used to grow up process heap
typedef struct{
    frame_t* frame;
    struct list_head head;
} __attribute__((packed)) heap_frame_t;
// allow to identify shared memory
typedef struct{
    shm_t* shm;
    struct list_head head;    
} __attribute__((packed)) shm_entry_t;
// process context
typedef struct process{
    registers_t regs;
    struct{
        u32_t esp0;u16_t ss0;
    } __attribute__((packed)) kstack;
    int mutex;
    int fd[MAX_FILE];
    int semid[MAX_SEMAPHORE];
    socket_t* socket[MAX_SOCKET];
    unsigned int id;
    #define PROCESS_DFLT_ID 1000
    int used;
    int realtime;
    unsigned int clock;
    u8_t cwd[PATH_MAX_LEN];
    u8_t name[DNAME_MAX_LEN];
    time_t time; // first running time
    time_t wake_up; // wakeup process from sleep mode 
    sigaction_t signal[NUM_SIG];
    int sigset;
    frame_t* __text;
    frame_t* __bss;
    frame_t* __stack;
    u32_t __kstack[PAGE_SIZE];
    char* __b_heap;
    char* __e_heap;
    struct process* parent;
    struct list_head head; // child entry
    struct list_head hhead; // process hashtable entry
    struct list_head sched_head; // schedule entry
    struct list child_list;
    struct list shm_list;
    struct list sem_list;
    struct list heap_list;
    int state;
    int status;
    dentry_t* input;
    dentry_t* output;
    dentry_t* errput;
} __attribute__((packed)) process_t;
// scheduler few context
typedef struct{
    int running;// process running
    int sleeping;// process sleeping
    int stoped; // process stoped
    struct list running_list;
    struct list sleeping_list;
    struct list stoped_list;
}__attribute__((packed)) schedule_t;
// only used to find a process by its id 
typedef struct process_hashtable{
    int init;
    struct list proc_list[PROCESS_HASH_MAX];
    void (*insert)(process_t*, struct process_hashtable*);
    void (*remove)(process_t*, struct process_hashtable*);
    process_t* (*find)(u32_t, struct process_hashtable*);
}process_hashtable_t;
// process managment
void unmap_all(process_t* __proc);
void map_all(process_t* __proc);
process_t* process();
int process_fork();
int process_wait(int* __status);
int process_waitpid(int __pid);
unsigned int process_pid();
unsigned int process_ppid();
int process_exit(int __status);
void process_sleep(int __sec);
void process_alarm(int __sec);
unsigned int process_clock();
process_t* process_lookup(unsigned int __pid);
int process_exec(process_t* __proc, u8_t* __path, int __argc, char** __argv);
int load_elf(process_t* __proc, u8_t* __buf, int __argc, char** __argv);
void process_cwd(u8_t* __cwd);
void process_pause();
int process_wakeup();
int process_wakeupid(int __pid);
int process_io(u8_t* __input, u8_t* __output, u8_t* __errput);
void process_hold(); // no system call interface
void process_release(); // no system call interface
void switch_process(process_t* __proc);
void sleeping_process();
// signal managment
void process_kill(int __pid, int __signum);
void process_signal(int __signum, handler_t __handler);
void process_sigaction(int __pid, int __signum, sigaction_t* __sigaction, sigaction_t* __restorer);
void signal_handler();
// heap managment
int process_sbrk();
char* process_bheap();
char* process_eheap();
// shared memory managment
char* shm_create(int __extern_id, int __vaddr, int __size, char __flags);
void shm_release(int __extern_id);
// initialization
void init_schedule();
void __isr0(registers_t* __regs); // PIT
#endif //__PROCESS_H__
