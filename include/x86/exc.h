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

#ifndef __EXC_H__
#define __EXC_H__
#include <x86/idt.h>
#include <ipc/signal.h>
#include <memory.h>
#include <process.h>
#include <syscall.h>
#define EXC_TABLE_SIZE	0x20
typedef isr_t exc_t;
void exc_divide_by_zero(registers_t* __regs);
void exc_debug(registers_t* __regs);
void exc_nmi(registers_t* __regs);
void exc_brp(registers_t* __regs);
void exc_overflow(registers_t* __regs);
void exc_bound(registers_t* __regs);
void exc_invopcode(registers_t* __regs);
void exc_device_not_avail(registers_t* __regs);
void exc_double_fault(registers_t* __regs);
void exc_coproc(registers_t* __regs);
void exc_invtss(registers_t* __regs);
void exc_segment_not_present(registers_t* __regs);
void exc_ssf(registers_t* __regs);
void exc_gpf(registers_t* __regs);
void exc_pf(registers_t* __regs);
void exc_reserved(registers_t* __regs);
void register_execption_handler(exc_t __exc, u8_t __num);
void init_exc();
#endif //__EXC_H__