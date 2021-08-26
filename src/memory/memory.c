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
#include <memory.h>
// kernel memory managment
static page_table_t __table[0x3ff] __attribute__((aligned(PAGE_SIZE)));
static page_directory_t __dir __attribute__((aligned(PAGE_SIZE)));
static memory_manager_t __memory_manager;
static alloc_t* __first = NULL;
static alloc_t* __last = NULL;
extern void* __kernel_base; // ## KERNEL PHYSICAL MEMORY BASE ##
extern void* __kernel_end; // ## KERNEL PHYSICAL MEMORY END ##

frame_t* dcache_frame()
{
	return &__memory_manager.dcache_frame;
}

frame_t* heap_frame()
{
	return &__memory_manager.heap_frame;
}

frame_t* kernel_frame()
{
	return &__memory_manager.kernel_frame;
}

memory_manager_t memory_manager()
{
	return __memory_manager;
}

void mm_init(multiboot_t* __multiboot)
{
	// ## MAP IDENTITY KERNEL PHYSICAL MEMORY ##
	__memory_manager.kernel_frame.v_addr = __memory_manager.kernel_frame.p_addr = 0x0;
	__memory_manager.kernel_base = (u32_t)(&__kernel_base);
	__memory_manager.kernel_end = (u32_t)(&__kernel_end);
	
	// ## FOR INITRD MEMORY LOCATION
	if(__multiboot->mods_count > 0)
	{
		__memory_manager.kernel_end = *(u32_t*)(__multiboot->mods_addr + 4);
		__memory_manager.initrd_base = *((u32_t*)__multiboot->mods_addr);
		__memory_manager.initrd_end = *(u32_t*)(__multiboot->mods_addr + 4);
	}
		
	// ## ALIGN KERNEL PHYSICAL MEMORY END ##
	if(__memory_manager.kernel_end % PAGE_SIZE)
		__memory_manager.kernel_end += (PAGE_SIZE - (__memory_manager.kernel_end % PAGE_SIZE));
	__memory_manager.kernel_frame.page = __memory_manager.kernel_end / PAGE_SIZE;
	
	#ifdef __DEBUG__
		kprintf("Installing paging system...");
	#endif //__DEBUG__
	enable_paging(kernel_frame());
	#ifdef __DEBUG__
		kprintf("[ok]\n");
	#endif //__DEBUG__

	if(__multiboot->mem_upper >= (0xffffffff/0x400)) // >= 4Go
	{
		// ## DCACHE = 200 Mo && HEAP = 256 Mo ##
		__memory_manager.dcache_frame.page = 200 * PAGE_PER_MO;
		__memory_manager.dcache_frame.v_addr = __memory_manager.dcache_frame.p_addr = __memory_manager.kernel_end;
		__memory_manager.kernel_end += __memory_manager.dcache_frame.page * PAGE_SIZE;
		__memory_manager.heap_frame.v_addr = __memory_manager.heap_frame.p_addr = __memory_manager.kernel_end;
		__memory_manager.heap_frame.page = 256 * PAGE_PER_MO;
		__memory_manager.kernel_end += __memory_manager.heap_frame.page * PAGE_SIZE;
	}
	else if(__multiboot->mem_upper >= (0x80000000/0x400) && 
		__multiboot->mem_upper < (0xffffffff/0x400)) // >= 2Go && < 4Go
	{
		// ## DCACHE = 100 Mo && HEAP = 128 Mo ##
		__memory_manager.dcache_frame.page = 100 * PAGE_PER_MO;
		__memory_manager.dcache_frame.v_addr = __memory_manager.dcache_frame.p_addr = __memory_manager.kernel_end;
		__memory_manager.kernel_end += __memory_manager.dcache_frame.page * PAGE_SIZE;
		__memory_manager.heap_frame.v_addr = __memory_manager.heap_frame.p_addr = __memory_manager.kernel_end;
		__memory_manager.heap_frame.page = 128 * PAGE_PER_MO;
		__memory_manager.kernel_end += __memory_manager.heap_frame.page * PAGE_SIZE;
	}
	else if(__multiboot->mem_upper < (0x80000000/0x400)) // < 2Go
	{
		// ## DCACHE = 50 Mo && HEAP = 64 Mo
		__memory_manager.dcache_frame.page = 50 * PAGE_PER_MO;
		__memory_manager.dcache_frame.v_addr = __memory_manager.dcache_frame.p_addr = __memory_manager.kernel_end;
		__memory_manager.kernel_end += __memory_manager.dcache_frame.page * PAGE_SIZE;
		__memory_manager.heap_frame.v_addr = __memory_manager.heap_frame.p_addr = __memory_manager.kernel_end;
		__memory_manager.heap_frame.page = 64 * PAGE_PER_MO;
		__memory_manager.kernel_end += __memory_manager.heap_frame.page * PAGE_SIZE;
	}

	__memory_manager.heap_begin = __memory_manager.heap_frame.p_addr;
	__memory_manager.heap_end = __memory_manager.heap_frame.p_addr + __memory_manager.heap_frame.page * PAGE_SIZE;
	__memory_manager.memory_used = 0;
	
	__memory_manager.frame_begin = __memory_manager.kernel_end;
	__memory_manager.frame_up = __memory_manager.kernel_end;
	__memory_manager.frame_end = __multiboot->mem_upper * 0x400;
	__memory_manager.frame_used = 0;
	__memory_manager.frame_free = 0;
	init_list(&__memory_manager.frame_list);

	// ## MAPPING DCACHE MEMORY ##
	#ifdef __DEBUG__
		kprintf("Installing dcache memory...");
	#endif //__DEBUG__
	kmap_frame(dcache_frame(),PAGE_PRESENT|PAGE_READ_WRITE);
	#ifdef __DEBUG__
		kprintf("[ok]\n");
	#endif //__DEBUG__	
	// ## MAPPING HEAP MEMORY ##
	#ifdef __DEBUG__
		kprintf("Installing heap memory...");
	#endif //__DEBUG__
	kmap_frame(heap_frame(),PAGE_PRESENT|PAGE_READ_WRITE);
	#ifdef __DEBUG__
		kprintf("[ok]\n");
	#endif //__DEBUG__
}

