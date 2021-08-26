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

#include <driver/rtc.h>
extern void process_kill(int __pid, int __signum);
static int __pid = 0;
kalarm_handler_t __kalarm_handler[MAX_KALARM_HANDLER] __attribute__((section(".data")));
int __bcd = 0;
unsigned int __clock = 0;

u8_t bcd2bin(u8_t __bcd){
	/* convert BCD data byte in binary */
	return ((__bcd >> 4)*10 + (__bcd & 0x0f));
}

u8_t bin2bcd(u8_t __bin){
	/* convert binary data byte in BCD */
	u8_t __low = __bin % 10;
	u8_t __high = (__bin - __low) / 10;
	return ((__high << 4) | (__low & 0x0f));
}

void rtc_write(u8_t __reg, u8_t __data){
	outb(RTC_CMD_REG,0x80|__reg); 	
	outb(RTC_DATA_REG,__data);
}

u8_t rtc_read(u8_t __reg){
	outb(RTC_CMD_REG,0x80|__reg);	
	return inb(RTC_DATA_REG);
}

void rtc_init(){
	#ifdef __DEBUG__
		kprintf("Installing RTC driver...");
	#endif //__DEBUG__
	// read rtc register B
	u8_t __status = rtc_read(RTC_SRB);	
	// enable alarm - update
	__status |= RTC_SRB_ALARM;
	__status |= RTC_SRB_UPDATE;
	__status |= RTC_SRB_HOUR;
	__bcd = !(__status & RTC_SRB_BINARY);
	rtc_write(RTC_SRB,__status);
	// register rtc irq handler
	register_interrupt_handler(__isr8,0x08);
	// flush rtc C register
	rtc_read(RTC_SRC);
	// initialize kalarm handler
	for(int i = 0; i < MAX_KALARM_HANDLER; i++)
		__kalarm_handler[i].used = 0;
	#ifdef __DEBUG__
		kprintf("[ok]\n");
	#endif //__DEBUG__
}

void rtc_update_pid(int __id){
	__pid = __id;
}

int rtc_pid(){
	return __pid;
}

void __isr8(registers_t* __regs){
	u8_t __status = rtc_read(RTC_SRC);
	if(__status & RTC_SRC_ALARM) {
		// send SIGALARM to process
		process_kill(__pid,SIGALARM);
		// execute kernel alarm routine
		for(int i = 0; i < MAX_KALARM_HANDLER; i++)
			if(__kalarm_handler[i].used){
				__kalarm_handler[i].khandler(__kalarm_handler[i].karg);
				__kalarm_handler[i].used = 0;
			}
	}else if(__status & RTC_SRC_UPDATE) {
		__clock++;
	}
}
