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
#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#include <x86/idt.h>
#include <process.h>
#include <fs/vfs.h>
#include <ipc/signal.h>
#include <ipc/shm.h>
#include <ipc/semaphore.h>
#include <termios.h>
#include <time.h>
// process system call 
#define SYS_SIGRETURN 	0
#define SYS_EXIT		1
#define SYS_FORK		2
#define SYS_WAITPID		3
#define SYS_EXEC		4
#define SYS_TIME		5
#define SYS_GETPID		6
#define SYS_STIME		7
#define SYS_ALARM		8
#define SYS_PAUSE		9
#define SYS_KILL		10
#define SYS_GETPPID		11
#define SYS_WAIT 		12
#define SYS_SLEEP		13
#define SYS_GETCWD		14
#define SYS_WAKEUP		15
#define SYS_WAKEUPID	16
// heap system call
#define SYS_BRK			17
#define SYS_BHEAP		18
#define SYS_EHEAP		19
// shared memory system call
#define SYS_SHMCREAT	20
#define SYS_SHMCLOSE	21
// sempahore system call
#define SYS_SEMCREAT	22
#define SYS_SEMP		23
#define SYS_SEMV		24
#define SYS_SEMUPDATE	25
// signal system call
#define SYS_SIGNAL		26
#define SYS_SIGACTION	27
// FS system call
#define SYS_MKNODE		28
#define SYS_READ		29
#define SYS_WRITE		30
#define SYS_OPEN		31
#define SYS_CLOSE		32
#define SYS_LINK		33
#define SYS_UNLINK		34
#define SYS_CHDIR		35
#define SYS_CHMOD		36
#define SYS_STAT		37
#define SYS_LSEEK		38
#define SYS_MOUNT		39
#define SYS_UMOUNT		40
#define SYS_RNDIR		41
#define SYS_MKDIR		42
#define SYS_RMDIR		43
#define SYS_EOF			44
#define SYS_FLUSH		45
#define SYS_TOUCH		46
#define SYS_EXIST		47
#define SYS_LOCK		48
#define SYS_SYNC		49
#define SYS_UNLOCK		50
#define SYS_PROCESSIO	51
#define SYS_CLOCK		52
#define SYSCALL_NUMS 	53
void init_syscall(void);
#endif //__SYSCALL_H__