// ## BASIC PHYSICAL MEMORY ALLOCATION ROUNTINE ##
void* sys_alloc(u32_t __size)
{
	if(!__size)
		return NULL;

	// initial state
	if(!__memory_manager.memory_used &&
		(__memory_manager.heap_begin + __size + sizeof(alloc_t)) <= 
		__memory_manager.heap_end)
	{
		__first = (alloc_t*)(__memory_manager.heap_begin);
		__first->status = 1;
		__first->size = __size;
		__first->prev = NULL;
		__first->next = NULL;

		// update the last alloc
		__last = __first;
		// update memory used field
		__memory_manager.memory_used += sizeof(alloc_t) + __size;
		memset((char*)__first+sizeof(alloc_t),0,__size);

		return (void*)((u32_t)__first + sizeof(alloc_t)); 
	}

	// go through free space
	for(alloc_t* __alloc = __first; __alloc != NULL; __alloc = __alloc->next)
	{
		// find free space
		if(!__alloc->status && __alloc->size >= __size)
		{
			// remaining bytes
			int __rbytes = __alloc->size - __size;
			if(__rbytes > sizeof(alloc_t))
			{
				alloc_t* __temp = (alloc_t*)((u32_t)__alloc + sizeof(alloc_t) +
					__size);

				// update list
				if(__alloc->next)
					__alloc->next->prev = __temp;

				__temp->next = __alloc->next;
				__alloc->next = __temp;
				__temp->prev = __alloc;

				__temp->status = 0;
				__temp->size = __rbytes - sizeof(alloc_t);
				
				// update alloc size
				__alloc->size = __size;

				// check if __last is __alloc
				if(__last == __alloc)
					__last = __temp;	 
			}

			// update alloc status
			__alloc->status = 1;
			memset((char*)__alloc + sizeof(alloc_t),0,__alloc->size);
			return (void*)((u32_t)__alloc + sizeof(alloc_t));
		}
	}

	// whether last_alloc + __size + sizeof(alloc_t) < memory_end then
	// create new memory space

	if((__memory_manager.memory_used + __memory_manager.heap_begin + __size + sizeof(alloc_t))
	 <= __memory_manager.heap_end)
	{
		alloc_t* __alloc = (alloc_t*)((u32_t)__last + sizeof(alloc_t) + __last->size);
		__alloc->size = __size;
		__alloc->status = 1;

		// update list
		__alloc->prev = __last;
		__alloc->next = NULL;
		__last->next = __alloc;

		// update memory used
		__memory_manager.memory_used += __size + sizeof(alloc_t);

		memset((char*)__alloc + sizeof(alloc_t),0,__size);
		__last = __alloc;
		return (void*)((u32_t)__alloc + sizeof(alloc_t));
	}

	return NULL;
}

