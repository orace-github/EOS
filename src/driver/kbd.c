/*
 * Copyright 2019 Orace KPAKPO
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

#include <driver/kbd.h>
// kernel keyboard event
kbd_event_t __kbd_event;
// current keyboard
kbd_t* __kbd = NULL;
// kernel keyboard hashtable
static kbd_hashtable_t __kbd_hashtable;
// 1 column NULL event
// 2 column RSHIFT | LSHIT | CAPS 
// 3 column LALT | RALT
// 4 column LCTRL | RCTRL

// inspired from (PEPIN AND KARYON OS)
static u32_t en_map[] = {
	0xFF, 0xFF, 0xFF, 0xFF,
	0x1B, 0x1B, 0x1B, 0x1B,	/*      esc     (0x01)  */
	'1', '!', '1', '1',
	'2', '@', '2', '2',
	'3', '#', '3', '3',
	'4', '$', '4', '4',
	'5', '%', '5', '5',
	'6', '^', '6', '6',
	'7', '&', '7', '7',
	'8', '*', '8', '8',
	'9', '(', '9', '9',
	'0', ')', '0', '0',
	'-', '_', '-', '-',
	'=', '+', '=', '=',
	0x08, 0x08, 0x7F, 0x08,	/*      backspace       */
	0x09, 0x09, 0x09, 0x09,	/*      tab     */
	'q', 'Q', 'q', 'q',
	'w', 'W', 'w', 'w',
	'e', 'E', 'e', 'e',
	'r', 'R', 'r', 'r',
	't', 'T', 't', 't',
	'y', 'Y', 'y', 'y',
	'u', 'U', 'u', 'u',
	'i', 'I', 'i', 'i',
	'o', 'O', 'o', 'o',
	'p', 'P', 'p', 'p',
	'[', '{', '[', '[',
	']', '}', ']', ']',
	0x0A, 0x0A, 0x0A, 0x0A,	/*      enter   */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      ctrl    */
	'a', 'A', 'a', 'A',
	's', 'S', 's', 's',
	'd', 'D', 'd', 'd',
	'f', 'F', 'f', 'f',
	'g', 'G', 'g', 'g',
	'h', 'H', 'h', 'h',
	'j', 'J', 'j', 'j',
	'k', 'K', 'k', 'k',
	'l', 'L', 'l', 'l',
	'm', 'M', 'm', 'm',
	0x27, 0x22, 0x27, 0x27,	/*      '"      */
	'`', '~', '`', '`',	/*      `~      */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      Lshift  (0x2a)  */
	'\\', '|', '\\', '\\',
	'z', 'Z', 'z', 'Z',
	'x', 'X', 'x', 'x',
	'c', 'C', 'c', 'c',
	'v', 'V', 'v', 'v',
	'b', 'B', 'b', 'b',
	'n', 'N', 'n', 'n',
	',', '?', ',', ',',
	0x2C, 0x3C, 0x2C, 0x2C,	/*      ,<      */
	0x2E, 0x3E, 0x2E, 0x2E,	/*      .>      */
	0x2F, 0x3F, 0x2F, 0x2F,	/*      /?      */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      Rshift  (0x36)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x37)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x38)  */
	' ', ' ', ' ', ' ',	/*      space   */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x3a)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x3b)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x3c)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x3d)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x3e)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x3f)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x40)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x41)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x42)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x43)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x44)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x45)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x46)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x47)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x48)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x49)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x4a)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x4b)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x4c)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x4d)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x4e)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x4f)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x50)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x51)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x52)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x53)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x54)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x55)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x56)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x57)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x58)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x59)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x5a)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x5b)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x5c)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x5d)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x5e)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x5f)  */
	0xFF, 0xFF, 0xFF, 0xFF,	/*      (0x60)  */
	0xFF, 0xFF, 0xFF, 0xFF	/*      (0x61)  */
};

static inline int kbd_hash(const u8_t* __name)
{
	int __key = 0;
	for(int i = 0; __name[i] != '\0'; i++)
		__key += __name[i];

	return __key % KBD_HASH_MAX;
}

static inline void kbd_insert(kbd_t* __kbd, kbd_hashtable_t* __kbd_hashtable)
{
	if(!__kbd)
		return;
	// insert at end of list
	list_insert(&__kbd_hashtable->kbd_list[kbd_hash(__kbd->name)],&__kbd->head);
}

static inline void kbd_remove(kbd_t* __kbd, kbd_hashtable_t* __kbd_hashtable)
{
	if(!__kbd)
		return;
	// remove from list
	list_remove(&__kbd_hashtable->kbd_list[kbd_hash(__kbd->name)],&__kbd->head);	
}

static inline kbd_t* kbd_find(u8_t* __name, kbd_hashtable_t* __kbd_hashtable)
{
	if(!__name)
		return NULL;
	// iterator
	struct list_head* __it = NULL;
	// go through list head
	list_for_each(__it,&__kbd_hashtable->kbd_list[kbd_hash(__name)].head)
	{
		kbd_t* __kbd = list_entry(__it,kbd_t,head);
		if(!strcmp(__name,__kbd->name))
			return __kbd;
	}

	return NULL;
}


void init_kbd()
{
	#ifdef __DEBUG__
		kprintf("Installing Keyboard driver...");
	#endif //__DEBUG__
	// init hashtable
	for(int i = 0; i < KBD_HASH_MAX; i++)
		init_list(&__kbd_hashtable.kbd_list[i]);
	__kbd_hashtable.insert = kbd_insert;
	__kbd_hashtable.remove = kbd_remove;
	__kbd_hashtable.find = kbd_find;

	// new english keyboard
	static kbd_t __en_kbd = {
		.name = "EN_KBD",
		.kbdmap = en_map,
	};
	// registered keyboard
	__kbd_hashtable.insert(&__en_kbd,&__kbd_hashtable);
	// set as current keyboard
	__kbd = &__en_kbd;

	#ifdef __DEBUG__
		kprintf("[ok]\n");
	#endif //__DEBUG__
	
}

void kbd_event(kbd_event_t* __event)
{
	if(!__event)
		return;
	// copy keyboard event
	*__event = __kbd_event;
}

void kbd(kbd_t* __keyboard)
{
	if(!__keyboard)
		return;
	// change current keyboard
	__kbd = __keyboard;
}

void add_kbd(kbd_t* __kbd)
{
	__kbd_hashtable.insert(__kbd,&__kbd_hashtable);
}

void remove_kbd(kbd_t* __kbd)
{
	__kbd_hashtable.remove(__kbd,&__kbd_hashtable);
}
