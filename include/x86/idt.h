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

#ifndef __IDT_H__
#define __IDT_H__

#include <types.h>
#include <io.h>
#include <x86/gdt.h>
#include <x86/pic.h>
#define IDT_SIZE 256
#define INTGATE 0x8e
#define TRAPGATE 0xef
typedef void (*isr_t)(registers_t*);
typedef struct{
    u16_t offset0_15;
    u16_t segment;
    u8_t reserved;
    u8_t flags;
    u16_t offset16_31;
}__attribute__((packed)) idt_entry_t;
typedef struct{
    u16_t limit;
    u32_t base;
}__attribute__((packed)) idtr_t;
void init_interrupt_des(u16_t __segment, u32_t __offset, u8_t __flags, idt_entry_t* __idt_entry);
void set_isr_handler(u32_t __handler, u8_t __num);
void init_idt();
void init_idt_interrupt();
void load_idtr();
void register_interrupt_handler(isr_t __isr, u8_t __num);
void final_irq_handler(registers_t* __regs);
#endif //__IDT_H__