void sys_free(void* __ptr)
{
	if(!__ptr)
		return;
	
	// compute alloc_t memory location
	alloc_t* __alloc = (alloc_t*)((u32_t)__ptr - sizeof(alloc_t));
	__alloc->status = 0;
	
	// update memory used
	__memory_manager.memory_used -= __alloc->size + sizeof(alloc_t);

	// check whether previous alloc_t is free
	if(__alloc->prev && !__alloc->prev->status)
	{
		__alloc->prev->size += sizeof(alloc_t) + __alloc->size;
		// update list
		__alloc->prev->next = __alloc->next;
		if(__alloc->next)
			__alloc->next->prev = __alloc->prev;

		// check whether __alloc is __last
		if(__last == __alloc)
			__last = __alloc->prev;

		// update __alloc
		__alloc = __alloc->prev;
	}

	// check whether next alloc_t is free
	if(__alloc->next && !__alloc->next->status)
	{
		__alloc->size += sizeof(alloc_t) + __alloc->next->size;
		// check whether __alloc->next is __last
		if(__alloc->next == __last)
			__last = __alloc;

		// update list
		__alloc->next = __alloc->next->next;
		if(__alloc->next)
			__alloc->next->prev = __alloc;
	}
	
}

// ## BASIC VIRTUAL MEMORY ALLOCATION ROUNTINE ##
frame_t* alloc_frame(int __pid, int __page)
{
	struct list_head* it = NULL;
	/* Loop through frame and find a frame sized the same or bigger */
	if(__memory_manager.frame_free)
	{
		list_for_each(it,&__memory_manager.frame_list.head)
		{
			frame_t* __frame = list_entry(it,frame_t,head);
			if(!__frame->status && (__frame->page >= __page))
			{
				int __rpage = __frame->page - __page;
				/* if remaining page is gretter than 0 create a new frame
				with __rpage */
				if(__rpage > 0)
				{
					frame_t* __nframe = sys_alloc(sizeof(frame_t));
					__nframe->p_addr = __frame->p_addr + (__page * PAGE_SIZE); 
					__nframe->status = 0;
					__nframe->pid = -1;
					__nframe->page = __rpage;
					__memory_manager.frame_free++; /* new free frame is add-on system */
					list_insert_at(&__memory_manager.frame_list,&__nframe->head,&__frame->head);
				}

				__frame->page = __page;
				__frame->status = 1;
				__frame->pid = __pid;
				__memory_manager.frame_free--;/* one free frame is removed from system */ 
				__memory_manager.frame_used += __page * PAGE_SIZE;
				return __frame;
			}
		}
	}

	if((__memory_manager.frame_up + __page * PAGE_SIZE) >= __memory_manager.frame_end)
		return NULL;

	frame_t* __frame = sys_alloc(sizeof(frame_t));
	if(!__frame)
		return NULL;
	
	__frame->p_addr = __memory_manager.frame_up;
	__frame->page = __page;
	__frame->status = 1;
	__frame->pid = __pid;
	__memory_manager.frame_up += __page * PAGE_SIZE;
	__memory_manager.frame_used += __page * PAGE_SIZE;
	list_insert(&__memory_manager.frame_list,&__frame->head);
	return __frame;
}

/* Find one free frame which has his next free */
frame_t* search()
{
	struct list_head* it = NULL, *next_it = NULL;

	list_for_each(it,&__memory_manager.frame_list.head)
	{
		frame_t* __frame = list_entry(it,frame_t,head);
		next_it = it->next;

		if(next_it)
		{
			frame_t* __next = list_entry(next_it,frame_t,head);
			if(!__frame->status && !__next->status)
				return __frame;
		}
	}

	return NULL;
}


int free_frame(int __pid, frame_t* __frame)
{

	if(__pid != __frame->pid)
		return 0;

	__frame->pid = -1;
	__frame->status = 0;
	__memory_manager.frame_free++;
	__memory_manager.frame_used -= __frame->page * PAGE_SIZE;
	/* one free frame is removed from system */ 

	loop:;
	/* Find one free frame which has his next free */
	frame_t* __start = search();

	if(__start)
	{
		frame_t* __next = list_entry(__start->head.next,frame_t,head);
		while(!__next->status)
		{
			__start->page += __next->page;
			list_remove(&__memory_manager.frame_list,&__next->head);
			sys_free(__next);
			__next = list_entry(__start->head.next,frame_t,head);
		}

		goto loop;
	}

	return 1;
}

// ########## PAGING CONTEXT ############
void init_page_directory()
{
    for(int i = 0; i < 0x3ff; i++)
    {
        __dir.v_addr[i] = (u32_t)(&__table[i]) | (PAGE_PRESENT|PAGE_READ_WRITE|PAGE_USER_SUPERUSER);
    }

    /*last page directory entry was hardwired to itself*/
    __dir.v_addr[0x3ff] = (u32_t) (__dir.v_addr) | (PAGE_PRESENT|PAGE_READ_WRITE);
}

