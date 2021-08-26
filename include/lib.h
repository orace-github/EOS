
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

#ifndef __LIB_H__
#define __LIB_H__
#include <types.h>
#include <io.h>

static inline void* memcpy(void* __dest, void* __src, int __size){
    for(int i = 0; i < __size; i++)
        ((char*)__dest)[i] = ((char*)__src)[i];
    return __dest;
}

static inline void* memset(void* __dest, char __byte, int __size){
    for(int i = 0; i < __size; i++)
       ((char*)__dest)[i] = __byte;
    return __dest;
}

static inline void* memset16(void* __dest, short __s, int __size){
    for(int i = 0; i < __size; i++)
        ((short*)__dest)[i] = __s;
    return __dest;
}

static inline void* memset32(void* __dest, int __v, int __size){
    for(int i = 0; i < __size; i++)
        ((int*)__dest)[i] = __v;
    return __dest;
}

static inline u32_t memcmp(const void* __src, const void* __dest, int __size){
    int i = 0;
    for(i = 0; i < __size && ((char*)__src)[i] == ((char*)__dest)[i]; i++);
    return (i == __size) ? 0: ((((char*)__src)[i] < ((char*)__dest)[i]) ? -1 : 1);
}

#endif //__LIB_H__
