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

#include <syscall.h>
#include <mutex.h>
#include <lib.h>
extern void syscall();
extern process_t* current;
extern process_t* __shell;
extern idt_entry_t __idt[];
// ## syscall table ## this may be expanded
void* syscall_table [] = {
	// ## [SIG_NUMBER] = HANDLER 
	[SYS_SIGRETURN] = NULL, 
	[SYS_EXIT] = process_exit,
	[SYS_FORK] = process_fork,
	[SYS_WAITPID] = process_waitpid,
	[SYS_EXEC] = process_exec,
	[SYS_TIME] = local_time,
	[SYS_GETPID] = process_pid,
	[SYS_STIME] = set_time,
	[SYS_ALARM] = process_alarm,
	[SYS_PAUSE] = process_pause,
	[SYS_KILL] = process_kill,
	[SYS_GETPPID] = process_ppid,
	[SYS_WAIT] = process_wait,
	[SYS_SLEEP] = process_sleep,
	[SYS_GETCWD] = process_cwd,
	[SYS_WAKEUP] = process_wakeup,
	[SYS_WAKEUPID] = process_wakeupid,
	[SYS_BRK] = process_sbrk,
	[SYS_BHEAP] = process_bheap,
	[SYS_EHEAP] = process_eheap,
	[SYS_SHMCREAT] = shm_create,
	[SYS_SHMCLOSE] = shm_release,
	[SYS_SEMCREAT] = semaphore_open,
	[SYS_SEMP] = semaphore_wait,
	[SYS_SEMV] = semaphore_signal,
	[SYS_SEMUPDATE] = semaphore_update,
	[SYS_SIGNAL] = process_signal,
	[SYS_SIGACTION] = process_sigaction,
	[SYS_MKNODE] = vfs_mknode,
	[SYS_READ] = vfs_read,
	[SYS_WRITE] = vfs_write,
	[SYS_OPEN] = vfs_open, 
	[SYS_CLOSE] = vfs_close,
	[SYS_LINK] = vfs_link,
	[SYS_UNLINK] = vfs_unlink,
	[SYS_CHDIR] = vfs_chdir,
	[SYS_CHMOD] = vfs_chmod,
	[SYS_STAT] = vfs_stat,
	[SYS_LSEEK] = vfs_seek,
	[SYS_MOUNT] = vfs_mount,
	[SYS_UMOUNT] = vfs_umount,
	[SYS_RNDIR] = vfs_rndir,
	[SYS_MKDIR] = vfs_mkdir,
	[SYS_RMDIR] = vfs_rmdir,
	[SYS_EOF] = vfs_eof,
	[SYS_FLUSH] = vfs_flush,
	[SYS_TOUCH] = vfs_touch,
	[SYS_EXIST] = vfs_exist, 
	[SYS_LOCK] = vfs_lock,
	[SYS_SYNC] = vfs_sync,
	[SYS_UNLOCK] = vfs_unlock,
	[SYS_PROCESSIO] = process_io,
	[SYS_CLOCK] = process_clock,
};

void set_syscall_handler(u32_t __handler, u8_t __snum){
    __idt[__snum].offset0_15 = (u16_t)(__handler & 0x0000ffff);
    __idt[__snum].offset16_31 = (u16_t)((__handler >> 16) & 0x0000ffff);
    __idt[__snum].flags = TRAPGATE;
}

void init_syscall(void){
	struct mutex __mutex;
	INIT_MUTEX(&__mutex);
	__mutex.mutex_lock();
	kprintf("Installing System Call Interface...");
	set_syscall_handler((u32_t)syscall,0x80);
	kprintf("[ok]\n");
	__mutex.mutex_unlock();
}

void __isrsyscall(registers_t* __regs){
	// save current registers
	memcpy(&current->regs,__regs,sizeof(registers_t));
	// check interrupt mode
	if(current->regs.cs != USER_CS){
		current->regs.esp = (u32_t)(&__regs->esp);
		current->regs.ss = current->kstack.ss0;
	}
	// ## check eax bound ##
	if(current->regs.eax >= SYSCALL_NUMS)
		return;
	// ## when sys_return is called ##
	if(current->regs.eax == 0){
		u32_t* __esp = NULL;
		// get process state from stack
		__esp = (u32_t*)current->regs.esp;
		current->regs.eax = __esp[18];
		current->regs.ebx = __esp[17];
		current->regs.ecx = __esp[16];
		current->regs.edx = __esp[15];
		current->regs.ebp = __esp[14];
		current->regs.esp = __esp[13];
		current->regs.esi = __esp[12];
		current->regs.edi = __esp[11];
		current->regs.eip = __esp[10];
		current->regs.eflags = __esp[9];
		current->regs.cs = __esp[8];
		current->regs.ss = __esp[7];
		current->regs.ds = __esp[6];
		current->regs.es = __esp[5];
		current->regs.gs = __esp[4];
		current->regs.fs = __esp[3];
		current->kstack.esp0 = __esp[2];
		switch_process(__shell); // go back to __shell
	}
	int eax = 0;
	// ## select syscall handler from syscall table ##	
	void* sys_handler =  syscall_table[current->regs.eax];
	__asm__ __volatile__("push %1;" /* edi */
						"push %2;"  /* esi */
						"push %3;"  /* edx */
						"push %4;"  /* ecx */
						"push %5;"  /* ebx */
						"call *%6;"  /* handler */
						"pop %%ebx;"
						"pop %%ebx;"
						"pop %%ebx;"
						"pop %%ebx;"
						"pop %%ebx;"
						:"=a"(eax):"r"(current->regs.edi), "r"(current->regs.esi),
						"r"(current->regs.edx), "r"(current->regs.ecx),
						"r"(current->regs.ebx),"r"(sys_handler));
	// ## save return value ##
	current->regs.eax = eax;
}