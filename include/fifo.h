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

#ifndef __FIFO_H__
#define __FIFO_H__
#include <types.h>
#include <lib.h>
#include <fs/fs.h>

typedef struct
{
    #define FIFO_BUFFER_SIZE    512
    int read;
    int write;
    int to_read;
    struct dentry* dentry;
    char rbuf[FIFO_BUFFER_SIZE];
} __attribute__((packed)) fifo_t;

static inline int fifo_flush(fifo_t* __fifo)
{
    // check if fifo memory location
    // is safe
    if(!__fifo)
        return 0;

    // fill fifo buffer with 0
    memset(__fifo->rbuf,0,FIFO_BUFFER_SIZE);
    // set fifo cursor at 0
    __fifo->read = 0;
    __fifo->write = 0; 
    __fifo->to_read = 0;

    return 1;
}

static inline int fifo_write(fifo_t* __fifo, char __data)
{
    // check if fifo memory location
    // is safe
    if(!__fifo)
        return 0;

    // write inside fifo buffer
    __fifo->rbuf[__fifo->write] = __data;
    // update write cursor
    __fifo->write++;
    // check write cursor bound
    if(__fifo->write == FIFO_BUFFER_SIZE)
        __fifo->write = 0;

    // make sure this data is readable
    // by updating to_read value
    __fifo->to_read++;

    return 1;
}

static inline int fifo_read(fifo_t* __fifo, char* __data, int __hold /* make
 sure that at least one data is readable */)
{
    // check if fifo memory location and
    // __data memory location status are 
    // set
    if(!__fifo || !__data)
        return 0;

    // make sure that fifo buffer 
    // isn't empty
    if(__hold)
        while(!__fifo->to_read);
    else if(!__fifo->to_read)
        return 0;

    // read data from fifo buffer
    *__data = __fifo->rbuf[__fifo->read];
    // update fifo read cursor
    __fifo->read++;
    // check read cursor bound
    if(__fifo->read == FIFO_BUFFER_SIZE)
        __fifo->read = 0;

    // updaate to_read value
    __fifo->to_read--;

    return 1; 
}

static inline void fifo_pop(fifo_t* __fifo)
{   
    // check wether data is inside buffer
    if(__fifo->to_read)
    {
        // update write cursor
        if(__fifo->write)
            __fifo->write--;
        else
            __fifo->write = FIFO_BUFFER_SIZE - 1;

        // update to_read field
        __fifo->to_read--;
        __fifo->rbuf[__fifo->write] = '\0';
    }
}

#endif //__FIFO_H__