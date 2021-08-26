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

#include <x86/idt.h>
#include <mutex.h>
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void irq255(); // default irq

void __isr255(registers_t* __regs){
    kprintf("__isr_default not implemented.\n");
}

idt_entry_t __idt[IDT_SIZE]; // System interrupt vector table
isr_t __isr_table[IDT_SIZE]; // System interrupt service address table
static idtr_t __idtr; // idt register

void init_interrupt_des(u16_t __segment, u32_t __offset, u8_t __flags, idt_entry_t* __idt_entry){
    __idt_entry->segment = __segment;
    __idt_entry->offset0_15 = (u16_t)(__offset & 0x0000ffff);
    __idt_entry->offset16_31 = (u16_t)((__offset >> 16) & 0x0000ffff);
    __idt_entry->reserved = 0x0;
    __idt_entry->flags = __flags;
}

void set_isr_handler(u32_t __handler, u8_t __inum){
    if(__inum < 0x08){
        __idt[PIC_MASTER_IDT_BASE + __inum].offset0_15 = (u16_t)(__handler & 0x0000ffff);
        __idt[PIC_MASTER_IDT_BASE + __inum].offset16_31 = (u16_t)((__handler >> 16) & 0x0000ffff);
        __idt[PIC_MASTER_IDT_BASE + __inum].flags = INTGATE;
    }
    else{
        __inum -= 0x08;
        __idt[PIC_SLAVE_IDT_BASE + __inum].offset0_15 = (u16_t)(__handler & 0x0000ffff);
        __idt[PIC_SLAVE_IDT_BASE + __inum].offset16_31 = (u16_t)((__handler >> 16) & 0x0000ffff);
        __idt[PIC_SLAVE_IDT_BASE + __inum].flags = INTGATE;
    }
}

void register_interrupt_handler(isr_t __isr, u8_t __num){
    if(!__isr)
        return;
    if(__num < IDT_SIZE && __num >= 0)
        __isr_table[__num] = __isr; // add interrupt service in table
}

void final_irq_handler(registers_t* __regs){
    if(!__isr_table[__regs->intno])
        return;
    isr_t __handler = __isr_table[__regs->intno];
    __handler(__regs); // call interrupt
}

void init_idt_interrupt(){
    register_interrupt_handler(__isr255,255);
    for(int i = 0; i < IDT_SIZE; i++)
        init_interrupt_des(KERNEL_CS,(u32_t)(irq255),INTGATE,&__idt[i]); 
    set_isr_handler((u32_t)irq0,0x0);
    set_isr_handler((u32_t)irq1,0x1);
    set_isr_handler((u32_t)irq2,0x2);
    set_isr_handler((u32_t)irq3,0x3);
    set_isr_handler((u32_t)irq4,0x4);
    set_isr_handler((u32_t)irq5,0x5);
    set_isr_handler((u32_t)irq6,0x6);
    set_isr_handler((u32_t)irq7,0x7);
    set_isr_handler((u32_t)irq8,0x8);
    set_isr_handler((u32_t)irq9,0x9);
    set_isr_handler((u32_t)irq10,0xA);
    set_isr_handler((u32_t)irq11,0xB);
    set_isr_handler((u32_t)irq12,0xC);
    set_isr_handler((u32_t)irq13,0xD);
    set_isr_handler((u32_t)irq14,0xE);
    set_isr_handler((u32_t)irq15,0xF);    
}

void load_idtr(){
    __idtr.base = (u32_t)&__idt;
    __idtr.limit = IDT_SIZE*sizeof(idt_entry_t) - 1;
    __asm__ __volatile__("lidt %0"::"m"(__idtr));
}

void init_idt(){
    kprintf("Installing IDT... ");
    init_pic8259A();
    init_idt_interrupt();
    timer();
    load_idtr();
    kprintf("[ok]\n");
}
