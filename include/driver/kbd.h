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

#ifndef __KBD_H___
#define __KBD_H___
#include <types.h>
#include <string.h>
#include <list.h>
#include <keycode.h>
#include <io.h>

#define KBD_CMD_REG		0x64
#define KBD_DATA_REG	0x60

#define KBD_CMD(CMD) \
	outb(KBD_CMD_REG,CMD)
#define KBD_DATA() 	\
	inb(KBD_DATA_REG)
#define KDB_STATUS 	\
	inb(KBD_CMD_REG)

#define KBD_LED_CMD		0xed
#define KBD_ECHO_CMD	0xee
#define KBD_CODE_CMD	0xf0
#define KBD_TYPE_CMD	0xf2
#define KBD_RATE_CMD	0xf3
#define KBD_ENABLE_CMD	0xf4
#define KBD_DESABLE_CMD	0xf5
#define KBD_DEFAULT_CMD	0xf6
#define KBD_TYPEMATIC_CMD	0xf7
#define KBD_MAKE_CMD	0xf8
#define KBD_RESET_CMD	0xff

#define KBD_ACK_RP		0xfa
#define KBD_RESEND_RP	0xfe
#define KBD_ERROR_RP	0xff
#define KBD_RESET_RP	0xaa
#define KBD_ECHO_RP		0xee

typedef struct
{
	u8_t alt;
	u8_t caps;
	u8_t ctrl;
	u8_t lshift;
	u8_t rshift;
	u8_t num 	: 1;
	u8_t f1 	: 1;
	u8_t f2 	: 1;
	u8_t f3 	: 1;
	u8_t f4 	: 1;
	u8_t f5 	: 1;
	u8_t f6 	: 1;
	u8_t f7 	: 1;
	u8_t f8 	: 1;
	u8_t f9 	: 1;
	u8_t f10 	: 1;
	u8_t esc 	: 1;
	u8_t pgup 	: 1;
	u8_t pgdwn  : 1;
	u8_t left 	: 1;
	u8_t right 	: 1;
	u8_t up 	: 1;
	u8_t down 	: 1;
	u8_t home 	: 1;
	u8_t enter	: 1;
	u8_t unused	: 2;	
} __attribute__((packed)) kbd_event_t;

typedef struct
{
	const u8_t* name; // keyboard type name
	const u32_t* kbdmap; // key mapping
	struct list_head head;
} __attribute__((packed)) kbd_t; 

typedef struct kbd_hashtable
{
	#define KBD_HASH_MAX	10
	struct list kbd_list[KBD_HASH_MAX];
	void (*insert)(kbd_t*, struct kbd_hashtable*);
	void (*remove)(kbd_t*, struct kbd_hashtable*);
	kbd_t* (*find)(u8_t*, struct kbd_hashtable*);
} kbd_hashtable_t;

void init_kbd();
void kbd_event(kbd_event_t* __event);
void kbd(kbd_t* __keyboard);
void add_kbd(kbd_t* __kbd);
void remove_kbd(kbd_t* __kbd);

#endif //__KBD_H___
