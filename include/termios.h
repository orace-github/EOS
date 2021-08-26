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

#ifndef __TERMIOS_H__
#define __TERMIOS_H__
#include <driver/kbd.h>
#include <fifo.h>
#include <ipc/signal.h>
#include <io.h>
#include <lib.h>

typedef unsigned long tcflag_t;
typedef unsigned char cc_t;
#define NCCS	17
/** Optionnal actions for tcsetattr. **/

#define TCSANOW		1
#define TCSAFLUSH	3

/** Flush modes. **/

#define TCIFLUSH	1
#define TCIOFLUSH	3

/** c_iflag bits **/

#define INLCR	0000100
#define IGNCR	0000200
#define ICRNL	0000400
#define IXON	0002000
#define IXOFF	0010000

/** c_oflag bits **/

#define OPOST	0000001
#define ONLCR	0000004
#define OCRNL	0000010
#define ONOCR	0000020
#define ONLRET	0000040

/** c_lflag bits **/

#define ISIG	0000001
#define ICANON	0000002
#define ECHO	0000010
#define ECHOE	0000020
#define ECHOK	0000040
#define ECHOCTL	0001000
#define ECHOKE	0004000

/** c_cflag bits **/

#define CSIZE	0000060
#define CS5	0000000
#define CS6	0000020
#define CS7	0000040
#define CS8	0000060
#define CSTOPB	0000100
#define CREAD	0000200
#define PARENB	0000400
#define PARODD	0001000
#define HUPCL	0002000

/* c_cc characters */

#define VINTR		0
#define VQUIT		1
#define VERASE		2
#define VKILL		3
#define VEOF		4
#define VTIME		5
#define VMIN		6
#define VSWTC		7
#define VSTART		8
#define VSTOP		9
#define VSUSP		10
#define VEOL		11
#define VREPRINT	12
#define VDISCARD	13
#define VWERASE		14
#define VLNEXT		15
#define VEOL2		16

// warning: never change __termios
// in this macro without change
// __termios static variable
#define IS_EOL(c) (c == '\n' \
				|| c == __termios->c_cc[VEOL] \
				|| c == __termios->c_cc[VEOL2] \
				|| c == __termios->c_cc[VEOF])

typedef struct
{
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	cc_t c_cc[NCCS];
} termios_t;

int tty_read(int __fd, char* __buf, int __length);
int tty_write(int __fd, char* __buf, int __length);
int tty_flush(int __fd);
int tty_set(termios_t* __term);
int tty_get(termios_t* __term);
void tty_showinput(int __bool);
// keyboard interrupt
// handler
void __isr1(registers_t* __regs);
void init_stdio(fifo_t* __io0, fifo_t* __io1, fifo_t* __io2, termios_t* __term);
#endif //__TERMIOS_H__