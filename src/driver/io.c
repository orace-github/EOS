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


#include <io.h>
static int __cx = 0;
static int __cy = 0;
int __color = BLACK|WHITE;
static volatile u16_t* __screen  = (volatile u16_t*)0xb8000;

void update_cursor(int __x, int __y)
{
    // compute position
    u16_t __pos = __y*80 + __x;
    // send position to screen 
    // controller
    outb(0x3d4,0x0f);
    outb(0x3d5,(u8_t)__pos);
    outb(0x3d4,0x0e);
    outb(0x3d5,(u8_t)(__pos>>8));
}

void put_char(char __data)
{
    if(__data == '\n')
    {
        __cy++;
        __cx = 0;
    }
    else if(__data == '\r')
    {
        __cx = 0;
    }
    else if(__data == '\b')
    {
        if(__cx > 0)
        {
            __cx--;
            __screen[__cy*80 + __cx] = (u16_t)((__color<<8)|'\0');
        }
        else
        {
            if(__cy > 0)
            {
                __cx = 79;
                __cy--;
                __screen[__cy*80 + __cx] = (u16_t)((__color<<8)|'\0');
            }
        }
    }
    else if(__data == '\t')
    {
        __cx += 4 - (__cx % 4);
    }
    else
    {
        __screen[__cy*80 + __cx] = (u16_t)((__color<<8)|__data);
        __cx++;
    }

    if(__cx > 79)
    {
        __cx = 0;
        __cy++;
    }

    if(__cy > 24)
    {
        scrollup(1);
        __cy = 24;
    }
    
    update_cursor(__cx,__cy);
}

void put_string(const char* __string)
{
    // make sure that __string 
    // memory location is safe
    if(!__string)
        return;

    for(int i = 0; __string[i] != '\0'; i++)
        put_char(__string[i]);
}

void put_hex(int __data)
{
    static char* __format = "0x00000000";
    static char* __key = "0123456789ABCDEF";
    int __tampon = 0;
    __tampon = __data & 0x0f;
    __format[9] = __key[__tampon];
    __tampon = (__data>>4) & 0x0f;
    __format[8] = __key[__tampon];
    __tampon = (__data>>8) & 0x0f;
    __format[7] = __key[__tampon];
    __tampon = (__data>>12) & 0x0f;
    __format[6] = __key[__tampon];
    __tampon = (__data>>16) & 0x0f;
    __format[5] = __key[__tampon];
    __tampon = (__data>>20) & 0x0f;
    __format[4] = __key[__tampon];
    __tampon = (__data>>24) & 0x0f;
    __format[3] = __key[__tampon];
    __tampon = (__data>>28) & 0x0f;
    __format[2] = __key[__tampon];
    put_string(__format);
}

void scrollup(int __line)
{
    volatile u16_t* __from = __screen + __line * 80;
    volatile u16_t* __to = __screen;
    volatile u16_t* __end = __screen + 25 * 80;

    while(__from < __end)
    {
        *__to++ = *__from++;
    }

    while(__to < __end)
    {
        *__to++ = '\0' | (__color << 8);
    }
}


void kprintf(const char* __str,...)
{
    static va_list __list;
    va_start(__list,__str);

    for(int i = 0; i < strlen(__str); i++)
    {
        if(__str[i] == '%')
        {
            if(__str[i+1] == 'x' || __str[i+1] == 'p')
            {
                int __arg = va_arg(__list,int);
                put_hex(__arg);
                i++;
            }else if(__str[i+1] == 's')
            {
                char* __msg = va_arg(__list,char*);
                put_string(__msg);
                i++;
            }
            else if(__str[i+1] == 'd')
            {
                int __arg = va_arg(__list,int);
                char __buf[32];
                itoa(__buf,__arg,10);
                put_string(__buf);
                i++;
            }
            else if(__str[i+1] == 'c')
            {
                char __data = va_arg(__list,int);
                put_char(__data);
                i++;
            }else
            {
                put_char(__str[i]);
            }
        }
        else
        {
            put_char(__str[i]);
        }

    }

    va_end(__list);
}
