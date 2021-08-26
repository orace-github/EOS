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

#include <kernel.h>
#include <termios.h>
#include <syscall.h>
#include <process.h>
#include <pci.h>
#include <mutex.h>
#include <memory.h>
#include <device.h>
#include <x86/gdt.h>
#include <x86/idt.h>
#include <x86/tss.h>
#include <x86/exc.h>
#include <ipc/semaphore.h>
#include <ipc/shm.h>
#include <ipc/signal.h>
#include <fs/dcache.h>
#include <fs/efs2.h>
#include <fs/vfs.h>
#include <driver/rtc.h>
#include <driver/ata.h>
#include <driver/kbd.h>
#include <driver/rtl8139.h>
#include <net/ethernet.h>
#include <net/tcp.h>
#include <net/ip.h>
#define INITIALIZE_x86()	\
	init_gdt();			\
	init_tss();			\
	init_idt();			\
	init_exc();			\

#define INIT_SYSCALL() \
	init_syscall();

#define INIT_SCHEDULE()	\
	init_schedule();

#define INIT_KBD()	\
	init_kbd();

#define INIT_TERMIOS(__in,__out,__err,__term) \
	init_stdio(__in,__out,__err,__term);


#define INIT_SHARED_MEMORY()	\
	init_shm_hashtable_t();

static frame_t __kframe __attribute__((section(".data")));
static frame_t __dcache_frame __attribute__((section(".data")));
static frame_t __heap_frame __attribute__((section(".data")));
static fifo_t __input __attribute__((section(".data")));
static fifo_t __output __attribute__((section(".data")));
static fifo_t __errput __attribute__((section(".data")));
static termios_t __termios __attribute__((section(".data")));
static dev_t __rootdev __attribute__((section(".data")));
static dentry_t* __devdentry __attribute__((section(".data")));
static dentry_t* __rootdentry __attribute__((section(".data")));
static struct mutex __mutex;

void initmodule(multiboot_t* __multiboot)
{

	// initialize shared memory
	INIT_SHARED_MEMORY();

	// initialize keyboard	
	INIT_KBD();
	// initialize termios and std IO
	INIT_TERMIOS(&__input,&__output,&__errput,&__termios);
	// show terminal input
	tty_showinput(1);
	// initialize system call
	INIT_SYSCALL();
	// __shell process
	kprintf("Installing shell process...");
	INIT_SCHEDULE();
	kprintf("[ok]\n");
}

int initfs(void)
{
	
}

int initrd(u32_t __initrd_base, u32_t __initrd_end)
{
	// now return 0
	return 1;
}

void db_test()
{
	efs2_inode_t* inode = sys_alloc(sizeof(efs2_inode_t));
	efs2_bds_t* bds = sys_alloc(sizeof(efs2_bds_t));
	if(!bds)
		kprintf("No enough memory");
	inode->i_num = 5;
	inode->i_pnum = 0;
	inode->i_block[0] = 0;
	char* msg = "Hello World";
	kprintf("%x --> %x\n",inode,
		get_physical_address(inode));	
	efs2_mkdir(__rootdentry,"home");
	efs2_mkdir(__rootdentry,"etc");
	efs2_mkdir(__rootdentry,"user");
	efs2_mkdir(__rootdentry,"bin");
	
	if(vfs_touch("/home/file.txt"))
		kprintf("new file created:)\n");

	if(vfs_touch("/home/config.eos"))
		kprintf("new file created:)\n");

	int id = vfs_open("/home/file.txt",WRITE_O|APPEND_O);
	int ie = vfs_open("/home/config.eos",WRITE_O);
	dentry_t* file = vfs_lookup("/home/config.eos");
	vfs_lock("/home/config.eos",WRITE_O);
	kprintf("%x\n",file->d_lock);
	
	if(!vfs_rmdir("/home/config.eos",0))
		kprintf("Remove failed\n");
	vfs_unlock("/home/config.eos");
	kprintf("%x\n",file->d_lock);
	
	if(vfs_write(ie,"config.eos",strlen("config.eos")));
		kprintf("Write failed\n");
	file->d_mode |= WRITE_O;
	vfs_chmod("/home/file.txt",WRITE_O);
	kprintf("texte length: %d file[1] id: %d file[2]:%d\n",vfs_write(id,"Hello World\n",strlen("Hello World\n")),id,ie);
	kprintf("%s\n",vfs_readfile("/home/file.txt"));

	if(file->d_mode & WRITE_O)
		kprintf("WRITE_O set\n");
	else
		kprintf("d_mode: %x\n",file->d_mode);

	static path_t vpath;
	vpath.size = 0;
	init_list(&vpath.e_list);
	path("home/orace/",&vpath);
	struct list_head* it = NULL;
	list_for_each(it,&vpath.e_list.head)
	{
		path_entry_t* entry = list_entry(it,path_entry_t,head);
		kprintf("%s %d\n",entry->name,entry->index);
	}
	
}

#define prompt "eos@shell:#"
void shell(){
	kprintf(prompt);
	static char __kbd[20];
	memset(__kbd,0,20);
	tty_read(0,__kbd,20);
	if(!strcmp(__kbd,"clock"))
		kprintf("clock time:%ds\n",time_clock());
	
}

int main(multiboot_t* __multiboot)
{
	INIT_MUTEX(&__mutex);
	__mutex.mutex_lock();
	INITIALIZE_x86();
	mm_init(__multiboot);
	init_dcache(dcache_frame());
	initmodule(__multiboot);
	init_devmanager();
	__mutex.mutex_unlock();
	initfs();
	db_test();
	if(find_pcidev(0,0,RTL8139_VENDOR_ID,RTL8139_DEVICE_ID,PCI_FIND_BY_ID))
		kprintf("rtl8139 pcidev found!\n");
	
	while(1)
		shell();
	return 0;
}
