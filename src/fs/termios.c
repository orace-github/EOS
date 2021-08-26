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

#include <termios.h>

// standard IO
static fifo_t* __stdin = NULL; // #0
static fifo_t* __stdout = NULL; // #1
static fifo_t* __stderr = NULL; // #2
// terminal
static termios_t* __termios = NULL;
static int __showinput = 0;

// keyboard 
extern kbd_t* __kbd;
extern kbd_event_t __kbd_event;

// process
extern unsigned int process_pid();
extern void process_kill(int __pid, int __signum);
extern void process_hold(); // no system call interface
extern void process_release(); // no system call interface

void init_stdio(fifo_t* __io0, fifo_t* __io1, fifo_t* __io2, termios_t* __term){
	#ifdef __DEBUG__
		kprintf("Installing standard IO...");
	#endif //__DEBUG__
	// #0 IO
	__stdin = __io0;
	// #1 IO
	__stdout = __io1;
	// #2 IO
	__stderr = __io2;
	// terminal
	__termios = __term;
	// __show input
	__showinput = 1;
	#ifdef __DEBUG__
		kprintf("[ok]\n");
	#endif //__DEBUG__
	register_interrupt_handler(__isr1,0x01);
}

void tty_showinput(int __bool)
{
	__showinput = __bool;
}

int tty_flush(int __fd)
{
	int __ret = 0;

	switch(__fd)
	{
		// flush stdin
		case 0:
			__ret = fifo_flush(__stdin);
			break;
		// flush stdout
		case 1:
			__ret = fifo_flush(__stdout);
			break;
		// flush stderr
		case 2:
			__ret = fifo_flush(__stderr);
			break;
		default:
			; 
	}

	return __ret;
}

int tty_read(int __fd, char* __buf, int __length)
{
	// hold current process
	process_hold();
	// wait until enter is pressed
	while(!__kbd_event.enter);
	// make sure that enter is released
	if(__kbd_event.enter)
		__kbd_event.enter = 0;
	// select fifo buffer
	fifo_t* __fifo = NULL;
	int __hold = 0;

	switch(__fd)
	{
		case 0:
			__fifo = __stdin;
			__hold = 1;
			break;
		case 1:
			__fifo = __stdout;
			__hold = 0;
			break;
		case 2:
			__fifo = __stderr;
			__hold = 0;
			break;
		default:
			// release process
			process_release();
			return 0;
	}

	// real data size
	// read from fifo 
	// buffer
	int __size = 0;
	for(int i = 0; i < __length; i++)
	{
		// read from fifo buffer
		int __ret = fifo_read(__fifo,&__buf[i],__hold);
		// make sure that data
		// has been read from 
		// fifo buffer
		if(!__ret)
		{
			__buf[i] = '\0';
			break;
		}

		// check if data is eol
		// character
		if(IS_EOL(__buf[i]))
		{
			__buf[i] = '\0';
			break;
		}

		// increase __size value
		// of 1
		__size++;

	}
	// release process
	process_release();
	return __size;
}

int tty_write(int __fd, char* __buf, int __length)
{
	// hold current process
	process_hold();
	// select fifo buffer
	fifo_t* __fifo = NULL;
	int __out = 0;
	switch(__fd)
	{
		case 0:
			__fifo = __stdin;
			__out = 0;
			break;
		case 1:
			__fifo = __stdout;
			__out = 1;
			break;
		case 2:
			__fifo = __stderr;
			__out = 0;
			break;
		default:
			// release process
			process_release();
			return 0;
	}

	int __size = 0;
	for(int i = 0; i < __length && __buf[i] != '\0'; i++)
	{
		// write data inside
		// fifo buffer
		fifo_write(__fifo,__buf[i]);
		__size++;	
	}

	// check if fifo buffer
	// is hardwired to screen
	if(__out && __showinput)
		kprintf(__buf);

	// release process
	process_release();

	return __size;
}


int tty_set(termios_t* __term)
{
	// make sure that 
	// termios memory 
	// location is safe
	if(!__term)
		return 0;

	// update kernel 
	// termios setting
	memcpy(__termios,__term,sizeof(termios_t));
	return 1;
}

int tty_get(termios_t* __term)
{
	// make sure that 
	// termios memory 
	// location is safe
	if(!__term)
		return 0;

	// update kernel 
	// termios setting
	memcpy(__term,__termios,sizeof(termios_t));
	return 1;	
}

