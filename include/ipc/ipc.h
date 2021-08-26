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

#ifndef __IPC_H__
#define __IPC_H__
#include <error.h>
#define IPC_CREATE 		0x0e
#define IPC_READ 		0x01
#define IPC_WRITE 		0x02	
#define IS_IPC_CREATE(mode)		(mode & IPC_CREATE)
#define IS_IPC_READ(mode) 		(mode & IPC_READ)
#define IS_IPC_WRITE(mode) 		(mode & IPC_WRITE)
#endif //__IPC_H__