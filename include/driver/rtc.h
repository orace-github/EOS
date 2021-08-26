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

#ifndef __RTC_H__
#define __RTC_H__

#include <types.h>
#include <io.h>
#include <x86/idt.h>
#include <ipc/signal.h>

#define RTC_CMD_REG 	0x70
#define RTC_DATA_REG 	0x71
#define RTC_IRQ_NUM 	0x08
#define RTC_SEC_REG 	0x00
#define RTC_ASEC_REG 	0x01
#define RTC_MIN_REG 	0x02
#define RTC_AMIN_REG 	0x03
#define RTC_HOUR_REG 	0x04
#define RTC_AHOUR_REG 	0x05
#define RTC_WDAY_REG 	0x06 // Week day (1 --> sunday)
#define RTC_MDAY_REG 	0x07 // month day
#define RTC_MONTH_REG 	0x08
#define RTC_YEAR_REG 	0x09
#define RTC_SRA 		0x0a
#define RTC_SRB 		0x0b
#define RTC_SRC 		0x0c
#define RTC_SRA_UPDATE 	0x80
#define RTC_SRB_HOUR 	0x02
#define RTC_SRB_BINARY 	0x04
#define RTC_SRB_UPDATE 	0x10
#define RTC_SRB_ALARM 	0x20
#define RTC_SRB_PERIODIC 0x40
#define RTC_SRC_UPDATE 	0x10
#define RTC_SRC_ALARM 	0x20
#define RTC_SRC_PERIODIC 0x40
#define RTC_SRC_ANY 	0x80
#define MAX_KALARM_HANDLER	0x10

typedef struct
{
	void (*khandler)(void*);
	void* karg;
	u8_t used;	
} kalarm_handler_t;

u8_t bcd2bin(u8_t __bcd);
u8_t bin2bcd(u8_t __bin);
void rtc_write(u8_t __reg, u8_t __data);
u8_t rtc_read(u8_t __reg);
void rtc_init();
void rtc_update_pid(int __id);
int rtc_pid();
void __isr8(registers_t* __regs);
#endif //__RTC_H__
