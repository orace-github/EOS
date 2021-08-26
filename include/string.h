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

#ifndef __STRING_H__
#define __STRING_H__
#include <lib.h>
extern void sys_free(void*);
extern void* sys_alloc(u32_t);

static inline char* strncpy(char* __dest, const char* __src, int __length)
{
    int i = 0;
    for(i = 0; i < __length && __src[i]; i++)
        __dest[i] = __src[i];
    
    for(; i < __length; i++)
        __dest[i] = '\0';
    __dest[__length - 1] = '\0';
    return __dest;
}

static inline int strlen(const char* __string)
{
    int __length = 0;
    for(; __string[__length]; __length++);
    return __length;
}

static inline int strncmp(const char* __str0, const char* __str, int __length)
{
    int i = 0;
    for(i = 0; i < __length && __str0[i] == __str[i]; i++);
    return (i == __length)? 0: ((__str0[i] < __str[i])? -1:1);
}

static inline char* strchr(char* __string, char __c)
{
    int i = 0;
    for(i = 0; __string[i]; i++)
        if(__string[i] == __c)
            return (__string + i);
    return NULL;
}

static inline char* strrchr(char* __string, char __c)
{
    int i = 0;
    for(i = strlen(__string)-1; i >= 0; i--)
        if(__string[i] == __c)
            return (__string + i);
    return NULL;
}

static inline char * strstr(const char* __in, const char* __str)
{
    char c;
    u32_t __len;

    c = *__str++;
    if (!c)
        return (char *)__in;

    __len = strlen(__str);
    do {
        char sc;

        do {
            sc = *__in++;
            if (!sc)
                return (char *)0;
        } while (sc != c);
    } while (strncmp(__in,__str,__len) != 0);

    return (char *)(__in - 1);
}

static inline int strcmp(const char* __str0, const char* __str)
{
    return strncmp(__str0,__str,strlen(__str));
}

static inline char* strcpy(char* __dest, char* __src)
{
    return strncpy(__dest,__src,strlen(__src)+1);
}

static inline char* strncat(char* __dest, const char* __src, int __length)
{
    int __offset = strlen(__dest);

    strncpy(__dest + __offset,__src,__length);

    return __dest;
}

static inline char* strcat(char* __dest, const char* __src)
{
    int __offset = strlen(__src);
 
    return strncat(__dest,__src,__offset);
}

static inline void itoa(char* __buf, u32_t __int, int __base)
{
    u32_t __tmp;
    int i, j;

    __tmp = __int;
    i = 0;

    do {
        __tmp = __int % __base;
        __buf[i++] = (__tmp < 10) ? (__tmp + '0') : (__tmp + 'a' - 10);
    } while (__int /= __base);
    __buf[i--] = 0;

    for (j = 0; j < i; j++, i--) {
        __tmp = __buf[j];
        __buf[j] = __buf[i];
        __buf[i] = __tmp;
    }
}

static inline int isspace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

static inline int isprint(char c)
{
    return ((c >= ' ' && c <= '~') ? 1 : 0);
}

static inline int atoi(char * __string)
{
    int __result = 0;
    u32_t __digit;
    int __sign;

    while (isspace(*__string)) {
        __string += 1;
    }

    /*
     * Check for a sign.
     */

    if (*__string == '-') {
        __sign = 1;
        __string += 1;
    } else {
        __sign = 0;
        if (*__string == '+') {
            __string += 1;
        }
    }

    for ( ; ; __string += 1) {
        __digit = *__string - '0';
        if (__digit > 9) {
            break;
        }
        __result = (10*__result) + __digit;
    }

    if (__sign) {
        return -__result;
    }
    return __result;
}

/*
static inline char* strdup(char* __src)
{
    int __len = strlen(__src) + 1;
    char* __dest = sys_alloc(__len);
    memcpy(__dest,__src,__len);
    return __dest;
}
*/
#endif //__STRING_H__
