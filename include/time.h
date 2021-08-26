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

#ifndef __TIME_H__
#define __TIME_H__
#include <types.h>
#include <driver/rtc.h>
#include <mutex.h>
extern int __bcd;
extern unsigned int __clock;
// kernel alarm routine
extern kalarm_handler_t __kalarm_handler[];
static unsigned int __rand = 1;
typedef struct
{
    u8_t year;
    u8_t month;
    u8_t day;
    u8_t hour;
    u8_t minute;
    u8_t second;
}__attribute__((packed)) time_t;
typedef struct
{
	time_t time;
	int  pid;
}__attribute__((packed)) alarm_t;

static inline void time_computing(time_t* __time, int __timeval /* in sec */){
	__timeval += __time->second;
	__time->second = __timeval % 60;
	if(!(__timeval /= 60)) // minute
		return;
	__timeval += __time->minute;
	__time->minute = __timeval % 60;
	if(!(__timeval /= 60)) // hour
		return;
	__timeval += __time->hour;
	__time->hour = __timeval % 24;
	if(!(__timeval /= 24)) // day
		return;
	__timeval += __time->day;
	__time->day = __timeval % 30;
	if(!(__timeval /= 30)) // month
		return;
	__timeval += __time->month;
	__time->month = __timeval % 12;
	if(!(__timeval /= 12)) // year
		return;
	__time->year += __timeval;
}

static inline void local_time(time_t* __time){
	// wait while rtc update in progress
	// to get 1 second to read time from 
	// rtc.
	while(rtc_read(RTC_SRA) & RTC_SRA_UPDATE);
	__time->year = rtc_read(RTC_YEAR_REG);
	__time->month = rtc_read(RTC_MONTH_REG);
	__time->day = rtc_read(RTC_MDAY_REG);
	__time->hour = rtc_read(RTC_HOUR_REG);
	__time->minute = rtc_read(RTC_MIN_REG);	
	__time->second = rtc_read(RTC_SEC_REG);

	if(__bcd){
		__time->year = bcd2bin(__time->year);
		__time->month = bcd2bin(__time->month);
		__time->day = bcd2bin(__time->day);
		__time->hour = bcd2bin(__time->hour);
		__time->minute = bcd2bin(__time->minute);
		__time->second = bcd2bin(__time->second);
	}
}

static inline void set_time(time_t* __time){
	// check if __time memory 
	// location is safe
	if(!__time)
		return;
	rtc_write(RTC_YEAR_REG, __bcd ? bin2bcd(__time->year) : __time->year);
	rtc_write(RTC_MONTH_REG, __bcd ? bin2bcd(__time->month) : __time->month);
	rtc_write(RTC_MDAY_REG, __bcd ? bin2bcd(__time->day) : __time->day);
	rtc_write(RTC_HOUR_REG, __bcd ? bin2bcd(__time->hour) : __time->hour);
	rtc_write(RTC_MIN_REG, __bcd ? bin2bcd(__time->minute) : __time->minute);
	rtc_write(RTC_SEC_REG, __bcd ? bin2bcd(__time->second) : __time->second);
}

static inline void alarm(alarm_t* __alarm){
	// setting rtc alarm registers
	rtc_write(RTC_ASEC_REG, __bcd ? bin2bcd(__alarm->time.second) : __alarm->time.second);
	rtc_write(RTC_AMIN_REG, __bcd ? bin2bcd(__alarm->time.minute) : __alarm->time.minute);
	rtc_write(RTC_AHOUR_REG, __bcd ? bin2bcd(__alarm->time.hour) : __alarm->time.hour);
	rtc_write(RTC_YEAR_REG, __bcd ? bin2bcd(__alarm->time.year) : __alarm->time.year);
	rtc_write(RTC_MONTH_REG, __bcd ? bin2bcd(__alarm->time.month) : __alarm->time.month);
	rtc_write(RTC_MDAY_REG, __bcd ? bin2bcd(__alarm->time.day) : __alarm->time.day);
	// now update rtc pid
	rtc_update_pid(__alarm->pid);
}

/* Hint: Only one function can be handled by kernel now due to not scheduling alarm timeout not yet implemented */
static inline void timeout(void (*__handler)(void*), void* __arg, alarm_t* __timeout){
	for(int i = 0; i < MAX_KALARM_HANDLER; i++)
		if(!__kalarm_handler[i].used){
			__kalarm_handler[i].khandler = __handler;
			__kalarm_handler[i].karg = __arg;
			__kalarm_handler[i].used = 1;
			alarm(__timeout);
			break;
		}
}

static void srand(unsigned int __seed){
	__rand = __seed;
}

static unsigned int rand(){
	__rand = __rand * 1103515245 + 12345;
	return ((unsigned int)(__rand / 65536) % 32768);
}

static unsigned int time_clock(){
	return __clock;
}

#endif //__TIME_H__
