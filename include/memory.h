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

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <types.h>
#include <lib.h>
#include <list.h>
#include <boot.h>

#define USER_OFFSET		0x40000000 // load process code at 1Go 
#define USER_STACK		0xE0000000 // load process stack at 3.8Go
#define USER_STACK_SIZE	0x8000 	   // 32ko of process stack
#define SHM_OFFSET		0xE0000000 // shared memory base
#define SHM_UP			0xFFC00000 // shared memory limit

typedef struct alloc
{
	u16_t status;
	u32_t size;
	struct alloc* next;
	struct alloc* prev;
} alloc_t;


typedef struct
{
	u32_t p_addr;
	u32_t v_addr;
	int page;
	int pid;
	u8_t status;
	struct list_head head;
} frame_t;

typedef struct
{
	u32_t heap_begin;
	u32_t heap_end;
	u32_t memory_used;
	u32_t frame_begin;
	u32_t frame_end;
	u32_t frame_up;
	u32_t frame_used;
	u32_t frame_free;
	u32_t kernel_base;
	u32_t kernel_end;
	u32_t initrd_base;
	u32_t initrd_end;
	frame_t kernel_frame;
	frame_t heap_frame;
	frame_t dcache_frame;
	struct list frame_list;
}__attribute__((packed)) memory_manager_t;

void mm_init(multiboot_t* __multiboot);
void* sys_alloc(u32_t __bytes);
void sys_free(void* __mm);
frame_t* alloc_frame(int __pid, int __page);
int free_frame(int __pid, frame_t* __frame);
frame_t* dcache_frame();
frame_t* heap_frame();
frame_t* kernel_frame();
memory_manager_t memory_manager();

#define PAGE_SIZE 0x1000 // 4 Ko
#define PAGING_FLAG 0x80000000
#define PAGE_PRESENT 0x01
#define PAGE_READ_WRITE 0x02
#define PAGE_USER_SUPERUSER 0x04
#define VADDR_PDE(addr) ((addr >> 22) & 0x3ff)
#define VADDR_PTE(addr) ((addr >> 12) & 0x3ff)
#define PAGE_PER_MO	((0x400*0x400)/PAGE_SIZE)

typedef struct
{
	u32_t present 	: 1;
	u32_t rw 		: 1;
	u32_t user 		: 1;
	u32_t 		 	: 1;
	u32_t cache_off	: 1;
	u32_t accessed 	: 1;
	u32_t written 	: 1;
	u32_t global 	: 1;
	u32_t reserved	: 4;
	u32_t address	: 20;
} __attribute__((packed)) page_t;

typedef struct
{
	page_t page[0x400];
} __attribute__((packed)) page_table_t;

typedef struct
{
	u32_t v_addr[0x400]; // Contains physical address of table above 
} __attribute__((packed)) page_directory_t;

void map_frame(frame_t* __frame, u8_t __flag);
void unmap_frame(frame_t* __frame);

// Only used if system paging not enable
// Because we can get access to physical address 
// of page directory
void kmap_frame(frame_t* __frame, u8_t __flag); 
void kunmap_frame(frame_t* __frame); 

void init_page_directory();
void enable_paging(frame_t* __frame);
void* get_physical_address(void* __addr);
page_t get_page(void* __addr);

static inline void invlpg(u32_t __addr)
{
	__asm__ __volatile__("invlpg (%0)"::"b"(__addr):"memory");
}

#endif // __MEMORY_H__
