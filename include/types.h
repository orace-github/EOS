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

#ifndef __TYPES_H__
#define __TYPES_H__
#define NULL 			(void*)0x0
#define TRUE			0x01
#define FALSE			0x00
#define va_start(v, l)	__builtin_va_start(v, l)
#define va_arg(v, l)	__builtin_va_arg(v, l)
#define va_end(v)	__builtin_va_end(v)
#define va_copy(d, s)	__builtin_va_copy(d, s)
typedef unsigned char u8_t;
typedef signed char s8_t;
typedef unsigned short u16_t;
typedef signed short s16_t;
typedef unsigned long u32_t;
typedef signed long s32_t;
typedef unsigned long long u64_t;
typedef signed long long s64_t;
typedef __builtin_va_list va_list;
typedef struct{
	u32_t intno /* interrupt number */;
	u32_t gs; u32_t fs;
	u32_t es; u32_t ds; u32_t edi;
	u32_t esi; u32_t ebp; u32_t kesp; 
	u32_t ebx; u32_t edx; u32_t ecx; 
	u32_t eax; u32_t eip; u32_t cs; 
	u32_t eflags; u32_t esp; u32_t ss;
}__attribute__((packed)) registers_t;
// Don't change the structure
#endif // __TYPES_H__