void enable_paging(frame_t* __frame)
{
    init_page_directory();
    kmap_frame(__frame,PAGE_PRESENT|PAGE_READ_WRITE);
    u32_t cr0 = 0;
    __asm__ __volatile__("movl %0, %%cr3;"::"r"(__dir.v_addr));
    __asm__ __volatile__("movl %%cr0, %0;":"=r"(cr0));
    cr0 |= PAGING_FLAG;
    __asm__ __volatile__("movl %0, %%cr0;"::"r"(cr0)); // Enable paging
}

void* get_physical_address(void* __addr)
{
    u32_t __pd_index = (u32_t)__addr >> 22;
    u32_t __pt_index = ((u32_t)__addr >> 12) & 0x3ff;
    
    u32_t* __pt = ((u32_t*)(0xffc00000) + (__pd_index << 10));
    return (void*)((__pt[__pt_index] & ~0xfff) | ((u32_t)__addr & 0xfff));
}

page_t get_page(void* __addr)
{
    page_t __page;
    u32_t __pd_index = (u32_t)__addr >> 22;
    u32_t __pt_index = ((u32_t)__addr >> 12) & 0x3ff;
    
    u32_t* __pt = (u32_t*)(0xffc00000 + (__pd_index << 10));
    u32_t __paddr = __pt[__pt_index];

    __page.address = __paddr >> 12;
    __page.present = __paddr & 0x01;
    __page.rw = __paddr & 0x02;
    __page.user = __paddr & 0x04;
    __page.accessed = __paddr & 0x20;
    __page.written = __paddr & 0x40;
    __page.cache_off = __paddr & 0x10;
    __page.global = __paddr & 0x80;
    return __page;
}

void map_frame(frame_t* __frame, u8_t __flag)
{
    if(!__frame)
        return;

    u32_t __v_addr = __frame->v_addr;
    u32_t __p_addr = __frame->p_addr;

    for(int i = 0; i < __frame->page; i++)
    {
        int __pd_index = VADDR_PDE(__v_addr); // PDE --> x
        int __pt_index = VADDR_PTE(__v_addr); // PTE --> y

        u32_t* __pt = ((u32_t*) (0xffc00000) + (__pd_index << 10));
        __pt[__pt_index] = __p_addr|__flag|0x01;
        invlpg(__v_addr);
        __v_addr += PAGE_SIZE;
        __p_addr += PAGE_SIZE;
    }

}

void unmap_frame(frame_t* __frame)
{
    if(!__frame)
        return;
    
    u32_t __v_addr = __frame->v_addr;
    u32_t __p_addr = __frame->p_addr;

    for(int i = 0; i < __frame->page; i++)
    {
        int __pd_index = VADDR_PDE(__v_addr); // PDE --> x
        int __pt_index = VADDR_PTE(__v_addr); // PTE --> y

        u32_t* __pt = ((u32_t*) (0xffc00000) + (__pd_index << 10));
        __pt[__pt_index] = 0x0;
        __v_addr += PAGE_SIZE;
        __p_addr += PAGE_SIZE;
    }
}

// Only used if system paging not enable
// Because we can get access to physical address 
// of page directory
void kmap_frame(frame_t* __frame, u8_t __flag)
{
    if(!__frame)
        return;
    
    u32_t __v_addr = __frame->v_addr;
    u32_t __p_addr = __frame->p_addr;

    for(int i = 0; i < __frame->page; i++)
    {
        int __pd_index = VADDR_PDE(__v_addr); // PDE --> x
        int __pt_index = VADDR_PTE(__v_addr); // PTE --> y

        __table[__pd_index].page[__pt_index].present = 1;
        __table[__pd_index].page[__pt_index].rw = __flag >> 1;
        __table[__pd_index].page[__pt_index].user = __flag >> 2;
        __table[__pd_index].page[__pt_index].cache_off = 0;
        __table[__pd_index].page[__pt_index].written = 0;
        __table[__pd_index].page[__pt_index].accessed = 0;
        __table[__pd_index].page[__pt_index].global = 0;    
        __table[__pd_index].page[__pt_index].address = __p_addr >> 12;
        invlpg(__v_addr);
        __v_addr += PAGE_SIZE;
        __p_addr += PAGE_SIZE;
    }
}

// Only used if system paging not enable
// Because we can get access to physical address 
// of page directory
void kunmap_frame(frame_t* __frame)
{
    if(!__frame)
        return;
    
    u32_t __v_addr = __frame->v_addr;
    u32_t __p_addr = __frame->p_addr;

    for(int i = 0; i < __frame->page; i++)
    {
        int __pd_index = VADDR_PDE(__v_addr); // PDE --> x
        int __pt_index = VADDR_PTE(__v_addr); // PTE --> y

        __table[__pd_index].page[__pt_index].present = 0;
        __v_addr += PAGE_SIZE;
        __p_addr += PAGE_SIZE;
    }
}