void __isr1(registers_t* __regs){
	// get keyboard scancode
	u8_t __code = KBD_DATA();
	u32_t __key = 0;

	// whether keyboard key is pressed
	if(__code <= 0x80)
	{
		// special keycode
		switch(__code)
		{
			case KEY_ENTER:
				__kbd_event.enter = 1;
				// update __stdin buffer
				fifo_write(__stdin,'\n');
				// print to screen if __showinput
				// is set
				if(__showinput)
					put_char('\n');
				break;
			case KEY_F1:
				__kbd_event.f1 = 1;
				break;
			case KEY_F2:
				__kbd_event.f2 = 1;
				break;
			case KEY_F3:
				__kbd_event.f3 = 1;
				break;
			case KEY_F4:
				__kbd_event.f4 = 1;
				break;
			case KEY_F5:
				__kbd_event.f5 = 1;
				break;
			case KEY_F6:
				__kbd_event.f6 = 1;
				break;
			case KEY_F7:
				__kbd_event.f7 = 1;
				break;
			case KEY_F8:
				__kbd_event.f8 = 1;
				break;
			case KEY_F9:
				__kbd_event.f9 = 1;
				break;
			case KEY_F10:
				__kbd_event.f10 = 1;
				break;
			case KEY_ESC:
				__kbd_event.esc = 1;
				break;
			case KEY_UP:
				__kbd_event.up = 1;
				break;	
			case KEY_DOWN:
				__kbd_event.down = 1;
				break;
			case KEY_LEFT:
				__kbd_event.left = 1;
				break;
			case KEY_RIGHT:
				__kbd_event.right = 1;
				break;
			case KEY_PGUP:
				__kbd_event.pgup = 1;
				break;
			case KEY_PGDWN:
				__kbd_event.pgdwn = 1;
				break;
			case KEY_NUMLOCK:
				__kbd_event.num = 1;
				break;
			case KEY_HOME:
				__kbd_event.home = 1;
				break;
			case KEY_LSHIFT:
				__kbd_event.lshift = 1;
				break;
			case KEY_RSHIFT:
				__kbd_event.rshift = 1;
				break;
			case KEY_ALT:
				__kbd_event.alt = 1;
				break;
			case KEY_CNTRL:
				__kbd_event.ctrl = 1;
				break;
			case KEY_CAPS:
				__kbd_event.caps = 1;
				break;
			default:
				// get keyboard unicode key
				if(__kbd_event.rshift || __kbd_event.lshift ||
					__kbd_event.caps)
					__key = __kbd->kbdmap[__code*4 + 1];
				else if(__kbd_event.alt)
					__key = __kbd->kbdmap[__code*4 + 2];
				else if(__kbd_event.ctrl)
					__key = __kbd->kbdmap[__code*4 + 3];
				else
					__key = __kbd->kbdmap[__code*4];

				// process signal
				if(__termios->c_iflag & ISIG && __kbd_event.ctrl)
				{
					if(__key == __termios->c_cc[VINTR])
					{
						// send SIGKILL to process
						process_kill(process_pid(),SIGKILL);
						return;
					}

					if(__key == __termios->c_cc[VSUSP])
					{
						// send SIGSTOP to process
						process_kill(process_pid(),SIGSTOP);
						return;
					}
				}			
				// write code inside __stdin
				if((char)__key == '\b')
					fifo_pop(__stdin);	
				else
					fifo_write(__stdin,(char)__key);

				// put data to the screen
				if(__showinput)
					put_char((char)__key);
		}
	}
	else // keyboard key is released
	{
		__code -= 0x80;
		// special keycode
		switch(__code)
		{
			case KEY_ENTER:
				__kbd_event.enter = 0;
				break;
			case KEY_F1:
				__kbd_event.f1 = 0;
				break;
			case KEY_F2:
				__kbd_event.f2 = 0;
				break;
			case KEY_F3:
				__kbd_event.f3 = 0;
				break;
			case KEY_F4:
				__kbd_event.f4 = 0;
				break;
			case KEY_F5:
				__kbd_event.f5 = 0;
				break;
			case KEY_F6:
				__kbd_event.f6 = 0;
				break;
			case KEY_F7:
				__kbd_event.f7 = 0;
				break;
			case KEY_F8:
				__kbd_event.f8 = 0;
				break;
			case KEY_F9:
				__kbd_event.f9 = 0;
				break;
			case KEY_F10:
				__kbd_event.f10 = 0;
				break;
			case KEY_ESC:
				__kbd_event.esc = 0;
				break;
			case KEY_UP:
				__kbd_event.up = 0;
				break;	
			case KEY_DOWN:
				__kbd_event.down = 0;
				break;
			case KEY_LEFT:
				__kbd_event.left = 0;
				break;
			case KEY_RIGHT:
				__kbd_event.right = 0;
				break;
			case KEY_PGUP:
				__kbd_event.pgup = 0;
				break;
			case KEY_PGDWN:
				__kbd_event.pgdwn = 0;
				break;
			case KEY_NUMLOCK:
				__kbd_event.num = 0;
				break;
			case KEY_HOME:
				__kbd_event.home = 0;
				break;
			case KEY_LSHIFT:
				__kbd_event.lshift = 0;
				break;
			case KEY_RSHIFT:
				__kbd_event.rshift = 0;
				break;
			case KEY_ALT:
				__kbd_event.alt = 0;
				break;
			case KEY_CNTRL:
				__kbd_event.ctrl = 0;
				break;
			case KEY_CAPS:
				__kbd_event.caps = 0;
				break;
			default:
				break;
		}
	}
}
