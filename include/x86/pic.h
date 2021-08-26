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

#ifndef __PIC_H__
#define __PIC_H__

#include <types.h>
#include <io.h>

#define PIC_MASTER_ICW1_REG	0x20
#define PIC_SLAVE_ICW1_REG	0xA0		
#define PIC_MASTER_ICW2_REG	0x21
#define PIC_SLAVE_ICW2_REG	0xA1		
#define PIC_MASTER_ICW3_REG	0x21
#define PIC_SLAVE_ICW3_REG	0xA1		
#define PIC_MASTER_ICW4_REG	0x21
#define PIC_SLAVE_ICW4_REG	0xA1		
#define PIC_MASTER_OCW1_REG	0x21
#define PIC_SLAVE_OCW1_REG	0xA1		
#define PIC_MASTER_OCW2_REG	0x20
#define PIC_SLAVE_OCW2_REG	0xA0		
#define PIC_IRQ0_MSK 0x01 // mask of IRQ0
#define PIC_IRQ1_MSK 0x02 // mask of IRQ1
#define PIC_IRQ2_MSK 0x04 // mask of IRQ2
#define PIC_IRQ3_MSK 0x08 // mask of IRQ3
#define PIC_IRQ4_MSK 0x10 // mask of IRQ4
#define PIC_IRQ5_MSK 0x20 // mask of IRQ5
#define PIC_IRQ6_MSK 0x40 // mask of IRQ6
#define PIC_IRQ7_MSK 0x80 // mask of IRQ7
#define PIC_MASTER_IDT_BASE 0x20 
#define PIC_SLAVE_IDT_BASE	0x70

static inline void init_pic8259A()
{
	// ICW1 (ICW4 + PRIORITY)
	#define ICW1_FLAGS	0x19
    outbp(PIC_MASTER_ICW1_REG,ICW1_FLAGS);
    outbp(PIC_SLAVE_ICW1_REG,ICW1_FLAGS);

    // ICW2(IDT BASE)
    outbp(PIC_MASTER_ICW2_REG,PIC_MASTER_IDT_BASE);
    outbp(PIC_SLAVE_ICW2_REG,PIC_SLAVE_IDT_BASE);

    // ICW3(MASTER: 0x04, SLAVE:0x02)
    #define MASTER_LINE	0x04
    #define SLAVE_LINE	0x02
    outbp(PIC_MASTER_ICW3_REG,MASTER_LINE);
    outbp(PIC_SLAVE_ICW3_REG,SLAVE_LINE);

    // ICW4 (EOI mode)
    #define EOI	0x01
    outbp(PIC_MASTER_ICW4_REG,EOI);
    outbp(PIC_SLAVE_ICW4_REG,EOI);

}

static void desable_interrupt(u8_t __inum)
{
	// OCW1( desable __inum)
	if(__inum < 0x08)
	{
		u8_t __status = inb(PIC_MASTER_OCW1_REG);
		__status |= __inum;
		outb(PIC_MASTER_OCW1_REG,__status);
	}
	else
	{
		__inum -= 0x08;
		u8_t __status = inb(PIC_SLAVE_OCW1_REG);
		__status |= __inum;
		outb(PIC_SLAVE_OCW1_REG,__status);
	}

}

static void enable_interrupt()
{
	// enable all interrupt
	outb(PIC_MASTER_OCW1_REG,0x0);
	outb(PIC_SLAVE_OCW1_REG,0x0);
}

static inline void timer()
{
	#define PIT0 0x40
	#define PIT_CMD_REG 0x43
	#define CHANNEL0 0x36
	#define DIVISOR 47727 // get IRQ_0 each 40ms

	// select PIT channel0
	outb(PIT_CMD_REG,CHANNEL0);
	// 40ms for IRQ0
	outb(PIT0,(u8_t)(DIVISOR & 0x00ff));
	outb(PIT0,(u8_t)(DIVISOR >> 8));
}

#endif //__PIC_H__