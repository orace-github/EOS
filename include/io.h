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

#ifndef __IO_H__
#define __IO_H__
#include <types.h>
#include <string.h>
#define BLACK	0x00
#define BLUE 	0x01
#define GREEN 	0x02
#define CYAN	0x03
#define RED 	0x04
#define MAGENTA	0x05
#define YELLOW 	0x06
#define WHITE 	0x07

// mask to enable both tic and intensity
#define NO_CLI_NO_INT 0x00
#define NO_CLI_INT    0x0e
#define CLI_NO_INT    0xe0
#define CLI_INT       0xee

static inline void outb(u16_t __port, u8_t __value)
{
    __asm__ __volatile__("outb %%al, %%dx;"
            ::"a"(__value),"d"(__port));
}

static inline void outbp(u16_t __port, u8_t __value)
{
    __asm__ __volatile__("outb %%al, %%dx; jmp 1f;1:"
            ::"a"(__value),"d"(__port));
}


static inline u8_t inb(u16_t __port)
{
    u8_t __value = 0;
    __asm__ __volatile__("inb %%dx, %%al":"=a"(__value):"d"(__port));
    return __value;
}

static inline void outl(u16_t __port, u32_t __data)
{
    __asm__ __volatile__("outl %%eax, %%dx"
                        ::"a"(__data),"d"(__port));
}

static inline u32_t inl(u16_t __port)
{
    u32_t __value = 0;

    __asm__ __volatile__("inl %%dx, %%eax"
                        :"=a"(__value):"d"(__port));
                        return __value;
}

static inline void outw(u16_t __port, u16_t __data)
{
    __asm__ __volatile__("outw %%ax, %%dx"
                        ::"a"(__data),"d"(__port));
}

static inline u16_t inw(u16_t __port)
{
    u16_t __value = 0;
    __asm__ __volatile__("inw %%dx, %%ax"
                        :"=a"(__value):"d"(__port));
                        return __value;
}

void put_char(char __data);
void put_string(const char* __string);
void put_hex(int __data);
void scrollup(int __line);
void update_cursor(int __x, int __y);
void kprintf(const char*, ...);

#endif //__IO_H__

