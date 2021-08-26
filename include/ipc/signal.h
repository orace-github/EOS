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

#ifndef __SIGNAL_H__
#define __SIGNAL_H__


#define SIGKILL 	1	// no writable
#define SIGABRT 	2	// no writable
#define SIGSEGV 	3	// no writable
#define SIGFPE 		4	// no writable
#define SIGPGE 		5	// no writable
#define SIGCHLD		6	// no writable
#define SIGWKP		7	// no writable

#define SIGINT 		8	// writable (Ctrl + C)
#define SIGSTOP 	9	// writable (Ctrl + Z)
#define SIGBUS 		10	// writable
#define SIGALARM 	11	// writable
#define SIGUSR1 	12	// writable
#define SIGUSR2 	13	// writable

#define NUM_SIG 	13

#include <ipc/ipc.h>
#include <types.h>

typedef void (*handler_t)(int);
#define SIGIGN 		(handler_t)(1)
#define SIGDEF 		(handler_t)(0)
#define CLEAR_SIGNAL(mask,sig) 		(mask &= ~(u32_t)(1 << sig-1))
#define SET_SIGNAL(mask,sig) 		(mask |= (u32_t)(1 << sig-1))
#define IS_SIGNAL(mask,sig) 		(mask & (1 << sig-1))

typedef struct
{
	handler_t __handler;
	u32_t __flags; // (IPC_READ IPC_WRITE)
} sigaction_t;

static inline int signal_dequeue(int  __sigset)
{
	int __signum = 0;
	// checking each signal bit
	for(int i = 1; i <= NUM_SIG; i++)
	{
		if(IS_SIGNAL(__sigset,i))
		{
			__signum = i;
			break;
		}
	}

	return __signum;
}

static inline void set_sigaction(sigaction_t* __sigaction, handler_t __handler, u32_t __flags)
{
	__sigaction->__handler = __handler;
	__sigaction->__flags = __flags;
}

static inline int sigaction(sigaction_t* __sigaction, sigaction_t* __restorer, handler_t __handler)
{
	if(!IS_IPC_WRITE(__sigaction->__flags))
		return 0;

	if(__restorer)
	{
		__restorer->__handler = __sigaction->__handler;
		__restorer->__flags = __sigaction->__flags;
	}

	__sigaction->__handler = __handler;

	return 1;
}

#endif //__SIGNAL_H__